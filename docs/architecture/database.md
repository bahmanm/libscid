# Database {#architecture_database}

Database is the persistence and query layer above Core.  It stores chess games
in database formats, keeps searchable metadata resident in memory, resolves
stored name identifiers, and materialises editable
@ref scid::core::Game "scid::core::Game" values when callers need full game
content.

@startuml database-session
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

rectangle "Database session" as Session #FFF7ED

rectangle "Metadata layer" as Metadata #EEF2FF

rectangle "Storage codec" as Codec #EEF2FF

rectangle "Database files" as Files #EEF2FF

CoreGame -[hidden]right- Session
Session -[hidden]right- Metadata
Codec -[hidden]right- Files
Session -[hidden]down- Codec

CoreGame --> Session : exchanges
Session --> Metadata : indexes
Metadata --> Codec : locates
Codec --> Files : persists
@enduml

---

@ref scid::database::scidBaseT "scid::database::scidBaseT" is the session
facade.  An opened session binds together the active storage codec, the
in-memory index, the namebase, working filters, sort caches, duplicate state,
and derived statistics.  Application code usually starts here rather than
constructing lower-level storage objects.

The metadata layer is deliberately cheap to query.
@ref scid::database::Index "Index" stores one
@ref scid::database::IndexEntry "IndexEntry" per game, with dates, players,
event/site/round handles, result, ratings, flags, ECO, material signatures,
stored offsets, and encoded data lengths.
@ref scid::database::NameBase "NameBase" owns the player, event, site, and
round strings referenced by those handles.
@ref scid::database::GameInfo "GameInfo" is the value snapshot callers can copy
out of the session without holding an index pointer.

The codec is the private storage mapping.  It knows how to open memory, PGN,
SCID4, or SCID5 representations and fill the session's
@ref scid::database::Index "Index" and
@ref scid::database::NameBase "NameBase".  For SCID5, the native storage unit
is the `.si5`, `.sg5`, and `.sn5` file triplet: index records, encoded game
blobs, and namebase/database metadata.

Full game content is loaded on demand.  Listing, sorting, filtering, flag
updates, many metadata edits, and search prefilters can work from the index and
namebase alone.  Loading or saving a complete game crosses into Core: the codec
decodes stored bytes into @ref scid::core::Game "scid::core::Game", or encodes
a @ref scid::core::Game "Game" back into the active database format.

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

@startuml database-session-domain-model
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
