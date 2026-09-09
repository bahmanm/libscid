# ECO {#architecture_eco}

ECO is the opening-classification layer.  It has two public halves:
@ref architecture_eco_code "Code" maps ECO text such as `B20` or `C50a1` to a
compact numeric value, and @ref architecture_eco_book "Book" loads a Scid ECO
text file and classifies exact @ref scid::core::Position "Position" values.

@startuml eco-overview
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

rectangle "ECO text file" as File #EEF2FF

rectangle "ECO classification" as Eco #FFF7ED

rectangle "Code mapping" as Code #EEF2FF

rectangle "Position lookup" as Position #EEF2FF

rectangle "Opening line views" as Lines #EEF2FF

rectangle "Database metadata" as Metadata #EEF2FF

File -[hidden]right- Eco
Eco -[hidden]right- Code
Position -[hidden]right- Lines
Lines -[hidden]right- Metadata
Eco -[hidden]down- Lines

File --> Eco : loads
Eco --> Code : parses / formats
Position --> Eco : exact board
Eco --> Lines : prefix rows
Code --> Metadata : compact value
@enduml

\htmlonly
<div style="display: none">
\endhtmlonly

- @subpage architecture_eco_code "Code"
- @subpage architecture_eco_book "Book"

\htmlonly
</div>
\endhtmlonly

---

@ref architecture_eco_code "Code" is the small value model.  It preserves
Scid's extended ECO forms, provides basic and extended display strings, expands
prefixes to their final subcode, and reduces full codes to database-statistics
buckets.

---

@ref architecture_eco_book "Book" is the classifier.  It reads the ECO source
file, replays each line from the standard starting position, stores the final
compact board, and later answers exact-position lookups or prefix listings.

---

ECO touches Database at the metadata boundary.  Database records carry a compact
ECO classification for browsing, sorting, searching and statistics, while the
ECO library supplies the public code vocabulary and the position classifier
that can produce those classifications.
