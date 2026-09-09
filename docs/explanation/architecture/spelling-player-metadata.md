# Player Metadata {#architecture_spelling_player_metadata}

Player metadata is the player-only payload attached to canonical names in a
spelling file.  @ref scid::spelling::PlayerInfo "PlayerInfo" interprets the
comment after a player entry, while @ref scid::spelling::PlayerElo "PlayerElo"
stores optional rating history from Elo lines.

The spell checker returns this data only for unambiguous player lookups.  A name
that is absent or matches more than one canonical player produces no metadata,
because there is no single record to return.

@startuml spelling-player-metadata
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

rectangle "Canonical player" as Player #EEF2FF

rectangle "Player metadata" as Metadata #FFF7ED

rectangle "Comment fields" as Comment #EEF2FF

rectangle "Biography rows" as Bio #EEF2FF

rectangle "Elo history" as Elo #EEF2FF

rectangle "Game date" as Date #EEF2FF

Player -[hidden]right- Metadata
Metadata -[hidden]right- Comment
Bio -[hidden]right- Elo
Elo -[hidden]right- Date
Metadata -[hidden]down- Bio

Player --> Metadata : unambiguous lookup
Metadata --> Comment : comment fields
Metadata --> Bio : biography
Metadata --> Elo : ratings
Date --> Elo : game date
@enduml

## Domain Model

@ref scid::spelling::PlayerInfo "PlayerInfo" is a parser over an owned comment
string.  It extracts Scid's conventional title prefix, the final
three-character country field, peak rating in brackets, birth date and optional
death date.  The raw comment remains available through
@ref scid::spelling::PlayerInfo::getComment "getComment()".

@ref scid::spelling::PlayerElo "PlayerElo" stores the rating rows as
year/rating pairs.  @ref scid::spelling::PlayerElo::getElo "getElo()" maps a
Core date to the correct slot using Scid's historical publication cadence
rules, returning zero when the data has no usable rating for that date.

@startuml spelling-player-metadata-domain-model
skinparam backgroundColor #FFFFFF
skinparam shadowing false
skinparam linetype ortho
skinparam defaultFontName Cantarel
skinparam defaultFontSize 10
skinparam classFontName "Source Code Pro"
skinparam classAttributeFontName "Source Code Pro"
skinparam roundcorner 8
skinparam Nodesep 88
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

class "SpellChecker" as DetailChecker <<loaded index>> #EEF2FF {
  getPlayerInfo()
  getPlayerElo()
  hasEloData()
}

class "PlayerInfo" as DetailInfo <<comment parser>> #FFF7ED {
  getTitle()
  getLastCountry()
  getBirthdate()
  getDeathdate()
  getPeakRating()
  getComment()
}

class "PlayerElo" as DetailElo <<rating history>> #FFF7ED {
  addEloData()
  getElo()
}

class "Biography lines" as DetailBio <<borrowed strings>> #EEF2FF

class "dateT" as DetailDate <<core date>> #EEF2FF

class "ratingT" as DetailRating <<core rating>> #EEF2FF

DetailChecker -[hidden]right- DetailInfo
DetailInfo -[hidden]right- DetailElo
DetailBio -[hidden]down- DetailInfo
DetailDate -[hidden]right- DetailRating
DetailElo -[hidden]down- DetailDate

DetailChecker --> DetailInfo : player
DetailChecker --> DetailElo : player
DetailInfo --> DetailBio : bio
DetailDate --> DetailElo : date
DetailElo --> DetailRating : rating
@enduml
