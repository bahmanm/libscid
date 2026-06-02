# Architecture and Diagrams {#architecture}

These architecture notes and diagrams show libscid from domain-centred angles.
They are companions to the API reference: use them to understand ownership,
conversion points and the shape of the public model before drilling into
individual classes and functions.

@startuml architecture-overview
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam roundcorner 8
skinparam Nodesep 76
skinparam Ranksep 64
skinparam ArrowColor #4B5563
skinparam RectangleBorderColor #6B7280
skinparam RectangleBackgroundColor #EEF2FF

<style>
rectangle {
  Padding 28
  MinimumWidth 165
  MinimumHeight 48
}
</style>

top to bottom direction

rectangle "ECO" as Eco #EEF2FF

rectangle "Spelling" as Spelling #EEF2FF

rectangle "Database" as Database #EEF2FF

rectangle "Core" as Core #FFF7ED

Eco -[hidden]right- Spelling
Eco -[hidden]down- Database
Spelling -[hidden]down- Database
Database -[hidden]down- Core

Spelling --> Database : names
Database --> Core : games
Eco --> Core : positions
Eco --> Database : codes
@enduml

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage architecture_core "Core"
- @subpage architecture_database "Database"
- @subpage architecture_eco "ECO"
- @subpage architecture_spelling "Spelling"

\htmlonly
</div>
\endhtmlonly

---

@ref architecture_core "Core" is the chess model: games, positions, movetext,
notation and PGN.

---

@ref architecture_database "Database" stores and queries game collections while
materialising @ref architecture_core "Core" games on demand.

---

@ref architecture_eco "ECO" classifies opening positions and provides the
compact opening-code vocabulary.

---

@ref architecture_spelling "Spelling" is the name-authority layer for canonical
database names and player metadata.
