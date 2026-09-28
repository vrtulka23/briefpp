Document model and C++ API
==========================

``report::Document`` owns metadata and top-level semantic nodes. ``section()`` returns a ``report::Node&`` that accepts nested sections and blocks. References returned by builders remain valid while their parent document or section exists. ``DocumentFragment`` is an alias of ``Document``; ``doc << fragment`` copies its nodes into another document.

Paragraphs support ``text()``, ``emphasis()``, ``strong()``, ``code()``, ``math()``, ``link(text, url)``, and ``reference(label)``. Figures support ``caption()``, ``label()``, and ``width(fraction)``. Tables require ``columns(...)`` before ``row(...)``, with matching cell counts. ``list(true)`` creates an ordered list; ``list(false)`` creates an unordered list. Use ``raw(Backend::Latex, content)`` only for content intended for one backend.

The model stores semantic content, while renderers handle format-specific escaping and syntax. Equation strings are math expressions and are passed through to the target math syntax. Labels should use identifiers accepted by each intended backend.
