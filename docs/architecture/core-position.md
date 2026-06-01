# Position {#architecture_core_position}

`scid::core::Position` is the mutable board-state aggregate.  It owns the board
array, piece and material indexes, side to move, castling rights, en-passant
target, move counters, and hash values.  Move generation, legality checks, move
application, undo, FEN/UCI parsing, and SAN formatting all resolve through that
state.

## Aggregate Root: `Position`

The aggregate view is the compact map of the position boundary.  Text formats
such as FEN, UCI position strings, coordinate moves, SAN, and legacy board
strings sit outside the aggregate.  `Position` turns portable move requests into
position-resolved actions, updates its internal indexes when moves are applied
or undone, and exposes derived facts such as legal replies, check, mate,
material and hashes.

@startuml core-position
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

rectangle "Text boundaries\n\n- <font:Atkinson Hyperlegible Mono>FEN / EPD</font>\n- <font:Atkinson Hyperlegible Mono>UCI position</font>\n- <font:Atkinson Hyperlegible Mono>SAN / coordinate moves</font>\n- <font:Atkinson Hyperlegible Mono>legacy board strings</font>" as TextBoundaries #EEF2FF

rectangle "Move requests and actions\n\n- <font:Atkinson Hyperlegible Mono>MoveSpec</font>\n- <font:Atkinson Hyperlegible Mono>MoveAction</font>\n- <font:Atkinson Hyperlegible Mono>MoveList</font>" as MoveBoundary #EEF2FF

rectangle "Aggregate root\n\n- <font:Atkinson Hyperlegible Mono>Position</font>" as Position #ECFDF5

rectangle "Board state\n\n- <font:Atkinson Hyperlegible Mono>Board</font>\n- <font:Atkinson Hyperlegible Mono>List</font>\n- <font:Atkinson Hyperlegible Mono>Material</font>\n- <font:Atkinson Hyperlegible Mono>Pinned</font>" as BoardState #FFF7ED

rectangle "Rule state\n\n- <font:Atkinson Hyperlegible Mono>ToMove</font>\n- <font:Atkinson Hyperlegible Mono>Castling</font>\n- <font:Atkinson Hyperlegible Mono>EPTarget</font>\n- <font:Atkinson Hyperlegible Mono>HalfMoveClock</font>\n- <font:Atkinson Hyperlegible Mono>PlyCounter</font>" as RuleState #FFF7ED

rectangle "Derived indexes\n\n- <font:Atkinson Hyperlegible Mono>Hash</font>\n- <font:Atkinson Hyperlegible Mono>PawnHash</font>\n- <font:Atkinson Hyperlegible Mono>rank/file/diagonal counts</font>\n- <font:Atkinson Hyperlegible Mono>SquareList</font>\n- <font:Atkinson Hyperlegible Mono>SquareSet</font>" as DerivedState #FFF7ED

TextBoundaries ----> Position : parse / format
MoveBoundary ----> Position : resolve / apply / undo

Position ----> BoardState : owns
Position ----> RuleState : owns
Position ----> DerivedState : owns and maintains
@enduml

## Domain Model

This diagram expands the position map into the main public types that sit around
board state and legal move handling.  It is intentionally loose: it shows the
types programmers meet when using `Position`, while omitting private helper
functions, attack tables and most low-level board geometry constants.

`MoveSpec` is a portable request: it names origin, destination, promotion, and
castling intent, but it cannot undo a move.  `Position` resolves that request
against the current board into a `MoveAction`, which carries captured pieces,
castling rights, en-passant state and halfmove data needed for exact undo.
`MoveList` is the fixed-capacity container filled by move generation.

Square collections are supporting structures rather than independent
aggregates.  `SquareList` is useful when callers need the actual attacking or
checking squares; `SquareSet` is a compact membership filter used while
generating moves.  Primitive scalar aliases such as `pieceT`, `colorT` and
`squareT` appear as field types rather than separate model nodes.

@startuml core-position-domain-model
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
  class "Position" as DetailPosition <<aggregate root>> {
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

package "Move values" #FFF7ED {
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

package "Square helpers" #FFF7ED {
  class "SquareList" as DetailSquareList {
    ListSize : uint
    Squares : squareT[MAX_SQUARELIST]
  }

  class "SquareSet" as DetailSquareSet {
    bits_ : uint64_t
  }
}

package "Text boundaries" #EEF2FF {
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
