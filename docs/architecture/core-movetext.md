# Movetext {#architecture_core_movetext}

`scid::core::Movetext` is the root of a game's playable move tree.  It owns the
comment before the first mainline move and the mainline itself.  From there, the
model is recursive: a `MoveSequence` owns ordered `Move` values, each `Move` can
own child `Variation` values, and each `Variation` owns another `MoveSequence`.

The aggregate view is the compact map of the movetext tree.  PGN, SAN and NAG
text sit outside the tree as notation mappings; cursors use the tree without
owning it; `Movetext` owns the mainline; moves own both their payload and the
variation branches that depart from them.

@startuml core-movetext
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

rectangle "Text format mappings\n\n- <font:Source Code Pro>PGN movetext</font>\n- <font:Source Code Pro>SAN</font>\n- <font:Source Code Pro>NAG</font>\n- <font:Source Code Pro>comments</font>" as TextMappings #EEF2FF

rectangle "Traversal and editing\n\n- <font:Source Code Pro>GameCursor</font>\n- <font:Source Code Pro>MovetextCursor</font>\n- <font:Source Code Pro>MovetextLocation</font>" as Traversal #EEF2FF

rectangle "Tree root\n\n- <font:Source Code Pro>Movetext</font>" as MovetextRoot #ECFDF5

rectangle "Line\n\n- <font:Source Code Pro>MoveSequence</font>" as Line #FFF7ED

rectangle "Move payload\n\n- <font:Source Code Pro>Move</font>\n- <font:Source Code Pro>MoveSpec</font>\n- <font:Source Code Pro>MoveMetadata</font>\n- <font:Source Code Pro>Nag</font>" as MovePayload #FFF7ED

rectangle "Recursive branch\n\n- <font:Source Code Pro>Variation</font>\n- <font:Source Code Pro>MoveSequence</font>" as Branch #FFF7ED

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

`MoveSpec` is the durable move intent stored in the tree.  It is not enough to
execute or undo a move by itself; `Position` resolves it when callers need board
state, legality checks or SAN generation.  `MoveMetadata` and `Nag` are attached
to a move as annotation data, while `Variation::initialComment` is the comment
that appears before the first move of a variation.  Cursors and
`MovetextLocation` are intentionally left to the traversal model; here they
matter only as users of the tree.

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
skinparam ClassBackgroundColor #F9FAFB
skinparam classAttributeIconSize 0

hide circle
hide empty methods

left to right direction

package "Tree root" #ECFDF5 {
  class "Movetext" as DetailMovetext <<tree root>> {
    initialComment : string
    mainline : MoveSequence
  }
}

package "Line and branch" #FFF7ED {
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

package "Move payload" #FFF7ED {
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
