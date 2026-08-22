# API Reference

The `libscid` Python API surface is structured into four domain modules:

| Domain | Key Symbols | Description |
| :--- | :--- | :--- |
| [**Core**](core.md) | [`Game`][libscid.Game], [`Cursor`][libscid.Cursor], [`Position`][libscid.Position], [`Arbiter`][libscid.Arbiter] | Game container, immutable move tree cursors, board analysis, and draw adjudication. |
| [**Database**](database.md) | [`Database`][libscid.Database], [`DatabaseFilters`][libscid.DatabaseFilters], [`DatabaseSearch`][libscid.DatabaseSearch], [`HeaderCriteria`][libscid.HeaderCriteria], [`Filter`][libscid.Filter] | PGN/Scid database indexing, multi-criteria header queries, and filter bitsets. |
| [**PGN & Events**](pgn.md) | [`PgnOptions`][libscid.PgnOptions], [`Nag`][libscid.Nag], [`MoveMetadata`][libscid.MoveMetadata], [`iter_movetext`][libscid.iter_movetext], [`MovetextEvent`][libscid.MovetextEvent] | PGN serialization, numeric annotation glyphs, metadata, and event-driven AST streaming. |
| [**Exceptions**](errors.md) | [`LibScidError`][libscid.LibScidError] | Library root error raised when native operations or validation fail. |

---

## Complete Public Surface (`libscid.__all__`)

```python
__all__ = [
    # Core Game & Navigation
    "Game",
    "Cursor",
    "Position",
    "Arbiter",
    # Database, Search & Filtering
    "Database",
    "DatabaseFilters",
    "DatabaseSearch",
    "HeaderCriteria",
    "Filter",
    # PGN, Annotations & Events
    "PgnOptions",
    "Nag",
    "MoveMetadata",
    "iter_movetext",
    "MovetextEvent",
    "MovetextLineStart",
    "MovetextMove",
    "MovetextLineEnd",
    # Exceptions
    "LibScidError",
]
```
