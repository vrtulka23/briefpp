Document model and C++ API
==========================

``briefpp::Document`` owns metadata and top-level semantic nodes. ``section()`` returns a ``briefpp::Node&`` that accepts nested sections and blocks. References returned by builders remain valid while their parent document or section exists. ``DocumentFragment`` is an alias of ``Document``; ``doc << fragment`` copies its nodes into another document.

Paragraphs support ``text()``, ``emphasis()``, ``strong()``, ``code()``, ``math()``, ``link(text, url)``, ``reference(label)``, and ``citation(key)``. The same ``InlineContent`` API is available through ``section.heading()``, ``figure.caption()``, ``table.cell()``, and ``list.item()``. Simple string overloads remain available. For a table with headers, call ``columns(...)`` before adding rows. For a headerless table, the first ``row(...)`` establishes its width; call ``column_count(n)`` first when building it with ``cell()``. Every row must have the same width. ``list(true)`` creates an ordered list; ``list(false)`` creates an unordered list. A list item can contain a nested list via ``item.list()``.

``definition_list().item(term, description)`` creates simple definitions. For rich terms and descriptions, use ``definition_item().term`` and ``definition_item().description``. ``horizontal_rule()`` and ``page_break()`` create semantic separators. Citation nodes store only a key; bibliography source management is external.

Nodes support ``label(id)`` and repeated ``role(name)`` calls for semantic IDs and roles. Roles are retained in the document model and become HTML classes. LaTeX can map selected roles to environments through ``LatexRenderer::role_environment()``. Unmapped roles are ignored in LaTeX; Markdown, RST, and Typst currently ignore all roles. Use ``raw(Backend::Latex, content)`` only for content intended for one backend.

The model stores semantic content, while renderers handle format-specific escaping and syntax. Equation strings are math expressions and are passed through to the target math syntax. Labels should use identifiers accepted by each intended backend.

The internal node remains a tagged structure for now. Its small set of node kinds and the existing public builder API do not justify a ``std::variant`` migration yet; this can be revisited as semantic kinds are added.
