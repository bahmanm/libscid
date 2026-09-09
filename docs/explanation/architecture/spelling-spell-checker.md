# Spell Checker {#architecture_spelling_spell_checker}

Spell checker is the loaded spelling-file index.  @ref scid::spelling::SpellChecker::create
"SpellChecker::create()" reads the file, builds canonical-name tables and
sorted alias indexes for each database `nameT`, and owns the strings returned
by lookup.

The lookup contract is deliberately simple.  @ref scid::spelling::SpellChecker::find
"find()" removes the comparison characters configured by the current spelling
section, searches the sorted alias index, and returns unique canonical names up
to the requested limit.  An exact alias wins over prefix matches.  Player lookup
adds Scid's surname-last convenience rule when the first lookup finds nothing.

@startuml spelling-spell-checker
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 72
skinparam Ranksep 56
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

<style>
rectangle {
  Padding 24
  MinimumWidth 155
  MinimumHeight 44
}
</style>

left to right direction

rectangle "Spelling file" as File #EEF2FF

rectangle "Spell checker" as Checker #FFF7ED

rectangle "Canonical names" as Names #EEF2FF

rectangle "Alias indexes" as Aliases #EEF2FF

rectangle "General corrections" as Corrections #EEF2FF

rectangle "Lookup result" as Result #EEF2FF

File -[hidden]right- Checker
Checker -[hidden]right- Aliases
Aliases -[hidden]right- Names
Names -[hidden]right- Result
Checker -[hidden]down- Corrections

File --> Checker : create()
Checker --> Aliases : searches
Checker --> Corrections : exposes
Aliases --> Names : canonical id
Names --> Result : borrowed names
@enduml

## Domain Model

The public object is intentionally immutable after loading.  The private loader
is stateful while it reads the file: it tracks the current name section and the
current canonical name so that aliases, normalisation rules, biography lines
and Elo rows attach to the right table.  Once loading succeeds,
@ref scid::spelling::SpellChecker "SpellChecker" is a read-only query object.

Returned names are borrowed pointers into the spell checker.  The same lifetime
rule applies to @ref scid::spelling::SpellChecker::getGeneralCorrections
"getGeneralCorrections()", @ref scid::spelling::SpellChecker::getPlayerInfo
"getPlayerInfo()" and @ref scid::spelling::SpellChecker::getPlayerElo
"getPlayerElo()": keep the spell checker alive while using returned views.

@startuml spelling-spell-checker-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 90
skinparam Ranksep 78
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 132
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

left to right direction

class "SpellChecker" as DetailChecker <<loaded index>> #FFF7ED {
  create()
  find()
  getGeneralCorrections()
  getPlayerInfo()
  getPlayerElo()
  hasEloData()
  numCorrectNames()
}

class "nameT" as DetailNameType <<database category>> #EEF2FF

class "NameNormalizer" as DetailNormalizer <<general rules>> #EEF2FF

class "Canonical names" as DetailNames <<owned strings>> #EEF2FF

class "Alias index" as DetailAlias <<sorted keys>> #EEF2FF {
  alias
  canonical index
}

class "Progress" as DetailProgress <<load callback>> #EEF2FF

DetailProgress -[hidden]right- DetailChecker
DetailChecker -[hidden]right- DetailNameType
DetailNameType -[hidden]right- DetailNormalizer
DetailNames -[hidden]right- DetailAlias
DetailChecker -[hidden]down- DetailNames

DetailProgress --> DetailChecker : create()
DetailChecker --> DetailNameType : per section
DetailChecker *--> DetailNormalizer : owns
DetailChecker *--> DetailNames : owns
DetailChecker *--> DetailAlias : owns
DetailAlias --> DetailNames : canonical id
@enduml
