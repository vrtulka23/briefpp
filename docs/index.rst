Brief++
========

Build reports from sections, paragraphs, figures, tables, equations, and other
content, then render the same document as Markdown, reStructuredText, HTML,
LaTeX, Typst, plain text, or JSON. Brief++ has a header-only C++17 API and a
Python package with native bindings. The C++ library has no runtime
dependencies; the Python package bundles the extension and the same headers.

Start with the :doc:`C++ guide <quickstart>` if you already have a C++
application, or the :doc:`Python guide <python>` if you are writing scripts or
notebooks. Both APIs build the same document model and use the same renderers.
The :doc:`document model <model>` explains how to compose richer content.
The :doc:`output guide <backends>` helps you choose a format and explains how
each renderer handles content it cannot represent directly.

Brief++ creates source files for LaTeX and Typst. To get a PDF, compile the
generated file with a separate tool such as ``pdflatex`` or ``typst``.

.. toctree::
   :maxdepth: 2

   quickstart
   python
   model
   backends
   roadmap
