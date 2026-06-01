# Game Metadata {#architecture_core_game_metadata}

Game metadata is the header side of `scid::core::Game`.  It is not a separate
aggregate root: `Game` owns the `GameHeader`, while `GameHeader` groups the
typed values that identify the event, players, result, ratings, ECO code, and
supplemental PGN tags.

The header tag mapping separates structured fields from free-form PGN tags.  The
Seven Tag Roster fields are represented by typed accessors on `Game`: event,
site, date, round, white, black, and result.  Other known tags are also typed
when libscid can use them directly: `EventDate`, player rating tags, `ECO`, and
the `FEN` tag that creates a non-standard start position.

Supplemental tags remain as `TagPair` values.  This lets the game preserve PGN
metadata that libscid does not interpret, without forcing every tag into the
typed model.  `pgn::parseGame()` reads both sides of that mapping; `pgn::encode()`
writes the Seven Tag Roster every time and writes supplemental tags when the
encode policy allows them.

@startuml core-game-metadata
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #F9FAFB

left to right direction

rectangle "PGN tag pairs\n\n- <font:Source Code Pro>Seven Tag Roster</font>\n- <font:Source Code Pro>EventDate</font>\n- <font:Source Code Pro>WhiteElo / BlackElo</font>\n- <font:Source Code Pro>ECO</font>\n- <font:Source Code Pro>FEN</font>\n- <font:Source Code Pro>custom tags</font>" as PgnTags #EEF2FF

rectangle "Header tag mapping\n\n- <font:Source Code Pro>pgn::parseGame</font>\n- <font:Source Code Pro>Game::addTag</font>\n- <font:Source Code Pro>pgn::encode</font>" as HeaderMapping #EEF2FF

rectangle "Game aggregate\n\n- <font:Source Code Pro>Game</font>\n- <font:Source Code Pro>GameHeader</font>\n- <font:Source Code Pro>startPosition</font>" as Game #ECFDF5

rectangle "Typed metadata\n\n- <font:Source Code Pro>EventInfo</font>\n- <font:Source Code Pro>Player</font>\n- <font:Source Code Pro>Rating</font>\n- <font:Source Code Pro>resultT</font>\n- <font:Source Code Pro>dateT</font>" as Typed #FFF7ED

rectangle "Supplemental metadata\n\n- <font:Source Code Pro>TagPair</font>\n- <font:Source Code Pro>extraTags</font>\n- <font:Source Code Pro>findExtraTag</font>\n- <font:Source Code Pro>removeExtraTag</font>" as Supplemental #FFF7ED

PgnTags ----> HeaderMapping : import
HeaderMapping ----> Game : set fields / tags
Game ----> Typed : owns
Game ----> Supplemental : preserves
Game ----> HeaderMapping : source
HeaderMapping ----> PgnTags : export
@enduml

## Domain Model

This diagram expands the metadata model into the public values programmers
handle.  The important distinction is not whether a value came from PGN; it is
whether libscid understands the value enough to store it in a typed field.

`GameHeader::event` holds the event name, site, round, game date, and event
date.  The game date is the PGN `Date` tag; the event date is the optional
`EventDate` tag.  Both are stored as `dateT`, whose year, month, and day fields
may be partially unknown.  `Player` stores the display name and one `Rating`.
The rating value zero means unknown, and the rating type names are the suffixes
used for tags such as `WhiteElo`, `BlackRapid`, or `WhiteUSCF`.

`Game::addTag()` and `Game::findOrCreateTag()` recognise the simple string
fields `Event`, `Site`, `Round`, `White`, and `Black`.  PGN parsing handles the
other typed metadata explicitly: dates, result, ratings, ECO, and FEN.  Tags
outside those known cases remain in `GameHeader::tags` as supplemental
`TagPair` values and are emitted after the known supplemental tags.

@startuml core-game-metadata-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam nodesep 60
skinparam ranksep 75
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #F9FAFB
skinparam classAttributeIconSize 0

hide circle
hide empty methods

left to right direction

package "Game aggregate" #ECFDF5 {
  class "Game" as DetailGame <<aggregate root>> {
    header() : GameHeader
    event()
    site()
    round()
    white()
    black()
    date()
    eventDate()
    result()
    eco()
    addTag()
    extraTags()
    hasNonStandardStart()
  }

  class "Position" as DetailPosition <<start position>>
}

package "Typed header" #FFF7ED {
  class "GameHeader" as DetailHeader {
    event : EventInfo
    white : Player
    black : Player
    result : resultT
    eco : string
    tags : vector<TagPair>
  }

  class "EventInfo" as DetailEvent {
    name : string
    site : string
    round : string
    date : dateT
    eventDate : dateT
  }

  class "Player" as DetailPlayer {
    name : string
    rating : Rating
  }

  class "Rating" as DetailRating {
    value : ratingT
    type : ratingTypeT
  }

  class "resultT" as DetailResult <<value>>
  class "dateT" as DetailDate <<value>>
}

package "Supplemental tags" #FFF7ED {
  class "TagPair" as DetailTagPair <<PGN tag>> {
    first : string
    second : string
  }
}

package "PGN import/export mapping" #EEF2FF {
  class "PGN tag mapping" as DetailPgnMapping <<functions>> {
    parse tag pairs
    set typed fields
    preserve extras
    write Seven Tag Roster
    write supplemental tags
    write FEN when needed
  }
}

DetailGame *---> DetailHeader
DetailGame o---> DetailPosition : <font:Source Code Pro>FEN start</font>

DetailHeader *---> DetailEvent
DetailHeader *---> "2" DetailPlayer : <font:Source Code Pro>white / black</font>
DetailHeader *---> "0..*" DetailTagPair
DetailHeader ...> DetailResult
DetailHeader ...> DetailDate
DetailEvent ...> DetailDate
DetailPlayer *---> DetailRating

DetailPgnMapping ---> DetailGame : import / export
DetailPgnMapping ...> DetailTagPair : unknown / extra tags
@enduml
