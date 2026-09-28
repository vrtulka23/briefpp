Renderers and output behavior
=============================

``doc.write(path)`` selects a renderer from ``.md``, ``.rst``, ``.html``,
``.tex``, ``.typ``, ``.txt``, or ``.json``. Each renderer can also return a
string directly with ``render(doc)``. All seven text formats are generated
using only the C++ standard library. PDF compilation is external.

The same document can be written to every format:

.. code-block:: cpp

   report::Document doc;
   doc.title("Validation Report");
   auto& results = doc.section("Results").label("results").role("summary");
   results.paragraph().text("See ").reference("results").text(" for details.");
   results.equation("E = mc^2").label("energy");
   doc.write("report.md");
   doc.write("report.rst");
   doc.write("report.html");
   doc.write("report.tex");
   doc.write("report.typ");
   doc.write("report.txt");
   doc.write("report.json");

Support summary
---------------

.. list-table:: Semantic mapping
   :header-rows: 1
   :widths: 27 12 12 12 12 12 13

   * - Content
     - Markdown
     - RST
     - HTML
     - LaTeX
     - Typst
     - Text
   * - Sections, paragraphs, inline formatting
     - Native
     - Native
     - Native
     - Native
     - Native
     - Text only
   * - Figures, tables, lists, definitions
     - Native/MyST
     - Native/Sphinx
     - Native
     - Native
     - Native
     - Readable text
   * - Equations and citations
     - MyST/key
     - Math/key
     - Code/key
     - Native
     - Math/key
     - Text/key
   * - Horizontal rules, page breaks
     - Rule/marker
     - Rule/marker
     - Rule/marker
     - Native
     - Native
     - Rule/ignored
   * - IDs and roles
     - ID only
     - ID only
     - Both
     - ID only
     - ID only
     - Ignored

JSON writes the complete semantic tree, including IDs, roles, inline kinds,
metadata, and backend targets for raw blocks. Its document-level schema key is
``cpp-reportlib/1``. It is intended for inspection and interchange, not for
round-trip parsing by this library.

Degradation and raw content
---------------------------

Roles are styling hooks in HTML's ``class`` attribute. Other renderers keep
their content but ignore roles. The HTML and Markdown page-break markers use
``page-break`` as a class; an external print stylesheet can style it. Plain
text omits page breaks and inline styling while keeping readable content.
Citation keys appear as visible bracketed keys where no bibliography source
is configured. LaTeX emits ``\cite{key}``; users supply bibliography tooling
separately.

Math strings are passed through in Markdown, RST, and LaTeX. Typst sends
expressions without backslashes to its native math syntax; those expressions
must also be valid Typst math. A TeX expression containing a backslash becomes
a visible raw-code block in Typst, so generated Typst remains readable without
a TeX-to-Typst parser. HTML displays math strings as code.

``raw(Backend::Html, content)`` and ``raw(Backend::Typst, content)`` insert
backend-specific content directly. Other renderers ignore those blocks. The
same applies to Markdown, RST, LaTeX, and plain-text raw blocks. JSON records
all raw blocks and their target backend.

Backend configuration
---------------------

``LatexRenderer`` supports ``document_class()``, ``paper()``, ``font_size()``,
``package()``, ``style()`` (an alias for a package), and ``preamble()``. Use a
LaTeX ``.sty`` file or preamble content for detailed styling.

``HtmlRenderer`` supports repeated ``stylesheet(path)`` calls. It emits links
to external CSS files; no CSS is bundled. ``TypstRenderer`` supports
``preamble(content)`` for theme imports or show rules. These options are
renderer-specific and do not change the common document model.

All renderers escape ordinary text for their output syntax. Raw blocks and
math expressions are intentionally passed through. Figure paths are emitted
as supplied and must resolve from the output document's build context.
