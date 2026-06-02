# scidBaseT {#architecture_database_scidbase}

@ref scid::database::scidBaseT "scid::database::scidBaseT" is the database
session facade.  An opened session binds together the active storage codec, the
in-memory index, the namebase, working filters, sort caches, duplicate state,
and derived statistics.  Application code usually starts here rather than
constructing lower-level storage objects.

The session is also the boundary between metadata-only operations and full game
materialisation.  Listing, sorting, filtering, flag updates, many metadata edits
and search prefilters work from the index and namebase.  Loading or saving a
complete game crosses into Core through @ref scid::core::Game "scid::core::Game".

@startuml database-scidbase
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

rectangle "Core game values" as CoreGame #EEF2FF

rectangle "Database session\n\nscidBaseT" as Session #FFF7ED

rectangle "Database metadata" as Metadata #EEF2FF

rectangle "Storage codec" as Codec #EEF2FF

rectangle "Filters and views" as Views #EEF2FF

CoreGame -[hidden]right- Session
Session -[hidden]right- Metadata
Codec -[hidden]right- Views
Session -[hidden]down- Codec

CoreGame --> Session : load / save
Session --> Metadata : owns
Session --> Codec : delegates storage
Session --> Views : owns / composes
Views --> Metadata : query
@enduml

## Domain Model

This diagram expands the session view into the public types programmers meet
first, plus the private codec role that explains where persistent data lives.
It is intentionally loose: it shows the main navigation paths, not every
operation on @ref scid::database::scidBaseT "scidBaseT".

@ref scid::database::scidBaseT "scidBaseT" owns the open-session state.
@ref scid::database::IndexEntry "IndexEntry" is the compact record used for
fast database views; @ref scid::database::NameBase "NameBase" turns the name IDs
in that record into strings.  @ref scid::database::HFilter "HFilter" is a handle
over filter storage, and sort/statistics caches are derived from the index.
Codecs are implementation details: consumers see the session API and Core
@ref scid::core::Game "Game" values, not codec objects.

@startuml database-scidbase-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 72
skinparam Ranksep 76
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 110
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

class "core::Game" as DetailGame <<editable game>> #EEF2FF

class "HFilter" as DetailHFilter <<query view>> #EEF2FF

class "scidBaseT" as DetailScidBase <<session facade>> #FFF7ED {
  open()
  gameInfo()
  loadGame()
  saveGame()
  listGames()
}

class "ICodecDatabase" as DetailCodec <<storage mapping>> #EEF2FF

class "Index" as DetailIndex #EEF2FF
class "IndexEntry" as DetailIndexEntry <<compact record>> #EEF2FF
class "NameBase" as DetailNameBase #EEF2FF

DetailScidBase -up-> DetailGame : load / save
DetailScidBase -down-> DetailIndex : metadata
DetailIndex -down-> "0..*" DetailIndexEntry : entries
DetailIndexEntry -down-> DetailNameBase : name ids
DetailScidBase -left-> DetailHFilter : filters
DetailScidBase -right-> DetailCodec : storage
@enduml
