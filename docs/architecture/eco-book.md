# Book {#architecture_eco_book}

Book is the exact-position ECO classifier.  @ref scid::eco::Book::load
"Book::load()" reads a Scid ECO text file, parses each line from the standard
starting position, and stores the final compact board together with the ECO
code, opening name and move text.

Lookup is intentionally exact.  @ref scid::eco::Book::findEcoString
"findEcoString()" and @ref scid::eco::Book::findEco "findEco()" use the
candidate position's hash to find a small bucket and then compare the compact
board string.  A transposition or nearby opening prefix is not enough; the
board must match one of the positions loaded from the ECO file.

@startuml eco-book
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 72
skinparam Ranksep 58
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

<style>
rectangle {
  Padding 24
  MinimumWidth 150
  MinimumHeight 44
}
</style>

left to right direction

rectangle "Scid ECO file" as File #EEF2FF

rectangle "Code mapping" as Code #EEF2FF

rectangle "Position replay" as Replay #EEF2FF

rectangle "ECO book" as Book #FFF7ED

rectangle "Exact lookup" as Lookup #EEF2FF

rectangle "Prefix listing" as Listing #EEF2FF

File -[hidden]right- Code
Code -[hidden]right- Book
Replay -[hidden]right- Book
Book -[hidden]right- Lookup
Lookup -[hidden]down- Listing

File --> Code : code text
File --> Replay : move text
Replay --> Book : final board
Code --> Book : canonical code
Book --> Lookup : code / name
Book --> Listing : code / name / moves
@enduml

## Domain Model

The book stores two views of the same ECO source.  For classification, it keeps
an index from position hash to compact board plus comment text.  For browsing,
it keeps the loaded comments in file order so
@ref scid::eco::Book::linesWithPrefix "linesWithPrefix()" can return
@ref scid::eco::Book::Line "Book::Line" rows without reconstructing them from
positions.

Returned strings are borrowed from the @ref scid::eco::Book "Book".  A
`Book::Line` is a display row with `code`, `name` and `moves` views; the
`findEcoString()` result is a single view in the form `C50a [Italian Game]`;
`findEco()` returns the packed `Code` or `ECO_None`.

@startuml eco-book-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 88
skinparam Ranksep 82
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 125
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

left to right direction

class "Scid ECO file" as DetailFile <<text source>> #EEF2FF {
  code
  quoted name
  move text
}

class "Book" as DetailBook <<classifier>> #FFF7ED {
  load()
  findEcoString()
  findEco()
  linesWithPrefix()
  lineCount()
  fewestPieces()
  size()
}

class "Position" as DetailPosition <<board state>> #EEF2FF {
  parseMoveSpec()
  applyMove()
  HashValue()
  PrintCompactStr()
}

class "Indexed position" as DetailIndexed <<stored key>> #EEF2FF {
  hash
  compact board
  comment
}

class "Book::Line" as DetailLine <<borrowed row>> #EEF2FF {
  code
  name
  moves
}

class "Code" as DetailCode <<packed value>> #EEF2FF

DetailFile -[hidden]right- DetailBook
DetailBook -[hidden]right- DetailIndexed
DetailPosition -[hidden]right- DetailLine
DetailLine -[hidden]right- DetailCode
DetailBook -[hidden]down- DetailPosition

DetailFile --> DetailBook : load()
DetailPosition --> DetailBook : lookup board
DetailBook ...> DetailPosition : replay moves
DetailBook *--> "0..*" DetailIndexed : stores board
DetailBook --> DetailLine : prefix rows
DetailBook --> DetailCode : findEco()
@enduml
