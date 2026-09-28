Renderers and output behavior
=============================

Choose an output based on where readers will use the report:

* Markdown (``.md``) is useful in repositories and MyST documentation.
* reStructuredText (``.rst``) works with Sphinx documentation sites.
* HTML (``.html``) opens in a browser with a responsive report style and MathJax formulas.
* LaTeX (``.tex``) and Typst (``.typ``) are source files for typesetting.
* Plain text (``.txt``) is useful for logs and terminals.
* JSON (``.json``) preserves the semantic tree for inspection or another tool.

``doc.write(path)`` selects a renderer from the filename extension. Renderers
also return a string directly with ``render(doc)`` in C++, or
``doc.render("html")`` in Python. The C++ library generates all seven formats
without runtime dependencies. To make a PDF, compile a generated ``.tex`` or
``.typ`` file with a separate tool.

The same document can be written to every format:

.. code-block:: cpp

   briefpp::Document doc;
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

In Python, the equivalent is:

.. code-block:: python

   for extension in ("md", "rst", "html", "tex", "typ", "txt", "json"):
       doc.write(f"report.{extension}")

The output files do not embed referenced image files or custom stylesheets.
Copy those assets to the location expected by the output site or compiler.
The HTML renderer embeds its default CSS; MathJax is loaded from a CDN unless
you choose a local script or clean HTML mode.

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
     - MyST/linked
     - Math/linked
     - Typeset/linked
     - Code/linked
     - Math/linked
     - Text/entry
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
     - ID/mapped roles
     - ID only
     - Ignored

JSON writes the complete semantic tree, including IDs, roles, inline kinds,
metadata, and backend targets for raw blocks. Its document-level schema key is
``briefpp/1``. It is intended for inspection and interchange, not for
round-trip parsing by this library.

Degradation and raw content
---------------------------

Roles are styling hooks in HTML's ``class`` attribute. LaTeX can wrap nodes
with configured environments for selected roles. Other renderers keep their
content but ignore roles. The HTML and Markdown page-break markers use
``page-break`` as a class; an external print stylesheet can style it. Plain
text omits page breaks and inline styling while keeping readable content.
Bibliography entries are emitted as a References section in every text format.
Markdown, RST, HTML, and Typst citations link to their entry. LaTeX emits
``\cite{key}`` and an inline ``thebibliography`` environment, so PDF builds
need no external bibliography database. Plain text shows bracketed keys and
matching entries; JSON includes the bibliography data. Rendering fails if a
citation key has no entry.

Math strings are passed through in Markdown, RST, and LaTeX. Typst sends
expressions without backslashes to its native math syntax; those expressions
must also be valid Typst math. A TeX expression containing a backslash becomes
a visible raw-code block in Typst, so generated Typst remains readable without
a TeX-to-Typst parser. HTML wraps TeX math in MathJax delimiters; clean HTML
shows the source in code elements.

HTML appearance and math
------------------------

The default HTML renderer includes a responsive report stylesheet and loads
MathJax 4 from jsDelivr. Inline ``math()`` and block ``equation()`` content
are typeset in the browser. The TeX source remains visible if the script
cannot load. The page has a viewport tag, a centered width limit, a mobile
padding breakpoint, fluid images, and horizontal scrolling for wide tables
and code blocks. A custom stylesheet can override the built-in design:

.. code-block:: cpp

   auto html = briefpp::HtmlRenderer{}
       .stylesheet("report.css")
       .render(doc);

For offline math, put a MathJax component and its required assets on your
site and set ``mathjax_source("path/to/tex-chtml.js")``. To emit plain HTML
with no embedded CSS, stylesheet links, or MathJax script, call
``clean_html()``. Math remains readable as escaped TeX in ``<code>`` elements:

.. code-block:: cpp

   auto clean = briefpp::HtmlRenderer{}.clean_html().render(doc);

The same options are available from Python:

.. code-block:: python

   from briefpp import HtmlRenderer

   styled = HtmlRenderer().stylesheet("report.css").render(doc)
   clean = HtmlRenderer().clean_html().render(doc)

``raw(Backend::Html, content)`` and ``raw(Backend::Typst, content)`` insert
backend-specific content directly. Other renderers ignore those blocks. The
same applies to Markdown, RST, LaTeX, and plain-text raw blocks. JSON records
all raw blocks and their target backend.

Backend configuration
---------------------

``LatexRenderer`` supports ``document_class()``, ``paper()``, ``font_size()``,
``package()``, ``style()`` (an alias for a package), and ``preamble()``. Use a
LaTeX ``.sty`` file or preamble content for detailed styling. Map a semantic
role to an environment defined by that style:

.. code-block:: cpp

   auto& details = doc.table().row("Value", "42").row("Units", "m/s");
   details.role("parameter-entry");

   briefpp::LatexRenderer latex;
   latex.style("sntreport")
        .role_environment("parameter-entry", "sntentry")
        .table_column_spec("parameter-entry",
                           R"(@{}p{0.25\linewidth}p{0.69\linewidth}@{})");
   auto tex = latex.render(doc);

The role wraps the entire node with ``\begin{sntentry}`` and
``\end{sntentry}``. Multiple mapped roles nest in the order they were added
and close in reverse order. Unmapped roles leave the output unchanged. Choose
environments that permit the contained LaTeX structure; a nonbreakable box is
not suitable around a long table. ``table_column_spec()`` changes the raw
LaTeX column specification for tables with the selected role. The first
matching role wins; unmapped tables keep left-aligned columns. The caller is
responsible for providing a specification matching the table's column count.

Headerless tables use MyST ``list-table`` with zero header rows, RST
``list-table`` with zero header rows, and native tables without invented
headings in HTML, LaTeX, and Typst.

``HtmlRenderer`` supports repeated ``stylesheet(path)`` calls. It emits links
to external CSS files; no CSS is bundled. ``TypstRenderer`` supports
``preamble(content)`` for theme imports or show rules. These options are
renderer-specific and do not change the common document model.

For example, Python can render an HTML page that references your stylesheet:

.. code-block:: python

   from briefpp import HtmlRenderer

   html = HtmlRenderer().stylesheet("report.css").render(doc)
   with open("report.html", "w", encoding="utf-8") as output:
       output.write(html)

All renderers escape ordinary text for their output syntax. Raw blocks and
math expressions are intentionally passed through. Figure paths are emitted
as supplied and must resolve from the output document's build context.
