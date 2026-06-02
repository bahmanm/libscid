# Name Registry {#architecture_database_name_registry}

The name registry is the database layer's string table for game identity
fields.  Player, event, site and round names are stored once in
@ref scid::database::NameBase "NameBase"; metadata rows keep compact
`idNumberT` handles into the appropriate `nameT` bucket.  This keeps the index
small while still letting callers resolve human-readable names when they need
them.

@ref scid::database::TagRoster "TagRoster" is the narrow bridge between
database IDs and PGN's Seven Tag Roster names.  When loading or exporting a
game, @ref scid::database::scidBaseT::tagRoster "tagRoster()" resolves an
@ref scid::database::IndexEntry "IndexEntry" through the current
@ref scid::database::NameBase "NameBase" and returns borrowed string pointers.
When saving, @ref scid::database::TagRoster::map "TagRoster::map()" maps those
strings back to IDs, adding names when the storage backend allows it.

@startuml database-name-registry
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

rectangle "Core game header" as CoreHeader #EEF2FF
rectangle "Tag roster" as TagRoster #EEF2FF
rectangle "Name registry" as NameRegistry #FFF7ED
rectangle "Index metadata" as IndexMetadata #EEF2FF
rectangle "Search and sort views" as Views #EEF2FF
rectangle "Namebase file" as NameFile #EEF2FF

CoreHeader -[hidden]right- TagRoster
TagRoster -[hidden]right- NameRegistry
NameRegistry -[hidden]right- IndexMetadata
Views -[hidden]right- NameFile
NameRegistry -[hidden]down- Views
IndexMetadata -[hidden]down- NameFile

CoreHeader --> TagRoster : standard names
TagRoster --> NameRegistry : resolve / add
NameRegistry --> IndexMetadata : ids for metadata
NameRegistry --> Views : display / sort keys
NameRegistry --> NameFile : persists strings
@enduml

A name ID is not globally meaningful.  It is meaningful only together with its
bucket: player ID 7, event ID 7 and round ID 7 point into different
@ref scid::database::NameBase "NameBase" tables.  Public value snapshots such
as @ref scid::database::GameInfo "GameInfo" preserve that compact form, so
callers that want strings either call
@ref scid::database::scidBaseT::tagRoster "tagRoster()" for one game or read
the registry through @ref scid::database::scidBaseT::getNameBase
"getNameBase()".

The registry owns the returned strings.  Pointers from
@ref scid::database::NameBase::GetName "GetName()" and
@ref scid::database::TagRoster "TagRoster" remain borrowed views into the
current database namebase; they are not independent strings and should not be
kept past the lifetime of the database state that produced them.

## Domain Model

This diagram shows the public string/ID boundary.  It leaves storage-codec
details loose: SCID4 and SCID5 persist names differently, but the in-memory
contract is the same once the database is open.

@ref scid::database::NameBase "NameBase" owns the four name buckets and their
legacy-compatible lookup order.  @ref scid::database::IndexEntry "IndexEntry"
and @ref scid::database::GameInfo "GameInfo" store IDs.  @ref scid::database::TagRoster
"TagRoster" exposes borrowed strings for Event, Site, Round, White and Black.
@ref scid::database::scidBaseT::getNameFreq "getNameFreq()" is a derived view:
it asks the registry to scan the current index and count how often each name ID
is referenced.

@startuml database-name-registry-domain-model
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
  tagRoster()
  getNameBase()
  getNameFreq()
}

class "IndexEntry" as DetailIndexEntry <<metadata row>> #EEF2FF {
  GetWhite()
  GetBlack()
  GetEvent()
  GetSite()
  GetRound()
}

class "NameBase" as DetailNameBase <<string registry>> #FFF7ED {
  namebase_find_or_add()
  GetName()
  FindExactName()
  getFirstMatches()
  calcNameFreq()
}

class "TagRoster" as DetailTagRoster <<borrowed roster>> #EEF2FF {
  event
  site
  round
  white
  black
  make()
  map()
}

class "GameInfo" as DetailGameInfo <<value snapshot>> #EEF2FF {
  white
  black
  event
  site
  round
}

class "nameT + idNumberT" as DetailNameId <<bucketed handle>> #EEF2FF

DetailSession -[hidden]right- DetailIndexEntry
DetailIndexEntry -[hidden]right- DetailNameId
DetailNameId -[hidden]right- DetailNameBase
DetailNameBase -[hidden]right- DetailTagRoster
DetailIndexEntry -[hidden]down- DetailGameInfo

DetailSession --> DetailIndexEntry : reads metadata
DetailIndexEntry --> DetailNameId : stores ids
DetailGameInfo --> DetailNameId : copies ids
DetailNameBase --> DetailNameId : assigns / finds
DetailNameId --> DetailNameBase : resolves
DetailNameBase --> DetailTagRoster : borrowed strings
DetailTagRoster --> DetailNameBase : maps strings
DetailSession --> DetailTagRoster : returns roster
@enduml
