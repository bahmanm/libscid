# API Reference

The `libscid` Python API surface is structured into four domain modules:

| Domain | Key Symbols | Related Guides |
| :--- | :--- | :--- |
| [Core](core.md) | [`Game`][libscid.Game], [`Cursor`][libscid.Cursor], [`Position`][libscid.Position], [`Arbiter`][libscid.Arbiter] | [Move Trees](../how-to/navigate-and-edit-trees.md), [Arbiter](../how-to/adjudicate-draws.md), [Cursor Immutability](../explanation/cursor-immutability.md) |
| [Database](database.md) | [`Database`][libscid.Database], [`DatabaseFilters`][libscid.DatabaseFilters], [`DatabaseSearch`][libscid.DatabaseSearch], [`HeaderCriteria`][libscid.HeaderCriteria], [`Filter`][libscid.Filter] | [Search Databases](../how-to/filter-and-search-databases.md), [Indexing & Filters](../explanation/indexing-and-filters.md) |
| [PGN & Events](pgn.md) | [`PgnOptions`][libscid.PgnOptions], [`Nag`][libscid.Nag], [`MoveMetadata`][libscid.MoveMetadata], [`iter_movetext`][libscid.iter_movetext], [`MovetextEvent`][libscid.MovetextEvent] | [Parse PGN](../how-to/parse-and-serialise-pgn.md), [Stream Events](../how-to/stream-movetext-events.md), [Event Streams](../explanation/event-stream-vs-tree-walk.md) |
| [Exceptions](errors.md) | [`LibScidError`][libscid.LibScidError] | [Bridge Architecture](../explanation/bridge-architecture.md) |

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
