Python binding
==============

Use the Python binding when a script already has the values for a report, such
as results from a simulation or measurements in a table. Install ``briefpp``
with ``pip install briefpp`` (Python 3.9 or newer). The package contains a
compiled extension and the C++ headers. Prebuilt wheels cover CPython 3.9
through 3.14 on Linux x86-64, Windows x64, and macOS Intel and Apple Silicon.
Building from source requires a C++17 compiler and CMake; pip installs the
Python build requirements automatically.

Your first Python report
------------------------

Save this as ``report.py`` and run ``python report.py``:

.. code-block:: python

   from briefpp import Document

   doc = Document().title("Atmospheric Simulation").author("Simulation Team")
   doc.section("Model").equation(r"\frac{dP}{dz} = -\rho g", "hydrostatic")

   results = doc.section("Results")
   results.figure("density.png").caption("Density profile").label("density").width(0.8)
   results.table().columns(["Parameter", "Value"]).row(["Temperature", "273.15 K"])
   results.paragraph().text("See ").reference("density").text(" for the profile.")

   print(doc.render("md"))
   doc.write("report.html")

``print`` shows the Markdown representation in your terminal. ``write`` saves
the HTML representation to a file. You can change the extension to ``.rst``,
``.tex``, ``.typ``, ``.txt``, or ``.json`` to save another representation. A
figure path is included in the generated file; the image itself is not copied.
Keep ``density.png`` alongside the output or adjust the path for your site.

Build a report from Python data
-------------------------------

You can add rows in a loop rather than assembling output text yourself:

.. code-block:: python

   from briefpp import Document

   readings = [("Morning", 18.2), ("Noon", 24.7), ("Evening", 20.1)]
   doc = Document().title("Temperature log")
   results = doc.section("Readings")
   table = results.table().columns(["Time", "Temperature (°C)"])
   for time, temperature in readings:
       table.row([time, f"{temperature:.1f}"])
   results.paragraph("Measurements were taken at one location.")
   doc.write("temperatures.md")

Use ``columns`` before ``row`` when the table has headers. A table without
headers can start with ``row``; its first row sets the number of columns. All
later rows must have the same width. If you need rich text inside a cell, set
``column_count(n)`` or ``columns`` first and fill cells one at a time:

.. code-block:: python

   table = doc.section("Checks").table().columns(["Check", "Status"])
   table.cell().text("Calibration")
   table.cell().strong("Passed")

Add styled text, lists, and reusable content
---------------------------------------------

Text builders let the renderer apply the right syntax for each output format:

.. code-block:: python

   section = doc.section("Interpretation")
   section.paragraph().text("The change was ").emphasis("small").text("; see ") \
       .link("source data", "https://example.org/data")
   steps = section.list(True)
   steps.item("Collect readings")
   steps.item("Check units")

Use ``label("name")`` on a node and ``reference("name")`` in a paragraph to
link to it. The same inline methods work on captions, headings, and list
items. Sections can contain subsections. Use ``DocumentFragment`` when you
want to build a group of blocks separately and copy them into a document:

.. code-block:: python

   from briefpp import DocumentFragment

   fragment = DocumentFragment()
   fragment.paragraph("Measurements were reviewed by two readers.")
   doc.append(fragment)

Render and customize output
---------------------------

``Document.render(format)`` returns a string. It accepts ``md`` or ``markdown``, ``rst``, ``tex`` or ``latex``, ``html``, ``typ`` or ``typst``, ``txt`` or ``text``, and ``json``. ``Document.write(path)`` selects a renderer from the file extension: ``.md``, ``.rst``, ``.tex``, ``.html``, ``.typ``, ``.txt``, or ``.json``. Pass a string path.

``Document`` and section nodes provide ``section``, ``paragraph``, ``equation``, ``figure``, ``table``, ``code_block``, ``list``, ``definition_list``, ``horizontal_rule``, ``page_break``, ``quote``, ``admonition``, ``warning``, ``note``, and ``raw``. A node's ``label`` and ``role`` methods set semantic IDs and roles. Paragraphs and other text nodes have ``text``, ``emphasis``, ``strong``, ``code``, ``math``, ``link``, ``reference``, and ``citation``. Rich text builders also come from ``section.heading()``, ``figure.caption()``, ``table.cell()``, ``list.item()``, and ``definition_list.definition_item()``. Keep a reference to a section when adding several blocks to it. Returned builders retain their owning document while they are in use.

To cite a source, register it on the document before rendering:

.. code-block:: python

   doc.bibliography_entry("smith2025", "A. Smith", "A study of reports", "2025",
                          "https://example.org/study")
   doc.paragraph().text("See ").citation("smith2025").text(" for details.")

The optional URL appears in the References section. Missing citation keys
raise ``ValueError`` when you render or write the document.

HTML output uses a responsive stylesheet and MathJax by default. Use
``HtmlRenderer().clean_html().render(doc)`` for plain HTML without CSS or
scripts, or ``HtmlRenderer().stylesheet("report.css").render(doc)`` for a
custom stylesheet. ``mathjax_source("path/to/tex-chtml.js")`` selects a local
MathJax script if the report must work without the CDN.

Tables accept lists of strings with ``columns(["A", "B"])`` and ``row(["1", "2"])``. For rich cells, call ``column_count(n)`` or ``columns(...)`` first, then add content with ``cell().text(...).strong(...)``. Use ``list(True)`` for a numbered list and ``list(False)`` for bullets. ``DocumentFragment`` is an alias of ``Document``; ``doc.append(fragment)`` copies its blocks into ``doc``.

For renderer settings, instantiate ``LatexRenderer``, ``HtmlRenderer``, or ``TypstRenderer`` and call ``render(doc)``. For example:

.. code-block:: python

   from briefpp import LatexRenderer

   latex = LatexRenderer().document_class("article").font_size(11).render(doc)

The other renderer classes are ``MarkdownRenderer``, ``RstRenderer``, ``PlainTextRenderer``, and ``JsonRenderer``. Backend-specific content can be added with ``doc.raw(Backend.LATEX, r"\newpage")`` after importing ``Backend``.

The bundled C++ include directory is returned by ``briefpp.get_include()``. The C++ library remains header-only and does not require the Python extension when used directly through CMake. See :doc:`model` for more about the document structure and :doc:`backends` for output behavior.
