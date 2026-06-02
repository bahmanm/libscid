# Storage Codecs {#architecture_database_storage_codecs}

Storage codecs are the database layer's persistence adapters.  Callers work
through @ref scid::database::scidBaseT "scidBaseT"; the session owns the
resident @ref scid::database::Index "Index" and
@ref scid::database::NameBase "NameBase", while the active codec knows how to
read and write the concrete storage format.  That split lets listing, sorting,
filtering and metadata queries stay format-agnostic once a database is open.

The public save path starts with @ref scid::core::Game "Game".  The database
storage encoder turns it into three pieces: an
@ref scid::database::IndexEntry "IndexEntry" for compact metadata,
@ref scid::database::TagRoster "TagRoster" for the standard PGN names, and a
byte buffer containing the encoded game body.  The codec then resolves roster
names through the name registry, writes or updates the encoded game record, and
stores the resulting offsets and name IDs back into the index entry.

@startuml database-storage-codecs
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

rectangle "Database session" as Session #EEF2FF
rectangle "Storage codec" as Codec #FFF7ED
rectangle "Index metadata" as IndexMetadata #EEF2FF
rectangle "Name registry" as NameRegistry #EEF2FF
rectangle "Encoded game data" as GameData #EEF2FF
rectangle "Database files" as Files #EEF2FF

Session -[hidden]right- Codec
Codec -[hidden]right- GameData
IndexMetadata -[hidden]right- NameRegistry
NameRegistry -[hidden]right- Files
Codec -[hidden]down- NameRegistry

Session --> Codec : load / save
Codec --> IndexMetadata : index entries
Codec --> NameRegistry : roster names
Codec --> GameData : game bytes
IndexMetadata --> Files : .si4 / .si5
NameRegistry --> Files : .sn4 / .sn5
GameData --> Files : .sg4 / .sg5
@enduml

Loading reverses the same boundary.  @ref scid::database::scidBaseT::loadGame
"loadGame()" reads an @ref scid::database::IndexEntry "IndexEntry", asks the
codec for the encoded bytes at the stored offset and length, resolves standard
tag strings through @ref scid::database::TagRoster "TagRoster", and decodes the
result into a @ref scid::core::Game "Game".  Move-only and game-view paths use
the same encoded record but skip work they do not need.

Native codecs keep the index, namebase and game data in their native form.
For SCID5, the file triplet is `.si5` for index records, `.sg5` for encoded
game records and `.sn5` for names plus database information.  SCID4 follows
the older analogous split.  Memory storage implements the same codec contract
without files, which also gives non-native formats a resident database model to
populate.

## Domain Model

This diagram shows the storage seam as programmers experience it from the
database API.  `ICodecDatabase` and the concrete codec classes are internal
implementation types, so the diagram names them conceptually rather than
making them part of the public surface.  The public values crossing the seam
are @ref scid::core::Game "Game",
@ref scid::database::IndexEntry "IndexEntry",
@ref scid::database::TagRoster "TagRoster" and encoded bytes.

@startuml database-storage-codecs-domain-model
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
  open()
  loadGame()
  saveGame()
  importGames()
}

class "Game" as DetailGame <<editable game>> #EEF2FF

class "game_storage" as DetailEncoder <<format mapper>> #EEF2FF {
  encode()
  decode()
}

class "ICodecDatabase" as DetailCodec <<storage adapter>> #FFF7ED {
  getGameData()
  addGame()
  saveGame()
  saveIndexEntry()
  addName()
  flush()
}

class "IndexEntry" as DetailIndexEntry <<metadata record>> #EEF2FF

class "TagRoster" as DetailTagRoster <<standard names>> #EEF2FF

class "ByteBuffer" as DetailByteBuffer <<encoded game>> #EEF2FF

class "CodecSCID5" as DetailScid5 <<native files>> #EEF2FF
class "CodecSCID4" as DetailScid4 <<legacy files>> #EEF2FF
class "CodecMemory" as DetailMemory <<resident store>> #EEF2FF
class "CodecProxy" as DetailProxy <<non-native bridge>> #EEF2FF

DetailSession -[hidden]right- DetailEncoder
DetailEncoder -[hidden]right- DetailCodec
DetailIndexEntry -[hidden]down- DetailTagRoster
DetailTagRoster -[hidden]right- DetailByteBuffer
DetailCodec -[hidden]down- DetailScid5
DetailScid5 -[hidden]right- DetailScid4
DetailScid4 -[hidden]right- DetailMemory
DetailMemory -[hidden]down- DetailProxy

DetailSession --> DetailGame : exchanges
DetailSession --> DetailEncoder : encode / decode
DetailEncoder --> DetailIndexEntry : metadata
DetailEncoder --> DetailTagRoster : roster names
DetailEncoder --> DetailByteBuffer : game bytes
DetailIndexEntry --> DetailCodec : save / load
DetailTagRoster --> DetailCodec : names
DetailByteBuffer --> DetailCodec : encoded record
DetailCodec <|.. DetailScid5
DetailCodec <|.. DetailScid4
DetailCodec <|.. DetailMemory
DetailMemory <|-- DetailProxy
@enduml
