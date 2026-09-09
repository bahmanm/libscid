# libscid C ABI Documentation

libscid is a high-performance C ABI chess library extracted from Scid and ScidUp, designed for applications requiring robust chess move generation, PGN parsing, database storage, and ECO classification without desktop UI dependencies.

_The published contract is the C11 ABI declared in `scid/scid.h`. The internal C++ engine is encapsulated and intentionally unexposed._

---

## Documentation Structure

The documentation is organised according to the Diataxis framework:

- [Tutorials](tutorials/quick-start.md): Learning-oriented guided lessons to get up and running with libscid. Start here if you are integrating libscid for the first time.
- [How-To Guides](how-to/index.md): Practical, recipe-oriented guides showing how to accomplish specific tasks including installation, PGN editing, variation tree traversal, and database searching.
- [C ABI Reference](reference/index_modules.md): Complete, authoritative technical specification of all public functions, opaque handles, structs, and status codes.
- [Explanation](explanation/architecture.md): Conceptual models, domain subsystems, architectural trade-offs, and design rationale behind the C ABI.

---

## Where to Begin

- Newcomer: Follow the [Quick Start Tutorial](tutorials/quick-start.md) to compile your first program with `cc`, Make, or CMake.
- Integration Tasks: Browse the [How-To Guides](how-to/index.md) to inspect code recipes for parsing PGN, managing game databases, or looking up opening classifications.
- Architecture and Design: Read [C ABI Design and Encapsulation](explanation/abi-design.md) and [Memory Management and Buffer Semantics](explanation/memory-model.md) to understand ownership invariants and buffer protocols.
- API Exploration: Jump directly into the [C ABI Modules Reference](reference/index_modules.md) for detailed symbol signatures and parameter documentation.
