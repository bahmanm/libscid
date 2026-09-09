# How to Classify Positions with an ECO Opening Book

This guide demonstrates how to load an Encyclopaedia of Chess Openings (ECO) book file and classify board positions to determine their canonical ECO codes (e.g. `C68`) and descriptive opening names (e.g. `Ruy Lopez, Exchange Variation`) using the libscid C ABI.

---

## 1. Overview of Symbols

- `scid_eco_book_load`: Loads and parses an ECO opening file (such as `scid.eco`) into a searchable `scid_eco_book` handle.
- `scid_eco_book_code_find`: Identifies the most specific ECO code matching a given board position.
- `scid_eco_code_to_string`: Converts a numeric `scid_eco_code` into a standard alphanumeric string (e.g. `"C68"`).
- `SCID_ECO_STRING_CAPACITY`: Compile-time buffer capacity constant for ECO code string output.
- `scid_eco_book_name_find`: Retrieves the full human-readable opening name associated with an ECO code or position.
- `scid_eco_book_free`: Deallocates the ECO book handle.

---

## 2. Complete Recipe

```c
--8<-- "examples/c/070-eco-lookup/main.c"
```

---

## 3. Key Concepts and Patterns

### Loading Books and Resolving Openings

```c
scid_eco_book* book = NULL;
scid_eco_book_load("share/scid/scid.eco", &book);

scid_eco_code code = SCID_ECO_NONE;
scid_eco_book_code_find(book, position, &code);

char code_str[SCID_ECO_STRING_CAPACITY];
size_t code_str_size = 0;
scid_eco_code_to_string(code, code_str, sizeof(code_str), &code_str_size);

char opening_name[128];
size_t name_size = 0;
scid_eco_book_name_find(book, position, opening_name, sizeof(opening_name), &name_size);

printf("ECO: %.*s - %.*s\n", (int)code_str_size, code_str, (int)name_size, opening_name);

scid_eco_book_free(book);
```
