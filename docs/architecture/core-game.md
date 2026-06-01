# Game {#architecture_core_game}

@ref scid::core::Game "scid::core::Game" is the editable aggregate for one chess
game.  It owns the structured header, the optional non-standard start position,
and the recursive movetext tree.  Read-only traversal, mutable editing, PGN
import/export, and position replay all work through that aggregate rather than
owning separate game state.

The aggregate view is the compact map of the game-facing API.  Text formats such
as PGN and notation sit outside the aggregate; cursors provide traversal and
editing; the @ref scid::core::Game "Game" root owns header data, movetext, and
any non-standard start position needed to replay the game.

@startuml core-game
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

rectangle "Text format mappings\n\n- <font:Source Code Pro>pgn::decode</font>\n- <font:Source Code Pro>pgn::encode</font>\n- <font:Source Code Pro>notation</font>" as TextMappings #EEF2FF

rectangle "Traversal and editing\n\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MovetextCursor</font>\n- <font:Source Code Pro>MovetextLocation</font>" as Traversal #EEF2FF

rectangle "Aggregate root\n\n- <font:Source Code Pro>Game</font>" as Game #FFF7ED

rectangle "Game header\n\n- <font:Source Code Pro>GameHeader</font>\n- <font:Source Code Pro>EventInfo</font>\n- <font:Source Code Pro>Player</font>\n- <font:Source Code Pro>Rating</font>\n- <font:Source Code Pro>TagPair</font>" as Header #EEF2FF

rectangle "Movetext tree\n\n- <font:Source Code Pro>Movetext</font>\n- <font:Source Code Pro>MoveSequence</font>\n- <font:Source Code Pro>Move</font>\n- <font:Source Code Pro>Variation</font>" as MovetextTree #EEF2FF

rectangle "Position context\n\n- <font:Source Code Pro>Position</font>" as PositionContext #EEF2FF

TextMappings ----> Game : parse / format
Traversal ----> Game : navigate / edit

Game ----> Header : owns
Game ----> MovetextTree : owns
Game ----> PositionContext : owns when needed
@enduml

## Domain Model

This diagram expands the aggregate map into the main public types that make up
the game model.  It is intentionally loose: it shows ownership, recursion and
API edges, while omitting most operations and low-level chess primitives.
Header data is structured first, with supplemental PGN tags kept beside the
typed fields rather than replacing them.

@ref scid::core::Movetext "Movetext" is recursive: the mainline is a
@ref scid::core::MoveSequence "MoveSequence", each @ref scid::core::Move "Move"
can own child @ref scid::core::Variation "Variation" objects, and each variation
owns another @ref scid::core::MoveSequence "MoveSequence".
@ref scid::core::GameCursor "GameCursor" and
@ref scid::core::MovetextCursor "MovetextCursor" are bound to a
@ref scid::core::Game "Game"; they do not own an independent copy of the
movetext.  PGN and notation remain text mappings around the aggregate: they
parse into, encode from, or interpret the game state instead of being part of
the aggregate.

@startuml core-game-domain-model
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

package "Aggregate root" {
  class "Game" as DetailGame <<aggregate root>> #FFF7ED {
    - header_ : GameHeader
    - movetext_ : Movetext
    - startPosition_ : optional<Position>
  }
}

package "Game Header" {
  class "GameHeader" as DetailGameHeader {
    event : EventInfo
    white : Player
    black : Player
    result : resultT
    eco : string
    tags : vector<TagPair>
  }

  class "EventInfo" as DetailEventInfo {
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

  class "TagPair" as DetailTagPair <<PGN tag>>
}

package "Movetext tree" {
  class "Movetext" as DetailMovetext {
    initialComment : string
    mainline : MoveSequence
  }

  class "MoveSequence" as DetailMoveSequence {
    moves : vector<Move>
  }

  class "Move" as DetailMove {
    spec : MoveSpec
    san : string
    metadata : MoveMetadata
    childVariations : vector<Variation>
  }

  class "MoveMetadata" as DetailMoveMetadata {
    nags : vector<Nag>
    comment : string
  }

  class "Variation" as DetailVariation {
    initialComment : string
    line : MoveSequence
  }

  class "MoveSpec" as DetailMoveSpec {
    from : squareT
    to : squareT
    promotion : pieceT
    castling : bool
  }

  class "Nag" as DetailNag
}

package "Position and navigation" {
  class "Position" as DetailPosition <<board state>>
  class "GameCursor" as DetailGameCursor <<read-only>>
  class "MovetextCursor" as DetailMovetextCursor <<mutable>>
  class "MovetextLocation" as DetailLocation <<bookmark>>
}

package "Text format mappings" {
  class "pgn::parseGame" as DetailPgnDecode <<function>>
  class "pgn::encode" as DetailPgnEncode <<function>>
  class "notation::*" as DetailNotation <<functions>>
}

note right of DetailNotation
  Text helpers parse, format,
  and name positions or moves.
end note

note right of DetailGameCursor
  Cursors traverse or edit a Game
  and expose board positions or
  stable movetext bookmarks.
end note

DetailGame *---> DetailGameHeader
DetailGame *---> DetailMovetext
DetailGame o---> DetailPosition : <font:Source Code Pro>start</font>

DetailGameHeader *---> DetailEventInfo
DetailGameHeader *---> "2" DetailPlayer : <font:Source Code Pro>white / black</font>
DetailPlayer *---> DetailRating
DetailGameHeader *---> "0..*" DetailTagPair

DetailMovetext *---> DetailMoveSequence : <font:Source Code Pro>mainline</font>
DetailMoveSequence *---> "0..*" DetailMove
DetailMove *---> DetailMoveSpec
DetailMove *---> DetailMoveMetadata
DetailMoveMetadata *---> "0..*" DetailNag
DetailMove *---> "0..*" DetailVariation
DetailVariation *---> DetailMoveSequence : recursive line

DetailPgnDecode ---> DetailGame : parse / format
@enduml
