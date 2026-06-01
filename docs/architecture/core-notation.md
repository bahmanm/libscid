# Notation {#architecture_core_notation}

Notation is the location-level text boundary around `scid::core::Game` and
`scid::core::Position`.  PGN handles whole-game import and export; notation
answers narrower questions at a `MovetextLocation`: what is the current UCI
position command, what is the previous or next move in UCI notation, and what
SAN should be displayed for the move beside the cursor?

## Text Boundary: Notation

The notation helpers start with a `Game` and a `MovetextLocation`.  They restore
a `GameCursor`, replay the moves needed to reach that location, and use
`Position` as the resolver for board-sensitive text.  UCI move text comes
directly from stored `MoveSpec` values; SAN prefers cached `Move::san` text and
falls back to `Position::makeSan()` when the move must be regenerated.

`notation::currentPositionUci()` emits a complete UCI `position` command for
the current location.  Standard-start games use `startpos`; non-standard starts
and positions after null moves are emitted from FEN.  The position API supplies
the lower-level text operations: reading FEN or UCI-position strings, parsing
SAN-like or coordinate moves into `MoveSpec`, generating SAN, applying
coordinate move lists, and printing FEN.

@startuml core-notation
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

rectangle "Text forms\n\n- <font:Source Code Pro>SAN</font>\n- <font:Source Code Pro>UCI moves</font>\n- <font:Source Code Pro>UCI position</font>\n- <font:Source Code Pro>FEN</font>\n- <font:Source Code Pro>coordinate moves</font>" as TextForms #EEF2FF

rectangle "Location notation\n\n- <font:Source Code Pro>currentPositionUci</font>\n- <font:Source Code Pro>previousMoveUci</font>\n- <font:Source Code Pro>nextMoveUci</font>\n- <font:Source Code Pro>previousSan</font>\n- <font:Source Code Pro>nextSan</font>\n- <font:Source Code Pro>partialMoveList</font>" as LocationNotation #EEF2FF

rectangle "Game location\n\n- <font:Source Code Pro>Game</font>\n- <font:Source Code Pro>MovetextLocation</font>\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MoveSpec</font>" as GameLocation #ECFDF5

rectangle "Position resolver\n\n- <font:Source Code Pro>Position</font>\n- <font:Source Code Pro>parseMoveSpec</font>\n- <font:Source Code Pro>makeSan</font>\n- <font:Source Code Pro>ReadFromFENorUCI</font>\n- <font:Source Code Pro>PrintFEN</font>" as PositionResolver #FFF7ED

TextForms ----> PositionResolver : parse
PositionResolver ----> TextForms : format

GameLocation ----> LocationNotation : source
LocationNotation ----> PositionResolver : replay / resolve
LocationNotation ----> TextForms : output
@enduml

## Domain Model

This diagram expands notation into the public helpers and the domain objects
they depend on.  It is intentionally loose: notation has no aggregate of its
own.  It is a boundary layer that restores a cursor location, derives a
position, and formats text from the move or board state at that point.

`currentPositionUci()`, `previousMoveUci()` and `nextMoveUci()` are coordinate
or UCI helpers.  They read stored `MoveSpec` values and, for full position
commands, replay from the effective start position.  `previousSan()` and
`nextSan()` are board-sensitive: they must know the position before the move,
and they return an empty string when the move cannot be replayed legally.

The `Position` text API is the lower-level half of the same boundary.
`ReadFromFEN()` and `ReadFromFENorUCI()` build board state from text;
`parseMoveSpec()` and `readCoordinateMoveSpec()` turn input notation into a
portable move request; `makeSan()` and `PrintFEN()` turn position state back
into text.

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
skinparam ClassBackgroundColor #F9FAFB
skinparam classAttributeIconSize 0

hide circle
hide empty methods

left to right direction

package "Notation API" #EEF2FF {
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

package "Game location" #ECFDF5 {
  class "Game" as DetailGame <<aggregate root>>
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

package "Position text API" #FFF7ED {
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
