# Position {#architecture_core_position}

@ref scid::core::Position "scid::core::Position" is the mutable board-state
aggregate.  It owns the board array, piece and material indexes, side to move,
castling rights, en-passant target, move counters, and hash values.  Move
generation, legality checks, move application, undo, FEN/UCI parsing, and SAN
formatting all resolve through that state.

The aggregate view is the compact map of the position-facing API.  Text formats
such as FEN, UCI position strings, coordinate moves, SAN, and legacy board
strings sit outside the aggregate.  @ref scid::core::Position "Position" turns
portable move requests into position-resolved actions, updates its internal
indexes when moves are applied or undone, and exposes derived facts such as
legal replies, check, mate, material and hashes.

@startuml core-position
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

rectangle "Text format mappings\n\n- <font:Source Code Pro>FEN / EPD</font>\n- <font:Source Code Pro>UCI position</font>\n- <font:Source Code Pro>SAN / coordinate moves</font>\n- <font:Source Code Pro>legacy board strings</font>" as TextMappings #EEF2FF

rectangle "Move requests and actions\n\n- <font:Source Code Pro>MoveSpec</font>\n- <font:Source Code Pro>MoveAction</font>\n- <font:Source Code Pro>MoveList</font>" as MoveRequests #EEF2FF

rectangle "Aggregate root\n\n- <font:Source Code Pro>Position</font>" as Position #FFF7ED

rectangle "Board state\n\n- <font:Source Code Pro>Board</font>\n- <font:Source Code Pro>List</font>\n- <font:Source Code Pro>Material</font>\n- <font:Source Code Pro>Pinned</font>" as BoardState #EEF2FF

rectangle "Rule state\n\n- <font:Source Code Pro>ToMove</font>\n- <font:Source Code Pro>Castling</font>\n- <font:Source Code Pro>EPTarget</font>\n- <font:Source Code Pro>HalfMoveClock</font>\n- <font:Source Code Pro>PlyCounter</font>" as RuleState #EEF2FF

rectangle "Derived indexes\n\n- <font:Source Code Pro>Hash</font>\n- <font:Source Code Pro>PawnHash</font>\n- <font:Source Code Pro>rank/file/diagonal counts</font>\n- <font:Source Code Pro>SquareList</font>\n- <font:Source Code Pro>SquareSet</font>" as DerivedState #EEF2FF

TextMappings ----> Position : parse / format
MoveRequests ----> Position : resolve / apply / undo

Position ----> BoardState : owns
Position ----> RuleState : owns
Position ----> DerivedState : owns and maintains
@enduml

## Domain Model

This diagram expands the position map into the main public types that sit around
board state and legal move handling.  It is intentionally loose: it shows the
types programmers meet when using @ref scid::core::Position "Position", while
omitting private helper functions, attack tables and most low-level board
geometry constants.

@ref scid::core::MoveSpec "MoveSpec" is a portable request: it names origin,
destination, promotion, and castling intent, but it cannot undo a move.
@ref scid::core::Position "Position" resolves that request against the current
board into a @ref scid::core::MoveAction "MoveAction", which carries captured
pieces, castling rights, en-passant state and halfmove data needed for exact
undo.  @ref scid::core::MoveList "MoveList" is the fixed-capacity container
filled by move generation.

Square collections are supporting structures rather than independent
aggregates.  @ref scid::core::SquareList "SquareList" is useful when callers
need the actual attacking or checking squares; @ref scid::core::SquareSet
"SquareSet" is a compact membership filter used while generating moves.
Primitive scalar aliases such as @ref scid::core::pieceT "pieceT",
@ref scid::core::colorT "colorT" and @ref scid::core::squareT "squareT" appear
as field types rather than separate model nodes.

@startuml core-position-domain-model
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
  class "Position" as DetailPosition <<aggregate root>> #FFF7ED {
    - Board : pieceT[66]
    - List : squareT[2][16]
    - Material : byte[16]
    - Pinned : directionT[16]
    - EPTarget : squareT
    - ToMove : colorT
    - Castling : byte
    - HalfMoveClock : ushort
    - PlyCounter : ushort
    - Hash : uint
    - PawnHash : uint
  }
}

package "Move values" {
  class "MoveSpec" as DetailMoveSpec <<portable request>> {
    from : squareT
    to : squareT
    promotion : pieceT
    castling : bool
  }

  class "MoveAction" as DetailMoveAction <<resolved action>> {
    from : squareT
    to : squareT
    promote : pieceT
    movingPiece : pieceT
    capturedPiece : pieceT
    capturedSquare : squareT
    castleFlags : byte
    epSquare : squareT
    oldHalfMoveClock : ushort
  }

  class "ScoredMove" as DetailScoredMove {
    score : int32_t
  }

  class "MoveList" as DetailMoveList {
    ListSize : uint
    Moves : ScoredMove[MAX_LEGAL_MOVES]
  }
}

package "Square helpers" {
  class "SquareList" as DetailSquareList {
    ListSize : uint
    Squares : squareT[MAX_SQUARELIST]
  }

  class "SquareSet" as DetailSquareSet {
    bits_ : uint64_t
  }
}

package "Text format mappings" {
  class "Position text API" as DetailTextApi <<methods>> {
    ReadFromFEN
    ReadFromFENorUCI
    PrintFEN
    parseMoveSpec
    makeSan
    applyCoordinateMoves
  }
}

DetailPosition ...> DetailMoveSpec : resolves
DetailPosition ...> DetailMoveAction : applies / undoes
DetailPosition ...> DetailMoveList : fills
DetailPosition ...> DetailSquareList : attack output
DetailPosition ...> DetailSquareSet : move filter

DetailScoredMove --|> DetailMoveAction
DetailMoveList *---> "0..*" DetailScoredMove

DetailTextApi ---> DetailPosition
DetailTextApi ...> DetailMoveSpec
@enduml
