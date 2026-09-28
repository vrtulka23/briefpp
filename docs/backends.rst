Renderers
=========

``MarkdownRenderer::render(doc)``, ``RstRenderer::render(doc)``, and ``LatexRenderer::render(doc)`` return strings. ``doc.write(path)`` selects a renderer from the ``.md``, ``.rst``, or ``.tex`` suffix. It throws on an unsupported suffix or file error.

Markdown uses readable CommonMark/GFM where possible and MyST ``math``, ``figure``, and admonition directives for scientific content. RST uses Sphinx math, figure, reference, and list-table directives. LaTeX emits a complete document and uses ``amsmath``, ``graphicx``, ``hyperref``, ``longtable``, and ``geometry``. Configure the latter with ``document_class()``, ``paper()``, and ``font_size()`` before calling ``render()``.

Raw blocks appear only in their selected backend. Output is UTF-8 text. Figure paths are emitted as supplied, so they must resolve in the output document's build context.
