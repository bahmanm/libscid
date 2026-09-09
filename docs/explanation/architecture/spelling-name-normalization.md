# Name Normalization {#architecture_spelling_name_normalization}

Name normalization is the general rewrite layer in a spelling file.  It is not
alias lookup: @ref scid::spelling::NameNormalizer "NameNormalizer" applies
section-local Prefix, Infix and Suffix rules directly to a mutable string and
reports how many substitutions it made.

The rules are byte-exact and case-sensitive.  Prefix and suffix normalization
stop after the first matching rule in their respective lists; infix
normalization replaces every occurrence of each loaded fragment.

@startuml spelling-name-normalization
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 72
skinparam Ranksep 52
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

rectangle "Spelling rules" as Rules #EEF2FF

rectangle "Name normalizer" as Normalizer #FFF7ED

rectangle "Input name" as Input #EEF2FF

rectangle "Normalized name" as Output #EEF2FF

rectangle "Substitution count" as Count #EEF2FF

Rules -[hidden]right- Normalizer
Input -[hidden]right- Output
Output -[hidden]right- Count
Normalizer -[hidden]down- Output

Rules --> Normalizer : addPrefix() / addInfix() / addSuffix()
Input --> Normalizer : mutable string
Normalizer --> Output : rewrites
Normalizer --> Count : returns count
@enduml

## Domain Model

The normalizer stores three ordered rule lists.  Each rule is a quoted pair
from the spelling file: source fragment and replacement fragment.  The public
`addPrefix()`, `addInfix()` and `addSuffix()` methods parse those quoted pairs
and return a Core error code when the directive is malformed.

@startuml spelling-name-normalization-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 86
skinparam Ranksep 76
skinparam ArrowColor #4B5563
skinparam ClassBorderColor #6B7280
skinparam ClassBackgroundColor #EEF2FF
skinparam classAttributeIconSize 0

<style>
class {
  Padding 16
  MinimumWidth 128
  MinimumHeight 42
}
</style>

hide circle
hide empty methods

left to right direction

class "NameNormalizer" as DetailNormalizer <<rewriter>> #FFF7ED {
  normalize()
  addPrefix()
  addInfix()
  addSuffix()
}

class "Rule lists" as DetailRules <<ordered rules>> #EEF2FF {
  prefix_
  infix_
  suffix_
}

class "string" as DetailString <<mutable name>> #EEF2FF

class "errorT" as DetailError <<parse result>> #EEF2FF

DetailRules -[hidden]right- DetailNormalizer
DetailNormalizer -[hidden]right- DetailString
DetailString -[hidden]right- DetailError

DetailNormalizer *--> DetailRules : stores
DetailString --> DetailNormalizer : normalize()
DetailNormalizer --> DetailString : mutates
DetailNormalizer --> DetailError : add rules
@enduml
