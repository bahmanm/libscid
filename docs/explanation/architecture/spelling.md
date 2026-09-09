# Spelling {#architecture_spelling}

Spelling is the name-authority layer above Database.  It loads Scid spelling
files, stores canonical names and aliases for each database name category, and
answers correction and metadata queries without mutating the database itself.

@ref architecture_spelling_name_normalization "Name Normalization" handles the
general prefix, infix and suffix rewrite rules.  @ref architecture_spelling_spell_checker
"Spell Checker" is the loaded index over canonical names and aliases.
@ref architecture_spelling_player_metadata "Player Metadata" is the
player-only side channel for title, country, dates, peak rating, biography and
rating history.

@startuml spelling-overview
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 68
skinparam Ranksep 54
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

rectangle "Spelling file" as File #EEF2FF

rectangle "Spell checker" as Checker #FFF7ED

rectangle "Name normalization" as Normalization #EEF2FF

rectangle "Canonical name lookup" as Lookup #EEF2FF

rectangle "Player metadata" as Metadata #EEF2FF

rectangle "Database names" as DatabaseNames #EEF2FF

File -[hidden]right- Checker
Checker -[hidden]right- Lookup
Normalization -[hidden]right- Metadata
Metadata -[hidden]right- DatabaseNames
Checker -[hidden]down- Metadata

File --> Checker : loads
Checker --> Normalization : rules
Checker --> Lookup : aliases
Checker --> Metadata : player records
DatabaseNames --> Checker : queries
Checker --> DatabaseNames : canonical names
@enduml

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage architecture_spelling_name_normalization "Name Normalization"
- @subpage architecture_spelling_spell_checker "Spell Checker"
- @subpage architecture_spelling_player_metadata "Player Metadata"

\htmlonly
</div>
\endhtmlonly

---

@ref architecture_spelling_name_normalization "Name normalization" is a broad
cleanup pass.  A caller can apply spelling-file Prefix, Infix and Suffix rules
for a database name category without performing canonical alias lookup.

---

@ref architecture_spelling_spell_checker "Spell checker" owns the loaded
strings.  It builds sorted alias indexes per database name category, removes
section-specific comparison characters from lookup keys, and returns borrowed
canonical-name pointers.

---

@ref architecture_spelling_player_metadata "Player metadata" is available only
for player names.  It interprets the comment plus Bio and Elo lines attached to
canonical player entries, again returning borrowed views owned by the loaded
spell checker.
