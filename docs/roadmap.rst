Roadmap
=======

The core supports programmatic composition, in-document bibliography entries, and seven text outputs. Future work should focus on renderer correctness, documentation, and keeping the public API small. Parsing, external bibliography database import, and PDF compilation remain outside the dependency-free C++ core. Python bindings are provided by a separate native extension in the Python package.

The C++ library is header-only and uses only the C++17 standard library. It is a semantic report generator, not a parser, document converter, plotting package, styling framework, or PDF engine. External LaTeX and Typst tools may compile generated text, but they are not required to generate it. New common nodes should represent useful concepts across several renderers rather than backend-specific syntax.
