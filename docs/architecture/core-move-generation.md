# Move Generation {#architecture_core_move_generation}

Move generation is a behaviour of @ref scid::core::Position
"scid::core::Position", not a separate aggregate.
@ref scid::core::Position "Position" owns the board, side to move, castling
rights, en-passant target, piece lists, pin directions, and attack indexes
needed to decide which moves are legal.  The public result is a
@ref scid::core::MoveList "MoveList" of resolved
@ref scid::core::MoveAction "MoveAction" values that can be applied, undone,
scored, ordered, or formatted.

The move-generation pipeline starts from the current side to move.
@ref scid::core::Position "Position" first recomputes pins and, unless the
caller has already ruled check out, counts attacks on the side-to-move king.  A
checked king takes the evasion path:
double check permits only king moves, while single check also allows captures
of the checking piece and blocks on the checking ray.

When the side to move is not in check, generation walks the piece list.  Pinned
pieces are constrained to the pin direction, pinned knights do not move, pawns
are checked for promotion and en-passant edge cases, and king moves are tested
against enemy attacks before being appended.  Castling is generated only when
non-captures are requested, the king is not currently in check, the castling
right is present, the king and rook path is clear, and the king's transit
squares are not attacked.

@startuml core-move-generation
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

rectangle "Move request\n\n- <font:Source Code Pro>GEN_CAPTURES</font>\n- <font:Source Code Pro>GEN_NON_CAPS</font>\n- <font:Source Code Pro>piece mask</font>\n- <font:Source Code Pro>maybeInCheck</font>" as Request #EEF2FF

rectangle "Position state\n\n- <font:Source Code Pro>Board</font>\n- <font:Source Code Pro>List</font>\n- <font:Source Code Pro>ToMove</font>\n- <font:Source Code Pro>Castling</font>\n- <font:Source Code Pro>EPTarget</font>" as PositionState #FFF7ED

rectangle "Legality context\n\n- <font:Source Code Pro>CalcPins</font>\n- <font:Source Code Pro>CalcNumChecks</font>\n- <font:Source Code Pro>SquareList</font>\n- <font:Source Code Pro>SquareSet</font>" as Legality #EEF2FF

rectangle "Piece generators\n\n- <font:Source Code Pro>GenPawnMoves</font>\n- <font:Source Code Pro>GenPieceMoves</font>\n- <font:Source Code Pro>GenKingMoves</font>\n- <font:Source Code Pro>GenCastling</font>\n- <font:Source Code Pro>GenCheckEvasions</font>" as Generators #EEF2FF

rectangle "Resolved moves\n\n- <font:Source Code Pro>MoveAction</font>\n- <font:Source Code Pro>ScoredMove</font>\n- <font:Source Code Pro>MoveList</font>" as Results #EEF2FF

Request ----> PositionState : asks current side
PositionState ----> Legality : pins / checks
Legality ----> Generators : constrains
PositionState ----> Generators : board and rule state
Generators ----> Results : resolve and append
Results ----> PositionState : apply / undo elsewhere
@enduml

## Domain Model

This diagram expands the generation pipeline into the public types and the
important helper families behind them.  It is intentionally loose: generation
is an algorithm over @ref scid::core::Position "Position" state, so the private
helper functions are shown as named method groups rather than as independent
classes.

@ref scid::core::MoveSpec "MoveSpec" and @ref scid::core::MoveAction
"MoveAction" sit on opposite sides of the position resolution step.
@ref scid::core::MoveSpec "MoveSpec" is the portable request stored in games and
accepted by notation parsers.  @ref scid::core::MoveAction "MoveAction" is the
reversible, position-resolved form produced by generation and
@ref scid::core::Position::resolveMove "Position::resolveMove()".  It records
captured pieces, captured squares, old castling rights, old en-passant target,
old halfmove clock, and piece-list indexes so @ref scid::core::Position::apply
"Position::apply()" and @ref scid::core::Position::undo "Position::undo()" can
restore state exactly.

The low-level geometry is deliberately split by dependency.  `knightAttacks`
and `kingAttacks` are table lookups for one-piece attacks.  `square_Move()` and
`square_Last()` describe one-step movement and slider ray ends.  The
`move_predicates` helpers validate piece geometry against an occupancy
predicate, which lets legality code reason about hypothetical board states
without first mutating every @ref scid::core::Position "Position" index.

@startuml core-move-generation-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam nodesep 65
skinparam ranksep 80
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

package "Position state" {
  class "Position" as DetailPosition <<aggregate root>> #FFF7ED {
    GenerateMoves()
    GenerateCaptures()
    GenPieceMoves()
    IsLegalMove()
    resolveMove()
    apply()
    undo()
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
    Moves : ScoredMove[MAX_LEGAL_MOVES]
  }
}

package "Generation context" {
  class "Move generation" as DetailGeneration <<methods>> {
    GenerateMoves()
    GenerateCaptures()
    CalcPins()
    CalcNumChecks()
    GenCheckEvasions()
    GenPawnMoves()
    GenPieceMoves()
    GenKingMoves()
    GenCastling()
  }

  class "SquareList" as DetailSquareList <<attack list>>
  class "SquareSet" as DetailSquareSet <<target filter>>
}

package "Geometry helpers" {
  class "Attack tables" as DetailAttackTables <<constants>> {
    knightAttacks
    kingAttacks
  }

  class "Square rays" as DetailSquareRays <<functions>> {
    square_Move()
    square_Last()
  }

  class "move_predicates" as DetailPredicates <<namespace>> {
    pseudo()
    attack()
    attack_slider()
    opens_ray()
  }
}

DetailPosition ...> DetailGeneration : owns algorithm

DetailPosition ...> DetailMoveSpec : resolves
DetailPosition ...> DetailMoveAction : creates / applies
DetailGeneration ...> DetailMoveList : fills
DetailMoveList *---> "0..*" DetailScoredMove
DetailScoredMove --|> DetailMoveAction

DetailGeneration ...> DetailSquareList : checking pieces
DetailGeneration ...> DetailSquareSet : evasion targets
DetailGeneration ...> DetailAttackTables : king / knight
DetailGeneration ...> DetailSquareRays : sliders / pawns
DetailGeneration ...> DetailPredicates : legality tests
DetailPredicates ...> DetailSquareRays : ray occupancy
@enduml
