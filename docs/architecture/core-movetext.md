# Movetext {#architecture_core_movetext}

@ref scid::core::Movetext "scid::core::Movetext" is the root of a game's
playable move tree.  It owns the comment before the first mainline move and the
mainline itself.  From there, the model is recursive: a
@ref scid::core::MoveSequence "MoveSequence" owns ordered
@ref scid::core::Move "Move" values, each @ref scid::core::Move "Move" can own
child @ref scid::core::Variation "Variation" values, and each
@ref scid::core::Variation "Variation" owns another
@ref scid::core::MoveSequence "MoveSequence".

The aggregate view is the compact map of the movetext tree.  PGN, SAN and NAG
text sit outside the tree as notation mappings; cursors use the tree without
owning it; @ref scid::core::Movetext "Movetext" owns the mainline; moves own
both their payload and the variation branches that depart from them.

@startuml core-movetext
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

rectangle "Text format mappings\n\n- <font:Source Code Pro>PGN movetext</font>\n- <font:Source Code Pro>SAN</font>\n- <font:Source Code Pro>NAG</font>\n- <font:Source Code Pro>comments</font>" as TextMappings #EEF2FF

rectangle "Traversal and editing\n\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MovetextCursor</font>\n- <font:Source Code Pro>MovetextLocation</font>" as Traversal #EEF2FF

rectangle "Tree root\n\n- <font:Source Code Pro>Movetext</font>" as MovetextRoot #FFF7ED

rectangle "Line\n\n- <font:Source Code Pro>MoveSequence</font>" as Line #EEF2FF

rectangle "Move payload\n\n- <font:Source Code Pro>Move</font>\n- <font:Source Code Pro>MoveSpec</font>\n- <font:Source Code Pro>MoveMetadata</font>\n- <font:Source Code Pro>Nag</font>" as MovePayload #EEF2FF

rectangle "Recursive branch\n\n- <font:Source Code Pro>Variation</font>\n- <font:Source Code Pro>MoveSequence</font>" as Branch #EEF2FF

TextMappings ----> MovetextRoot : parse / format
Traversal ----> MovetextRoot : navigate / edit

MovetextRoot ----> Line : owns mainline
Line ----> MovePayload : owns moves
MovePayload ----> Branch : owns child variations
Branch ----> Line : owns recursive line
@enduml

## Domain Model

This diagram expands the movetext tree into the public types that carry moves,
comments, annotations and variation branches.  It is intentionally loose: it
shows recursive ownership and the split between stored move intent and display
metadata, while leaving detailed cursor mechanics to the traversal
model.

@ref scid::core::MoveSpec "MoveSpec" is the durable move intent stored in the
tree.  It is not enough to execute or undo a move by itself;
@ref scid::core::Position "Position" resolves it when callers need board state,
legality checks or SAN generation.  @ref scid::core::MoveMetadata
"MoveMetadata" and @ref scid::core::Nag "Nag" are attached to a move as
annotation data, while @ref scid::core::Variation::initialComment
"Variation::initialComment" is the comment that appears before the first move of
a variation.  Cursors and @ref scid::core::MovetextLocation "MovetextLocation"
are intentionally left to the traversal model; here they matter only as users of
the tree.

@startuml core-movetext-domain-model
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

package "Tree root" {
  class "Movetext" as DetailMovetext <<tree root>> #FFF7ED {
    initialComment : string
    mainline : MoveSequence
  }
}

package "Line and branch" {
  class "MoveSequence" as DetailMoveSequence {
    moves : vector<Move>
  }

  class "Move" as DetailMove {
    spec : MoveSpec
    san : string
    metadata : MoveMetadata
    childVariations : vector<Variation>
  }

  class "Variation" as DetailVariation {
    initialComment : string
    line : MoveSequence
  }
}

package "Move payload" {
  class "MoveSpec" as DetailMoveSpec {
    from : squareT
    to : squareT
    promotion : pieceT
    castling : bool
  }

  class "MoveMetadata" as DetailMoveMetadata {
    nags : vector<Nag>
    comment : string
  }

  class "Nag" as DetailNag <<annotation>>
}

DetailMovetext *---> DetailMoveSequence : <font:Source Code Pro>mainline</font>
DetailMoveSequence *---> "0..*" DetailMove
DetailMove *---> DetailMoveSpec
DetailMove *---> DetailMoveMetadata
DetailMoveMetadata *---> "0..*" DetailNag
DetailMove *---> "0..*" DetailVariation
DetailVariation *---> DetailMoveSequence : recursive line
@enduml
