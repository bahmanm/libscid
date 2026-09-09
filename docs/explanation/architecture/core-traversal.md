# Traversal {#architecture_core_traversal}

Core traversal is the operational model for walking and editing a game's
recursive movetext tree.  @ref scid::core::GameCursor
"scid::core::GameCursor" is the read-only view:
it moves between moves, enters and exits variations, captures restorable
locations, and can replay the path to produce a @ref scid::core::Position
"Position".  @ref scid::core::MovetextCursor "MovetextCursor" uses the same
location model, but it edits the tree owned by @ref scid::core::Game "Game".

The traversal model is built around a cursor position between moves.  The cursor's
@ref scid::core::GameCursor::previousMove "previousMove()" is immediately before
that position, and @ref scid::core::GameCursor::nextMove "nextMove()" is
immediately after it.  Entering a variation records a parent-frame and moves the
cursor to the start of the child line; exiting restores the parent line and the
same between-moves position that opened the branch.

@ref scid::core::MovetextLocation "MovetextLocation" is the portable bookmark.
It stores a path of variation steps from the mainline plus the
@ref scid::core::MovetextLocation::nextIndex "nextIndex" in the active line.
Restoring a location validates that every indexed move and variation still
exists before the cursor state is changed.

@startuml core-traversal
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

left to right direction

rectangle "Game state\n\n- <font:Source Code Pro>Game</font>\n- <font:Source Code Pro>Movetext</font>\n- <font:Source Code Pro>Position</font>" as GameState #FFF7ED

rectangle "Cursor position\n\n- <font:Source Code Pro>currentLine</font>\n- <font:Source Code Pro>nextIndex</font>\n- <font:Source Code Pro>parent frames</font>" as CursorPosition #EEF2FF

rectangle "Read-only traversal\n\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>previousMove</font>\n- <font:Source Code Pro>nextMove</font>\n- <font:Source Code Pro>currentPosition</font>" as ReadOnly #EEF2FF

rectangle "Mutable editing\n\n- <font:Source Code Pro>MovetextCursor</font>\n- <font:Source Code Pro>addMove</font>\n- <font:Source Code Pro>addVariation</font>\n- <font:Source Code Pro>promote / delete / truncate</font>" as Mutable #EEF2FF

rectangle "Stable bookmark\n\n- <font:Source Code Pro>MovetextLocation</font>\n- <font:Source Code Pro>Step</font>\n- <font:Source Code Pro>path</font>\n- <font:Source Code Pro>nextIndex</font>" as Location #EEF2FF

ReadOnly ----> GameState : reads
Mutable ----> GameState : edits

ReadOnly ----> CursorPosition : maintains
Mutable ----> CursorPosition : maintains

CursorPosition ----> Location : capture / restore
Location ----> GameState : validates against tree

ReadOnly ----> GameState : replay to position
@enduml

## Domain Model

This diagram expands traversal into the public cursor and location types, plus
the tree state they point into.  It is intentionally loose: it shows the shared
between-moves model and the important operations programmers use, while omitting
the many convenience predicates that report whether a cursor is at a line,
variation, or game edge.

@ref scid::core::GameCursor "GameCursor" and @ref scid::core::MovetextCursor
"MovetextCursor" both hold a current line, a
@ref scid::core::MovetextLocation::nextIndex "nextIndex", and a stack of parent
frames.  The parent stack is an implementation detail, but it is the key to
understanding variation traversal: each frame remembers the parent line, the
move that owns the selected variation, and the selected variation index.
@ref scid::core::MovetextLocation "MovetextLocation" stores the same idea
without pointers, so it can be captured from either cursor and later restored if
the tree shape still matches.

The read-only cursor is the only traversal type that reconstructs board state:
it collects @ref scid::core::GameCursor::movesToCursor "movesToCursor()" and
applies each stored @ref scid::core::MoveSpec "MoveSpec" from the game's start
position.  The mutable cursor instead concentrates on structural edits:
inserting moves, attaching or deleting variations, changing comments and NAGs,
promoting variations, and truncating lines.

@startuml core-traversal-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

hide circle
hide empty methods

left to right direction

package "Game state" {
  class "Game" as DetailGame <<aggregate root>> #FFF7ED
  class "Movetext" as DetailMovetext <<tree root>>
  class "Position" as DetailPosition <<board state>>
}

package "Movetext tree" {
  class "MoveSequence" as DetailMoveSequence {
    moves : vector<Move>
  }

  class "Move" as DetailMove {
    spec : MoveSpec
    san : string
    metadata : MoveMetadata
    childVariations : vector<Variation>
  }

  class "Variation" as DetailVariation {
    initialComment : string
    line : MoveSequence
  }
}

package "Cursor state" {
  class "ParentFrame" as DetailParentFrame <<private per cursor>> {
    line : MoveSequence*
    nextIndex : size_t
    variationIndex : size_t
  }

  class "MovetextLocation" as DetailLocation <<bookmark>> {
    path : vector<Step>
    nextIndex : size_t
  }

  class "Step" as DetailStep {
    nextIndex : size_t
    variationIndex : size_t
  }
}

package "Traversal API" {
  class "GameCursor" as DetailGameCursor <<read-only>> {
    previousMove()
    nextMove()
    enterVariation(index)
    exitVariation()
    movesToCursor()
    currentPosition()
    location()
    restore(location)
  }

  class "MovetextCursor" as DetailMovetextCursor <<mutable>> {
    previousMove()
    nextMove()
    addMove(spec)
    addVariation(comment)
    setComment(comment)
    addPreviousMoveNag(nag)
    promoteVariationToMainline()
    deleteVariation()
    truncate()
    location()
    restore(location)
  }
}

DetailGame *---> DetailMovetext
DetailGame o---> DetailPosition : <font:Source Code Pro>start</font>

DetailMovetext *---> DetailMoveSequence : <font:Source Code Pro>mainline</font>
DetailMoveSequence *---> "0..*" DetailMove
DetailMove *---> "0..*" DetailVariation
DetailVariation *---> DetailMoveSequence : recursive line

DetailGameCursor ---> DetailGame
DetailMovetextCursor ---> DetailGame

DetailGameCursor *---> "0..*" DetailParentFrame
DetailMovetextCursor *---> "0..*" DetailParentFrame
DetailParentFrame ---> DetailMoveSequence : parent line

DetailGameCursor ...> DetailLocation : capture / restore
DetailMovetextCursor ...> DetailLocation : capture / restore
DetailLocation *---> "0..*" DetailStep

DetailGameCursor ...> DetailPosition : replay result
@enduml
