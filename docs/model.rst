Document model
==============

A ``Document`` contains metadata such as title and author, followed by blocks.
Blocks include sections, paragraphs, equations, figures, tables, lists, code,
quotes, and notes. A section can contain further blocks and subsections. You
describe *what* each block means once; each renderer chooses the output syntax.

Sections and rich text
----------------------

In C++, ``section()`` returns a ``Node&``. Keep that reference when you add
several blocks to a section. References remain valid while their parent
document or section exists:

.. code-block:: cpp

   briefpp::Document doc;
   auto& summary = doc.section("Summary").label("summary");
   summary.paragraph().text("The result is ").strong("positive")
       .text(". See ").reference("summary").text(" for context.");

Python has the same builder methods:

.. code-block:: python

   from briefpp import Document

   doc = Document()
   summary = doc.section("Summary").label("summary")
   summary.paragraph().text("The result is ").strong("positive")

Paragraphs support ``text``, ``emphasis``, ``strong``, ``code``, ``math``,
``link(text, url)``, ``reference(label)``, and ``citation(key)``. Use those
methods on ``section.heading()``, ``figure.caption()``, ``table.cell()``, or
``list.item()`` for rich text in those positions. For plain text, the simple
string overloads are shorter. Citation keys are retained; Brief++ does not
manage a bibliography database.

Tables and lists
----------------

For a table with headers, set columns before rows. Every row must have the
same number of cells:

.. code-block:: cpp

   summary.table().columns("Metric", "Value")
       .row("Count", "12")
       .row("Mean", "4.2");

For a headerless table, the first ``row`` establishes its width. To add rich
cell content, call ``column_count(n)`` or ``columns(...)`` first, then use
``cell()``. The renderer rejects an incomplete row:

.. code-block:: python

   table = summary.table().column_count(2)
   table.cell().text("Status")
   table.cell().strong("Passed")

``list(true)`` in C++ and ``list(True)`` in Python make numbered lists; use
``false`` or ``False`` for bullets. The zero-argument ``item()`` gives you a
rich text builder and can contain a nested list:

.. code-block:: python

   checks = summary.list()
   item = checks.item()
   item.strong("Verify")
   item.list().item("Check the units")

``definition_list().item(term, description)`` adds a plain definition. For
rich terms and descriptions, call ``definition_item()`` and fill its ``term``
and ``description`` builders.

Reusable blocks and format-specific content
-------------------------------------------

``DocumentFragment`` is an alias of ``Document``. In C++, ``doc << fragment``
copies the fragment's blocks. In Python, call ``doc.append(fragment)``. This
lets you build repeated sections independently before adding them to a report.

``label(id)`` gives a block an identifier and ``reference(id)`` refers to it.
Repeated ``role(name)`` calls add semantic roles. HTML renders roles as CSS
classes; LaTeX can map them to environments through
``LatexRenderer.role_environment``. Other renderers keep the content but
ignore those roles. Use ``raw(Backend::Latex, content)`` in C++ or
``raw(Backend.LATEX, content)`` in Python only for content meant for one
backend. Other backends omit that raw block.

Ordinary text is escaped by each renderer. Math expressions and raw blocks
pass through, so write math in the syntax expected by the target format.
Labels should use identifiers accepted by each output format you plan to
generate. See :doc:`backends` for details on format differences.
