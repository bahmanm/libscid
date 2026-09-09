# Filters and Views {#architecture_database_filters_views}

Filters and views are the database layer's way to look at a subset of the
metadata catalog without loading complete games.  A filter answers "which games
are visible?" and can also carry a small per-game value.  A view takes that
filtered set and presents it in a stable order for UI pages, search results,
tree statistics or bulk operations.

@ref scid::database::Filter "Filter" is the dense storage: one byte per game,
with zero meaning excluded and non-zero meaning included.  @ref scid::database::HFilter
"HFilter" is the public handle most APIs accept.  It can view a main filter by
itself or the intersection of a main filter and a read-only mask.
@ref scid::database::HFilterInverted "HFilterInverted" is the complementary
range used by OR-style searches that only need games not already included.

@startuml database-filters-views
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 62
skinparam Ranksep 54
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

rectangle "Database session" as Session #EEF2FF

rectangle "Filter handle" as FilterHandle #FFF7ED

rectangle "Filter storage" as FilterStorage #EEF2FF

rectangle "Metadata catalog" as Metadata #EEF2FF

rectangle "Sorted game view" as View #EEF2FF

rectangle "Search and tree workflows" as SearchTree #EEF2FF

Session -[hidden]right- FilterHandle
FilterHandle -[hidden]right- FilterStorage
SearchTree -[hidden]right- Metadata
Metadata -[hidden]right- View
Session -[hidden]down- SearchTree
FilterHandle -[hidden]down- Metadata
FilterStorage -[hidden]down- View

Session --> FilterHandle : creates / resolves
FilterHandle --> FilterStorage : reads / writes values
FilterHandle --> Metadata : selects games
Metadata --> View : sort keys
SearchTree --> FilterHandle : writes matches
SearchTree --> Metadata : prefilters
@enduml

The stored filter value is deliberately not just a boolean.  A value of zero
excludes the game.  A value of one includes the game at the start-position
hint.  Larger values encode `ply + 1`, so a position search can remember where
the match occurred and a tree view can reopen the game at the relevant point.
When the filter is all included at value one, @ref scid::database::Filter
"Filter" uses a lazy representation and does not allocate the byte array.

## Domain Model

This diagram expands filters and list views into the public types programmers
meet.  It is intentionally loose: it shows how filter handles, filter storage
and sorted result pages relate, without exposing the private sort-cache
implementation.

@ref scid::database::scidBaseT "scidBaseT" owns the default filter and named
filters.  @ref scid::database::scidBaseT::newFilter "newFilter()" creates a
named filter, @ref scid::database::scidBaseT::getFilter "getFilter()" resolves
IDs such as `dbfilter`, `all`, and IDs returned by `newFilter()`, and
@ref scid::database::scidBaseT::composeFilter "composeFilter()" creates an
intersection ID of the form `+main+mask`.

@ref scid::database::HFilter "HFilter" is pointer-like because many call sites
treat it as a nullable handle.  Iterating `*filter` yields included game
numbers through @ref scid::database::HFilter::const_iterator "HFilter::const_iterator";
@ref scid::database::HFilter::get "get()" returns the visible byte value.
Mutating operations update only the main filter, even when a mask is present.

Sorted views are produced by @ref scid::database::scidBaseT::listGames
"listGames()" and @ref scid::database::scidBaseT::sortedPosition
"sortedPosition()".  They combine an @ref scid::database::HFilter "HFilter"
with the resident metadata catalog and a compact sort criterion string.
Retained sort caches make repeated paging cheap, but callers still see only
game numbers suitable for @ref scid::database::scidBaseT::gameInfo "gameInfo()",
@ref scid::database::scidBaseT::tagRoster "tagRoster()" or
@ref scid::database::scidBaseT::loadGame "loadGame()".

@startuml database-filters-views-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 90
skinparam Ranksep 90
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 120
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

left to right direction

class "scidBaseT" as DetailSession <<session facade>> #EEF2FF {
  newFilter()
  getFilter()
  composeFilter()
  listGames()
  sortedPosition()
}

class "HFilter" as DetailHFilter <<filter handle>> #FFF7ED {
  get()
  set()
  insert_or_assign()
  includeAll()
  size()
}

class "Filter" as DetailFilter <<byte storage>> #EEF2FF {
  Get()
  Set()
  Fill()
  Count()
}

class "HFilter::const_iterator" as DetailIterator <<included games>> #EEF2FF

class "HFilterInverted" as DetailInverted <<excluded games>> #EEF2FF

class "IndexEntry" as DetailIndexEntry <<sort/filter metadata>> #EEF2FF

class "Sorted page" as DetailSortedPage <<view result>> #EEF2FF {
  game numbers
  start / count
}

DetailSession -[hidden]right- DetailHFilter
DetailHFilter -[hidden]right- DetailFilter
DetailHFilter -[hidden]down- DetailIterator
DetailIterator -[hidden]right- DetailSortedPage

DetailSession --> DetailHFilter : resolves
DetailHFilter --> DetailFilter : main + mask
DetailHFilter --> DetailIterator : iterates included
DetailInverted --> DetailIterator : iterates excluded

DetailHFilter --> DetailSortedPage : selects
DetailIndexEntry --> DetailSortedPage : sort keys
DetailSession --> DetailSortedPage : list / locate
@enduml
