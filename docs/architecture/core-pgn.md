# PGN {#architecture_core_pgn}

PGN is the text import/export mapping around `scid::core::Game`.  Decoding reads tags,
comments, NAGs, SAN moves and Recursive Annotation Variations into the editable
game model.  Encoding walks the same model in the opposite direction, emitting
tag pairs, movetext, comments, annotations, variations and the final result.

The PGN import/export mapping has two public directions.  `pgn::parseGame()` consumes input
text and mutates a `Game`; callers may parse into a fresh game or append
movetext at a `MovetextLocation`.  `pgn::encode()` serialises a `Game` into an
appendable destination and then applies line wrapping.

Parsing is stateful because SAN moves and RAVs are relative to a current
position in the game tree.  The parser resolves SAN against `Position`, edits
the movetext tree through cursor semantics, records diagnostics in `ParseLog`,
and updates the current `MovetextLocation` when the location overload is used.
Encoding is also position-aware: when a move has no cached SAN text, the encoder
replays from the current `Position` and generates SAN before writing the move.

@startuml core-pgn
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

rectangle "PGN text\n\n- <font:Source Code Pro>tag pairs</font>\n- <font:Source Code Pro>movetext</font>\n- <font:Source Code Pro>comments</font>\n- <font:Source Code Pro>NAGs</font>\n- <font:Source Code Pro>RAVs</font>" as PgnText #EEF2FF

rectangle "Decode mapping\n\n- <font:Source Code Pro>parseGame</font>\n- <font:Source Code Pro>ParseLog</font>\n- <font:Source Code Pro>MovetextLocation</font>" as Decode #EEF2FF

rectangle "Game model\n\n- <font:Source Code Pro>Game</font>\n- <font:Source Code Pro>GameHeader</font>\n- <font:Source Code Pro>Movetext</font>\n- <font:Source Code Pro>Position</font>" as GameModel #ECFDF5

rectangle "Traversal and editing\n\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MovetextCursor</font>\n- <font:Source Code Pro>pgn::nextLocation</font>" as Traversal #FFF7ED

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
are implementation details, while `parseGame()`, `ParseLog`, `encode()`,
`EncodeOptions`, and the PGN traversal helpers are the programmer-facing
surface.

`ParseLog` is cumulative.  It records bytes, lines, game count and formatted
diagnostics across parse calls, so batch importers can reuse one log while
streaming multiple games.  `EncodeOptions` is the export policy: it controls
symbolic NAGs, supplemental tags, comments, variations and line width.

`parseGame()` writes into `Game` rather than returning a detached parse tree.
Tags become header fields or supplemental tags; SAN tokens become `MoveSpec`
values inserted into `Movetext`; comments and NAGs attach to the current move
or variation.  `encode()` reads the same structures, uses `Position` when SAN
must be generated, and uses `break_lines()` to turn internal token separators
into final PGN whitespace.

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
skinparam ClassBackgroundColor #F9FAFB
skinparam classAttributeIconSize 0

hide circle
hide empty methods

left to right direction

package "PGN API" #EEF2FF {
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

package "Game model" #ECFDF5 {
  class "Game" as DetailGame <<aggregate root>>
  class "GameHeader" as DetailGameHeader
  class "Movetext" as DetailMovetext <<tree root>>
  class "Position" as DetailPosition <<board state>>
}

package "Movetext data" #FFF7ED {
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
