# Notation {#architecture_core_notation}

Notation is the location-level text import/export mapping around
@ref scid::core::Game "scid::core::Game" and @ref scid::core::Position
"scid::core::Position".  PGN handles whole-game import and export; notation
answers narrower questions at a @ref scid::core::MovetextLocation
"MovetextLocation": what is the current UCI position command, what is the
previous or next move in UCI notation, and what SAN should be displayed for the
move beside the cursor?

The notation helpers start with a @ref scid::core::Game "Game" and a
@ref scid::core::MovetextLocation "MovetextLocation".  They restore a
@ref scid::core::GameCursor "GameCursor", replay the moves needed to reach that
location, and use @ref scid::core::Position "Position" as the resolver for
board-sensitive text.  UCI move text comes directly from stored
@ref scid::core::MoveSpec "MoveSpec" values; SAN prefers cached
@ref scid::core::Move::san "Move::san" text and falls back to
@ref scid::core::Position::makeSan "Position::makeSan()" when the move must be
regenerated.

@ref scid::core::notation::currentPositionUci "notation::currentPositionUci()"
emits a complete UCI `position` command for the current location.  Standard-start
games use `startpos`; non-standard starts and positions after null moves are
emitted from FEN.  The position API supplies the lower-level text operations:
reading FEN or UCI-position strings, parsing SAN-like or coordinate moves into
@ref scid::core::MoveSpec "MoveSpec", generating SAN, applying coordinate move
lists, and printing FEN.

@startuml core-notation
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

rectangle "Text forms\n\n- <font:Source Code Pro>SAN</font>\n- <font:Source Code Pro>UCI moves</font>\n- <font:Source Code Pro>UCI position</font>\n- <font:Source Code Pro>FEN</font>\n- <font:Source Code Pro>coordinate moves</font>" as TextForms #EEF2FF

rectangle "Location notation\n\n- <font:Source Code Pro>currentPositionUci</font>\n- <font:Source Code Pro>previousMoveUci</font>\n- <font:Source Code Pro>nextMoveUci</font>\n- <font:Source Code Pro>previousSan</font>\n- <font:Source Code Pro>nextSan</font>\n- <font:Source Code Pro>partialMoveList</font>" as LocationNotation #EEF2FF

rectangle "Game location\n\n- <font:Source Code Pro>Game</font>\n- <font:Source Code Pro>MovetextLocation</font>\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MoveSpec</font>" as GameLocation #FFF7ED

rectangle "Position resolver\n\n- <font:Source Code Pro>Position</font>\n- <font:Source Code Pro>parseMoveSpec</font>\n- <font:Source Code Pro>makeSan</font>\n- <font:Source Code Pro>ReadFromFENorUCI</font>\n- <font:Source Code Pro>PrintFEN</font>" as PositionResolver #EEF2FF

TextForms ----> PositionResolver : parse
PositionResolver ----> TextForms : format

GameLocation ----> LocationNotation : source
LocationNotation ----> PositionResolver : replay / resolve
LocationNotation ----> TextForms : output
@enduml

## Domain Model

This diagram expands notation into the public helpers and the domain objects
they depend on.  It is intentionally loose: notation has no aggregate of its
own.  It is a mapping layer that restores a cursor location, derives a
position, and formats text from the move or board state at that point.

@ref scid::core::notation::currentPositionUci "currentPositionUci()",
@ref scid::core::notation::previousMoveUci "previousMoveUci()" and
@ref scid::core::notation::nextMoveUci "nextMoveUci()" are coordinate or UCI
helpers.  They read stored @ref scid::core::MoveSpec "MoveSpec" values and, for
full position commands, replay from the effective start position.
@ref scid::core::notation::previousSan "previousSan()" and
@ref scid::core::notation::nextSan "nextSan()" are board-sensitive: they must
know the position before the move, and they return an empty string when the move
cannot be replayed legally.

The @ref scid::core::Position "Position" text API is the lower-level half of the
same mapping.  @ref scid::core::Position::ReadFromFEN "ReadFromFEN()" and
@ref scid::core::Position::ReadFromFENorUCI "ReadFromFENorUCI()" build board
state from text; @ref scid::core::Position::parseMoveSpec "parseMoveSpec()" and
@ref scid::core::Position::readCoordinateMoveSpec "readCoordinateMoveSpec()"
turn input notation into a portable move request;
@ref scid::core::Position::makeSan "makeSan()" and
@ref scid::core::Position::PrintFEN "PrintFEN()" turn position state back into
text.

@startuml core-notation-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam nodesep 75
skinparam ranksep 90
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

package "Notation API" {
  class "Position command helpers" as DetailPositionCommandHelpers <<functions>> {
    currentPositionUci()
  }

  class "Move text helpers" as DetailMoveTextHelpers <<functions>> {
    previousMoveUci()

    nextMoveUci()

    previousSan()

    nextSan()

    partialMoveList()
  }
}

package "Game location" {
  class "Game" as DetailGame <<aggregate root>> #FFF7ED
  class "MovetextLocation" as DetailLocation <<bookmark>>
  class "GameCursor" as DetailGameCursor <<read-only>>
  class "Move" as DetailMove {
    spec : MoveSpec
    san : string
  }
  class "MoveSpec" as DetailMoveSpec {
    from : squareT

    to : squareT

    promotion : pieceT

    castling : bool
  }
}

package "Position text API" {
  class "Position" as DetailPosition <<board state>>

  class "Position parsers" as DetailPositionParsers <<methods>> {
    ReadFromFEN()

    ReadFromFENorUCI()

    parseMoveSpec()

    readCoordinateMoveSpec()

    applyCoordinateMoves()
  }

  class "Position formatters" as DetailPositionFormatters <<methods>> {
    makeSan()

    PrintFEN()
  }
}

DetailGameCursor ---> DetailGame
DetailGameCursor ...> DetailLocation : restore
DetailGameCursor ...> DetailMove : previous / next
DetailMove *---> DetailMoveSpec

DetailPositionCommandHelpers ---> DetailGameCursor : restore / replay
DetailMoveTextHelpers ---> DetailGameCursor : inspect / replay
DetailPositionCommandHelpers ...> DetailPosition : replay / FEN
DetailMoveTextHelpers ...> DetailPosition : replay / SAN

DetailPositionParsers ---> DetailPosition : build / advance
DetailPositionParsers ...> DetailMoveSpec : parse
DetailPositionFormatters ---> DetailPosition : format
DetailPositionFormatters ...> DetailMoveSpec : SAN input
@enduml
