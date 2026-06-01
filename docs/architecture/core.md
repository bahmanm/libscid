# Core {#architecture_core}

Core is the chess model of libscid.  It owns the in-memory representation of a
game, the board state needed to validate and replay moves, and the value types
that keep chess data portable across storage, PGN, notation and user-facing
tools.

@startuml core-overview
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 48
skinparam Ranksep 42
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #F9FAFB

<style>
rectangle {
  Padding 12
}
</style>

left to right direction

rectangle "Text format mappings" as TextMappings #EEF2FF

rectangle "Game aggregate" as GameAggregate #ECFDF5

rectangle "Movetext tree" as MovetextTree #FFF7ED

rectangle "Traversal" as Traversal #EEF2FF

rectangle "Position aggregate" as PositionAggregate #ECFDF5

TextMappings ----> GameAggregate : maps
GameAggregate ----> MovetextTree : owns
MovetextTree ----> Traversal : walks
Traversal ----> PositionAggregate : replays
@enduml

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage architecture_core_game "Game"
- @subpage architecture_core_game_metadata "Game Metadata"
- @subpage architecture_core_position "Position"
- @subpage architecture_core_move_generation "Move Generation"
- @subpage architecture_core_movetext "Movetext"
- @subpage architecture_core_traversal "Traversal"
- @subpage architecture_core_pgn "PGN"
- @subpage architecture_core_notation "Notation"

\htmlonly
</div>
\endhtmlonly

---

There are two central aggregates:
- @ref architecture_core_game "Game" owns a complete game record: header data, optional non-standard start position, and the recursive movetext tree.
- @ref architecture_core_position "Position" owns a board state at one moment in the game: pieces, side to move, castling rights, en-passant state, clocks, hashes, material and derived indexes.

Most higher-level work in Core is either about changing one of those aggregates, or about translating between them.

---

@ref architecture_core_movetext "Movetext" is the durable move tree inside a game.  It stores move intent,
comments, NAGs and variation structure, but it does not by itself prove legality
or reconstruct board state.  @ref architecture_core_move_generation "Move generation"
belongs to `Position`, where stored move intent can be resolved against a real
board state.

---

@ref architecture_core_traversal "Traversal" supplies the cursor model for walking and
editing that tree.  When a caller needs a board, the cursor replays the stored
move intent through `Position`.

---

Text formats sit around the model as mappings, not as owning state.  @ref architecture_core_pgn "PGN"
maps a complete game to and from text.  @ref architecture_core_notation "Notation"
maps position-resolved moves to and from strings such as SAN or coordinate
notation.  @ref architecture_core_game_metadata "Header metadata" is structured
inside `GameHeader`, with supplemental PGN tags preserved beside the typed
fields.
