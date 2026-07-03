# PGN {#architecture_core_pgn}

PGN is the text import/export mapping around @ref scid::core::Game
"scid::core::Game".  Decoding reads tags, comments, NAGs, SAN moves and
Recursive Annotation Variations into the editable game model.  Encoding walks
the same model in the opposite direction, emitting tag pairs, movetext,
comments, annotations, variations and the final result.

The PGN import/export mapping has two public directions.
@ref scid::core::pgn::parseGame "pgn::parseGame()" consumes input text and
mutates a @ref scid::core::Game "Game"; callers may parse into a fresh game or
append movetext at a @ref scid::core::MovetextLocation "MovetextLocation".
@ref scid::core::pgn::encode "pgn::encode()" serialises a
@ref scid::core::Game "Game" into an appendable destination and then applies
line wrapping.

Parsing is stateful because SAN moves and RAVs are relative to a current
position in the game tree.  The parser resolves SAN against
@ref scid::core::Position "Position", edits the movetext tree through cursor
semantics, records diagnostics in @ref scid::core::pgn::ParseLog "ParseLog",
and updates the current @ref scid::core::MovetextLocation "MovetextLocation"
when the location overload is used.
Encoding is also position-aware: when a move has no cached SAN text, the encoder
replays from the current @ref scid::core::Position "Position" and generates SAN
before writing the move.

The installed C ABI exposes this machinery through game-level primitives rather
than the internal PGN parser locations: `scid_game_create_blank()` creates a
game without moves, `scid_game_create()` parses PGN into a game,
`scid_game_cursor_*()` functions navigate and edit the game tree,
`scid_game_merge_moves()` applies source-game moves at a cursor, and
`scid_game_to_pgn()` exports using optional `scid_game_pgn_options`. Internal
types such as @ref scid::core::MovetextLocation "MovetextLocation" and helpers
such as PGN-order traversal remain implementation details for the core library.

@startuml core-pgn
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

rectangle "PGN text\n\n- <font:Source Code Pro>tag pairs</font>\n- <font:Source Code Pro>movetext</font>\n- <font:Source Code Pro>comments</font>\n- <font:Source Code Pro>NAGs</font>\n- <font:Source Code Pro>RAVs</font>" as PgnText #EEF2FF

rectangle "Decode mapping\n\n- <font:Source Code Pro>parseGame</font>\n- <font:Source Code Pro>ParseLog</font>\n- <font:Source Code Pro>MovetextLocation</font>" as Decode #EEF2FF

rectangle "Game model\n\n- <font:Source Code Pro>Game</font>\n- <font:Source Code Pro>GameHeader</font>\n- <font:Source Code Pro>Movetext</font>\n- <font:Source Code Pro>Position</font>" as GameModel #FFF7ED

rectangle "Traversal and editing\n\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MovetextCursor</font>\n- <font:Source Code Pro>pgn::nextLocation</font>" as Traversal #EEF2FF

rectangle "Encode mapping\n\n- <font:Source Code Pro>encode</font>\n- <font:Source Code Pro>EncodeOptions</font>\n- <font:Source Code Pro>break_lines</font>" as Encode #EEF2FF

PgnText ----> Decode : input
Decode ----> GameModel : fills / appends
Decode ----> Traversal : resolves location

GameModel ----> Encode : source
Traversal ----> Encode : PGN order
Encode ----> PgnText : output
@enduml

## Domain Model

This diagram expands the PGN import/export mapping into the public API types and the domain
objects they read or write.  It is intentionally loose: the lexer and visitor
are implementation details, while @ref scid::core::pgn::parseGame
"parseGame()", @ref scid::core::pgn::ParseLog "ParseLog",
@ref scid::core::pgn::encode "encode()", @ref scid::core::pgn::EncodeOptions
"EncodeOptions", and the PGN traversal helpers are the programmer-facing
surface.

@ref scid::core::pgn::ParseLog "ParseLog" is cumulative.  It records bytes,
lines, game count and formatted diagnostics across parse calls, so batch
importers can reuse one log while streaming multiple games.
@ref scid::core::pgn::EncodeOptions "EncodeOptions" is the export policy: it
controls symbolic NAGs, supplemental tags, comments, variations and line width.

@ref scid::core::pgn::parseGame "parseGame()" writes into
@ref scid::core::Game "Game" rather than returning a detached parse tree.  Tags
become header fields or supplemental tags; SAN tokens become
@ref scid::core::MoveSpec "MoveSpec" values inserted into
@ref scid::core::Movetext "Movetext"; comments and NAGs attach to the current
move or variation.  @ref scid::core::pgn::encode "encode()" reads the same
structures, uses @ref scid::core::Position "Position" when SAN must be
generated, and uses @ref scid::core::pgn::break_lines "break_lines()" to turn
internal token separators into final PGN whitespace.

@startuml core-pgn-domain-model
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

package "PGN API" {
  class "parseGame" as DetailParseGame <<function>> {
    input : char*
    game : Game&
    location : MovetextLocation*
    log : ParseLog&
  }

  class "ParseLog" as DetailParseLog {
    log : string
    n_bytes : unsigned long long
    n_lines : unsigned long long
    n_games : unsigned long long
  }

  class "encode" as DetailEncode <<function>> {
    game : Game const&
    dest : appendable
    options : EncodeOptions
  }

  class "EncodeOptions" as DetailEncodeOptions {
    symbolicNags : bool
    includeSupplementalTags : bool
    includeComments : bool
    includeVariations : bool
    lineWidth : optional<unsigned>
  }

  class "break_lines" as DetailBreakLines <<helper>>

  class "MovetextLocation" as DetailLocation <<bookmark>>
}

package "Game model" {
  class "Game" as DetailGame <<aggregate root>> #FFF7ED
  class "GameHeader" as DetailGameHeader
  class "Movetext" as DetailMovetext <<tree root>>
  class "Position" as DetailPosition <<board state>>
}

package "Movetext data" {
  class "Move" as DetailMove {
    spec : MoveSpec
    san : string
    metadata : MoveMetadata
    childVariations : vector<Variation>
  }

  class "MoveSpec" as DetailMoveSpec
  class "MoveMetadata" as DetailMoveMetadata {
    nags : vector<Nag>
    comment : string
  }

  class "Variation" as DetailVariation {
    initialComment : string
  }

  class "Nag" as DetailNag <<annotation>>
}

DetailParseGame ---> DetailGame : fill / append
DetailParseGame ---> DetailParseLog : diagnostics
DetailParseGame ...> DetailLocation : start / final

DetailEncode ---> DetailGame : read
DetailEncode ---> DetailEncodeOptions : policy
DetailEncode ...> DetailBreakLines : final whitespace

DetailGame *---> DetailGameHeader
DetailGame *---> DetailMovetext
DetailGame o---> DetailPosition : <font:Source Code Pro>start</font>

DetailMovetext *---> "0..*" DetailMove
DetailMove *---> DetailMoveSpec
DetailMove *---> DetailMoveMetadata
DetailMoveMetadata *---> "0..*" DetailNag
DetailMove *---> "0..*" DetailVariation

@enduml
