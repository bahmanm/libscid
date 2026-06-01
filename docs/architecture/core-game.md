# Game {#architecture_core_game}

`scid::core::Game` is the editable aggregate for one chess game.  It owns the
structured header, the optional non-standard start position, and the recursive
movetext tree.  Read-only traversal, mutable editing, PGN import/export, and
position replay all work through that aggregate rather than owning separate game
state.

## Aggregate Root: `Game`

The aggregate view is the compact map of the game boundary.  Text formats such
as PGN and notation sit outside the aggregate; cursors provide traversal and
editing; the `Game` root owns header data, movetext, and any non-standard start
position needed to replay the game.

@startuml core-game
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName "Atkinson Hyperlegible Next"
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #F9FAFB

left to right direction

rectangle "Text boundaries\n\n- <font:Atkinson Hyperlegible Mono>pgn::decode</font>\n- <font:Atkinson Hyperlegible Mono>pgn::encode</font>\n- <font:Atkinson Hyperlegible Mono>notation</font>" as TextBoundaries #EEF2FF

rectangle "Traversal and editing\n\n- <font:Atkinson Hyperlegible Mono>GameCursor</font>\n- <font:Atkinson Hyperlegible Mono>MovetextCursor</font>\n- <font:Atkinson Hyperlegible Mono>MovetextLocation</font>" as Traversal #EEF2FF

rectangle "Aggregate root\n\n- <font:Atkinson Hyperlegible Mono>Game</font>" as Game #ECFDF5

rectangle "Game header\n\n- <font:Atkinson Hyperlegible Mono>GameHeader</font>\n- <font:Atkinson Hyperlegible Mono>EventInfo</font>\n- <font:Atkinson Hyperlegible Mono>Player</font>\n- <font:Atkinson Hyperlegible Mono>Rating</font>\n- <font:Atkinson Hyperlegible Mono>TagPair</font>" as Header #FFF7ED

rectangle "Movetext tree\n\n- <font:Atkinson Hyperlegible Mono>Movetext</font>\n- <font:Atkinson Hyperlegible Mono>MoveSequence</font>\n- <font:Atkinson Hyperlegible Mono>Move</font>\n- <font:Atkinson Hyperlegible Mono>Variation</font>" as MovetextTree #FFF7ED

rectangle "Position context\n\n- <font:Atkinson Hyperlegible Mono>Position</font>" as PositionContext #FFF7ED

TextBoundaries ----> Game : parse / format
Traversal ----> Game : navigate / edit

Game ----> Header : owns
Game ----> MovetextTree : owns
Game ----> PositionContext : owns when needed
@enduml

## Domain Model

This diagram expands the aggregate map into the main public types that make up
the game model.  It is intentionally loose: it shows ownership, recursion and
API boundaries, while omitting most operations and low-level chess primitives.
Header data is structured first, with supplemental PGN tags kept beside the
typed fields rather than replacing them.

`Movetext` is recursive: the mainline is a `MoveSequence`, each `Move` can own
child `Variation` objects, and each variation owns another `MoveSequence`.
`GameCursor` and `MovetextCursor` are bound to a `Game`; they do not own an
independent copy of the movetext.  PGN and notation remain boundaries around the
aggregate: they parse into, encode from, or interpret the game state instead of
being part of the aggregate.

@startuml core-game-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName "Atkinson Hyperlegible Next"
skinparam defaultFontSize 10
skinparam classFontName "Atkinson Hyperlegible Mono"
skinparam classAttributeFontName "Atkinson Hyperlegible Mono"
skinparam roundcorner 8
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #F9FAFB
skinparam classAttributeIconSize 0

hide circle
hide empty methods

left to right direction

package "Aggregate root" #ECFDF5 {
  class "Game" as DetailGame <<aggregate root>> {
    - header_ : GameHeader
    - movetext_ : Movetext
    - startPosition_ : optional<Position>
  }
}

package "Game Header" #FFF7ED {
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

package "Movetext tree" #FFF7ED {
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

package "Position and navigation" #EEF2FF {
  class "Position" as DetailPosition <<board state>>
  class "GameCursor" as DetailGameCursor <<read-only>>
  class "MovetextCursor" as DetailMovetextCursor <<mutable>>
  class "MovetextLocation" as DetailLocation <<bookmark>>
}

package "Text boundaries" #EEF2FF {
  class "pgn::parseGame" as DetailPgnDecode <<function>>
  class "pgn::encode" as DetailPgnEncode <<function>>
  class "notation::*" as DetailNotation <<functions>>
}

DetailGame *---> DetailGameHeader
DetailGame *---> DetailMovetext
DetailGame o---> DetailPosition : <font:Atkinson Hyperlegible Mono>start</font>

DetailGameHeader *---> DetailEventInfo
DetailGameHeader *---> "2" DetailPlayer : <font:Atkinson Hyperlegible Mono>white / black</font>
DetailPlayer *---> DetailRating
DetailGameHeader *---> "0..*" DetailTagPair

DetailMovetext *---> DetailMoveSequence : <font:Atkinson Hyperlegible Mono>mainline</font>
DetailMoveSequence *---> "0..*" DetailMove
DetailMove *---> DetailMoveSpec
DetailMove *---> DetailMoveMetadata
DetailMoveMetadata *---> "0..*" DetailNag
DetailMove *---> "0..*" DetailVariation
DetailVariation *---> DetailMoveSequence : recursive line

DetailGameCursor ---> DetailGame
DetailMovetextCursor ---> DetailGame
DetailGameCursor ...> DetailLocation
DetailMovetextCursor ...> DetailLocation
DetailGameCursor ...> DetailPosition

DetailPgnDecode ---> DetailGame
DetailPgnEncode ...> DetailGame
DetailNotation ...> DetailGame
DetailNotation ...> DetailLocation
DetailNotation ...> DetailPosition
@enduml
