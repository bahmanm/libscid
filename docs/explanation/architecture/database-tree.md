# Tree {#architecture_database_tree}

Tree statistics are database-side opening views.  They start from a filtered
set of games, read the ply hint stored in each filter value, and group the next
move played at that point.  The result is not a decoded forest of games; it is
a compact set of @ref scid::database::TreeNode "TreeNode" values suitable for
opening explorers, novelty workflows and "what was played here?" interfaces.

@ref scid::database::scidBaseT::setPositionSearchFilter
"setPositionSearchFilter()" usually prepares the input.  It rewrites an
@ref scid::database::HFilter "HFilter" so matching games are included and the
stored byte is `ply + 1`: value one means the game reaches the requested
position at the start, value two means after the first half-move, and so on.
@ref scid::database::scidBaseT::getTreeStat "getTreeStat()" then consumes that
filter and folds all visible next moves into tree nodes.

@startuml database-tree
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 70
skinparam Ranksep 58
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

<style>
rectangle {
  Padding 26
  MinimumWidth 150
  MinimumHeight 44
}
</style>

left to right direction

rectangle "Position search" as PositionSearch #EEF2FF
rectangle "Filter with ply hints" as FilterHints #EEF2FF
rectangle "Tree statistics" as TreeStats #FFF7ED
rectangle "Move aggregates" as MoveAggregates #EEF2FF
rectangle "Stored-line shortcuts" as StoredLines #EEF2FF
rectangle "Game view fallback" as GameView #EEF2FF

PositionSearch -[hidden]right- FilterHints
FilterHints -[hidden]right- TreeStats
TreeStats -[hidden]right- MoveAggregates
StoredLines -[hidden]right- GameView
FilterHints -[hidden]down- StoredLines
TreeStats -[hidden]down- GameView

PositionSearch --> FilterHints : writes matches
FilterHints --> TreeStats : supplies ply
StoredLines --> TreeStats : fast next move
GameView --> TreeStats : decoded next move
TreeStats --> MoveAggregates : groups by move
@enduml

For each visible game, @ref scid::database::scidBaseT "scidBaseT" reads the
filter value, subtracts one to recover the ply, and asks the index-backed
stored-line code for the move at that ply.  Stored lines cover common opening
prefixes and let the tree answer many early-position questions without decoding
game text.  If the stored line has no move for that ply, the database opens a
lightweight game view and reads the move from the encoded game record.

The tree node itself is deliberately small.  @ref scid::database::TreeNode
"TreeNode" owns the candidate @ref scid::core::FullMove "FullMove", the total
frequency, result buckets, rating sums and year sums.  Convenience methods such
as @ref scid::database::TreeNode::score "score()",
@ref scid::database::TreeNode::eloPerformance "eloPerformance()",
@ref scid::database::TreeNode::avgElo "avgElo()" and
@ref scid::database::TreeNode::avgYear "avgYear()" turn those sums into the
numbers a caller normally wants to display.  The returned nodes are sorted by
descending game count.

## Domain Model

This diagram shows the public tree model and the nearby database machinery it
depends on.  The stored-line and game-view boxes are conceptual here: callers
do not own those objects, but they explain why tree queries can often stay in
metadata and only decode games when the index shortcut is not enough.

@ref scid::database::TreeNode "TreeNode" is the only value returned to the
caller.  @ref scid::database::HFilter "HFilter" controls both inclusion and
the ply to inspect, @ref scid::database::IndexEntry "IndexEntry" supplies
result/rating/year metadata and the stored-line code, and
@ref scid::core::FullMove "FullMove" identifies the candidate move.

@startuml database-tree-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 92
skinparam Ranksep 82
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 130
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

left to right direction

class "scidBaseT" as DetailSession <<session facade>> #EEF2FF {
  setPositionSearchFilter()
  getTreeStat()
}

class "HFilter" as DetailFilter <<ply hints>> #EEF2FF {
  get()
}

class "IndexEntry" as DetailIndexEntry <<metadata row>> #EEF2FF {
  GetStoredLineCode()
  GetResult()
  GetWhiteElo()
  GetBlackElo()
  GetYear()
}

class "Stored line" as DetailStoredLine <<opening shortcut>> #EEF2FF {
  getMove()
}

class "Game view" as DetailGameView <<decoded fallback>> #EEF2FF {
  getMove()
}

class "TreeNode" as DetailTreeNode <<move aggregate>> #FFF7ED {
  move
  freq[]
  add()
  score()
  eloPerformance()
  avgElo()
  avgYear()
}

class "FullMove" as DetailFullMove <<candidate move>> #EEF2FF

DetailSession -[hidden]down- DetailFilter
DetailFilter -[hidden]down- DetailIndexEntry
DetailIndexEntry -[hidden]right- DetailStoredLine
DetailStoredLine -[hidden]right- DetailTreeNode
DetailTreeNode -[hidden]right- DetailFullMove
DetailStoredLine -[hidden]down- DetailGameView

DetailSession --> DetailFilter : consumes
DetailFilter --> DetailIndexEntry : selects ply / game
DetailIndexEntry --> DetailStoredLine : stored code
DetailStoredLine --> DetailTreeNode : next move
DetailGameView --> DetailTreeNode : fallback move
DetailIndexEntry --> DetailTreeNode : result / rating / year
DetailTreeNode --> DetailFullMove : owns
@enduml
