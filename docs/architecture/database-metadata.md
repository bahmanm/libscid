# Database Metadata {#architecture_database_metadata}

The metadata catalog is the resident, cheap-to-query side of an open database.
It is not a separate public session: @ref scid::database::scidBaseT "scidBaseT"
owns the catalog and exposes it through value snapshots, name resolution, list
views, filters, searches and metadata updates.

@ref scid::database::Index "Index" maps database game numbers to compact
@ref scid::database::IndexEntry "IndexEntry" records.
@ref scid::database::NameBase "NameBase" owns the player, event, site and round
strings named by those records.  @ref scid::database::GameInfo "GameInfo" is the
copyable public view assembled from an index entry, while
@ref scid::database::TagRoster "TagRoster" is the borrowed Seven Tag Roster view
used when metadata crosses the Core or storage boundary.

@startuml database-metadata
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 58
skinparam Ranksep 54
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

<style>
rectangle {
  Padding 22
  MinimumWidth 145
  MinimumHeight 44
}
</style>

left to right direction

rectangle "Database session" as Session #EEF2FF

rectangle "Metadata catalog" as Catalog #FFF7ED

rectangle "Name strings" as Names #EEF2FF

rectangle "Game list and filters" as Views #EEF2FF

rectangle "Core game values" as CoreGame #EEF2FF

rectangle "Storage codec" as Codec #EEF2FF

Session -[hidden]right- Catalog
Catalog -[hidden]right- Names
Catalog -[hidden]down- Views
CoreGame -[hidden]down- Codec

Session --> Catalog : owns
Codec --> Catalog : fills / rewrites
Catalog --> Names : resolves ids
Catalog --> Views : drives
Catalog <--> CoreGame : tag metadata
@enduml

The catalog lets callers answer many database questions without decoding the
encoded movetext.  Game lists, sort keys, filters, flag updates, name-frequency
statistics, duplicate hints, material prefilters and opening/tree shortcuts all
start from @ref scid::database::IndexEntry "IndexEntry" plus
@ref scid::database::NameBase "NameBase".  Full game loading is still available,
but it is no longer the default cost for browsing a database.

## Domain Model

This diagram expands the catalog into the public types programmers meet.  It is
intentionally loose: it shows the data path from compact storage records to
copyable public snapshots and name strings, while leaving codec-specific file
layout to the storage model.

@ref scid::database::Index "Index" owns the table of entries and keeps returned
entry pointers stable while games are appended.  @ref scid::database::IndexEntry
"IndexEntry" is compact and storage-shaped: names are
`idNumberT` handles, counts are packed buckets, flags are bit masks, and offsets
point into the active game-data store.
@ref scid::database::NameBase "NameBase" gives those handles meaning inside a
name category such as player, event, site or round.

@ref scid::database::GameInfo "GameInfo" is the public value snapshot for UI,
query and service code.  @ref scid::database::GameInfoUpdate "GameInfoUpdate"
is the narrow update shape for metadata-only edits.  @ref scid::database::TagRoster
"TagRoster" bridges between the five standard PGN tag strings stored through the
namebase and the compact IDs stored in an @ref scid::database::IndexEntry
"IndexEntry".

@startuml database-metadata-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 95
skinparam Ranksep 95
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
  gameInfo()
  updateGameInfo()
  tagRoster()
  getNameBase()
  listGames()
}

class "Index" as DetailIndex <<resident table>> #EEF2FF {
  GetEntry()
  GetNumGames()
  addEntry()
  replaceEntry()
}

class "IndexEntry" as DetailIndexEntry <<compact record>> #FFF7ED {
  name ids
  dates / result / ratings
  flags / counts / ECO
  offsets / lengths
  material hints
}

class "NameBase" as DetailNameBase <<string table>> #EEF2FF {
  namebase_find_or_add()
  GetName()
  FindExactName()
  calcNameFreq()
}

class "GameInfo" as DetailGameInfo <<value snapshot>> #EEF2FF {
  name ids
  dates / result / ratings
  flags / counts / ECO
  storage hints
}

class "TagRoster" as DetailTagRoster <<borrowed names>> #EEF2FF {
  event
  site
  round
  white
  black
}

DetailSession -[hidden]right- DetailIndex
DetailIndex -[hidden]right- DetailIndexEntry
DetailIndexEntry -[hidden]right- DetailNameBase
DetailGameInfo -[hidden]down- DetailIndexEntry
DetailIndexEntry -[hidden]down- DetailTagRoster

DetailSession --> DetailIndex : owns
DetailIndex --> "0..*" DetailIndexEntry : stores
DetailIndexEntry --> DetailNameBase : id handles

DetailIndexEntry ..> DetailGameInfo : copied into

DetailIndexEntry --> DetailTagRoster : maps ids
DetailTagRoster --> DetailNameBase : borrowed strings
@enduml
