# 070 ECO Lookup

This example loads a small ECO book through the C ABI, plays a Queen's Gambit
Declined position, and prints the matching ECO code and name.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/c/070-eco-lookup -B _build/examples/c/070-eco-lookup -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/c/070-eco-lookup
ctest --test-dir _build/examples/c/070-eco-lookup --output-on-failure
```
