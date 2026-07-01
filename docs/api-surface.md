# API Surface {#api_surface}

The installed API surface is the C ABI declared by `scid/scid.h`.

libscid uses opaque handles for owned objects:

- `scid_position`
- `scid_game`
- `scid_game_cursor`
- `scid_eco_book`
- `scid_database`

Functions return `scid_error`; `SCID_OK` means success. Strings are passed as
UTF-8 `const char*` inputs or caller-owned output buffers with explicit
capacity and size parameters.

Objects created by libscid must be released with the matching `*_free`
function. The C++ implementation behind the ABI is not installed as a consumer
API.
