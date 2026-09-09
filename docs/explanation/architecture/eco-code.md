# Code {#architecture_eco_code}

Code is the compact ECO value model.  The public text form is familiar opening
classification text such as `A00`, `B20`, or Scid's extended `C50a1`; the stored
form is a small integer suitable for sorting, persistence, range checks and
statistics.

`fromString()` is the parse boundary.  It accepts one-, two-, and
three-character prefixes as the first code in that range, and it accepts
canonical extended forms with a lower-case subcode and optional `1..4`
extension.  `toBasicString()` and `toExtendedString()` move in the other
direction.

@startuml eco-code
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 70
skinparam Ranksep 52
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

<style>
rectangle {
  Padding 24
  MinimumWidth 145
  MinimumHeight 44
}
</style>

left to right direction

rectangle "ECO text" as Text #EEF2FF

rectangle "Packed code" as Packed #FFF7ED

rectangle "Display text" as Display #EEF2FF

rectangle "Range endpoint" as Range #EEF2FF

rectangle "Statistics bucket" as Bucket #EEF2FF

Text -[hidden]right- Packed
Packed -[hidden]right- Display
Packed -[hidden]down- Range
Range -[hidden]right- Bucket

Text --> Packed : fromString()
Packed --> Display : toString()
Packed --> Range : lastSubCode()
Packed --> Bucket : reduce()
@enduml

## Domain Model

The code model is deliberately not a class hierarchy.  `Code` is the packed
value, `String` is the fixed output buffer, and the functions describe the
legal transformations around that value.  The important distinction is between
display/range operations, which preserve the ECO ordering, and `reduce()`,
which folds Scid numeric extensions into the same bucket as their parent
subcode for statistics.

`basicCode()` removes Scid's extended suffix.  `lastSubCode()` turns a prefix
or partial extended code into the final concrete code covered by that range.
`ECO_None` is the sentinel for "no known classification"; it can be parsed,
formatted and range-expanded, but it is not a valid input to `reduce()`.

@startuml eco-code-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 82
skinparam Ranksep 74
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 118
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

left to right direction

class "Code" as DetailCode <<packed value>> #FFF7ED {
  uint16_t
  ECO_None
}

class "String" as DetailString <<output buffer>> #EEF2FF {
  char[6]
}

class "fromString()" as DetailParse <<parse>> #EEF2FF

class "toString()" as DetailFormat <<format>> #EEF2FF

class "basicCode()" as DetailBasic <<range>> #EEF2FF

class "lastSubCode()" as DetailLast <<range>> #EEF2FF

class "reduce()" as DetailReduce <<statistics>> #EEF2FF

DetailParse -[hidden]right- DetailCode
DetailCode -[hidden]right- DetailFormat
DetailBasic -[hidden]right- DetailLast
DetailLast -[hidden]right- DetailReduce
DetailCode -[hidden]down- DetailLast

DetailParse --> DetailCode : creates
DetailCode --> DetailFormat : source
DetailFormat --> DetailString : writes
DetailCode --> DetailBasic : strip suffix
DetailCode --> DetailLast : expand range
DetailCode --> DetailReduce : bucket
@enduml
