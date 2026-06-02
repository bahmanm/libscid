# Database {#architecture_database}

Database is the persistence and query layer above Core.  It stores chess games
in database formats, keeps searchable metadata resident in memory, resolves
stored name identifiers, and materialises editable
@ref scid::core::Game "scid::core::Game" values when callers need full game
content.

@startuml database-overview
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

rectangle "Database metadata" as Metadata #FFF7ED

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

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage architecture_database_scidbase "scidBaseT"
- @subpage architecture_database_metadata "Database Metadata"

\htmlonly
</div>
\endhtmlonly

---

@ref architecture_database_scidbase "scidBaseT" is the session facade.  It owns
the open database state, coordinates codecs, filters, sort caches and metadata,
and is the usual public entry point for database work.

---

@ref architecture_database_metadata "Database metadata" is the resident catalog
behind browsing, filtering, sorting and metadata-only updates.  It lets callers
answer many database questions without decoding a complete
@ref scid::core::Game "Game".

---

Storage codecs map the session and metadata catalog to concrete database
formats.  For SCID5, the native storage unit is the `.si5`, `.sg5`, and `.sn5`
file triplet: index records, encoded game blobs, and namebase/database
metadata.

---

Full game content is loaded on demand.  Listing, sorting, filtering, flag
updates, many metadata edits, and search prefilters can work from metadata
alone.  Loading or saving a complete game crosses into Core: the codec decodes
stored bytes into @ref scid::core::Game "scid::core::Game", or encodes a
@ref scid::core::Game "Game" back into the active database format.
