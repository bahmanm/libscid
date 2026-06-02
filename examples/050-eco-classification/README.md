# 050 ECO Classification

This example loads a small ECO book, plays a few moves on a
`scid::core::Position`, and classifies the resulting opening position.

Install libscid first; see the installation guide:

- https://libscid.bahmanm.com/installation.html

```sh
cmake -S examples/050-eco-classification -B _build/examples/050-eco-classification -DCMAKE_PREFIX_PATH=install/libscid
cmake --build _build/examples/050-eco-classification
ctest --test-dir _build/examples/050-eco-classification --output-on-failure
```
