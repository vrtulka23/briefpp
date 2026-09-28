# Brief++ — Focused Development Plan

## 1. Project Purpose

Brief++ (`briefpp` in code and package names) is a **small, dependency-free, header-only C++ library for programmatically constructing technical/scientific documents and rendering the same semantic document to several text-based output formats**.

Its primary use case is C++ software that already owns structured information—parameters, equations, validation results, tables, figures, metadata, algorithm descriptions, simulation results—and needs to emit documentation or reports directly from the C++ core.

Typical usage:

```cpp
briefpp::Document doc;

doc.title("Simulation Report");
doc.section("Parameters");
doc.paragraph("The following parameters were used.");
doc.table(...);
doc.equation(...);

doc.write("report.md");
doc.write("report.rst");
doc.write("report.tex");
```

The central abstraction is:

```text
C++ application
      |
      v
Semantic Document AST
      |
      +---------+---------+---------+---------+
      v         v         v         v         v
   Markdown    RST       HTML      LaTeX     Typst
```

The library SHOULD make this common case simple.

---

## 2. Strict Scope Boundary

The project MUST remain small and focused.

`briefpp` is NOT intended to become:

- a general-purpose markup language;
- a Markdown/MyST/RST parser;
- a Pandoc replacement;
- a Sphinx replacement;
- a TeX engine;
- a Typst engine;
- a PDF rendering engine;
- a word processor;
- a GUI document editor;
- a universal document converter;
- a CSS implementation;
- a LaTeX styling framework;
- a template programming language;
- a plotting library;
- an image-processing library;
- a bibliography management application;
- a general serialization framework;
- a web framework.

The core responsibility is deliberately narrow:

> **Represent common technical-document semantics and serialize them into useful text-based document formats.**

If a feature does not directly support this responsibility, it SHOULD remain outside the core.

---

## 3. Design Test for New Features

Before adding a feature, ask:

1. Is this a common semantic concept in technical/scientific documents?
2. Can multiple output backends represent it meaningfully?
3. Does it belong to the document model rather than one renderer?
4. Can it be implemented without introducing a mandatory dependency?
5. Does it materially improve programmatic report/document generation?

If the answer to several of these questions is "no", do not add it to the common AST.

Backend-specific functionality belongs in:

- renderer configuration;
- themes;
- raw backend blocks;
- external style files;
- optional helper layers.

Do not expand the common AST merely because one output language supports a feature.

---

## 4. Dependency Policy

The C++ core MUST remain:

- header-only;
- standard-library-only;
- free of mandatory third-party dependencies.

Native output generation MUST require no external programs for:

```text
.md
.rst
.html
.tex
.typ
.txt
.json
```

External tools MAY optionally be used after generation.

Examples:

```text
.tex -> pdflatex / xelatex / lualatex / latexmk -> PDF
.typ -> typst -> PDF
.md  -> external converter if desired
```

Such tools are NOT dependencies of the core library.

Python bindings, documentation tooling, and tests MAY have development/optional dependencies, but these MUST NOT leak into the C++ library.

---

## 5. Preserve the Semantic AST Principle

The AST represents meaning, not backend syntax.

Good:

```text
Heading
Paragraph
Strong
Link
Equation
Figure
Table
Citation
Reference
Admonition
```

Bad:

```text
LatexSection
HtmlDiv
CssClassNode
RstDirective
TypstShowRule
```

Backend-specific constructs MUST NOT become common node types unless they represent a genuinely backend-independent document concept.

Use raw backend content as the escape hatch:

```cpp
doc.raw(briefpp::Backend::Latex, R"(\newcommand{...})");
```

Do not distort the common AST to eliminate every possible use of raw content.

---

# 6. Current Development Goal

The current implementation already contains the main document primitives and Markdown, RST, and LaTeX rendering.

The next phase SHOULD NOT consist of continuously adding convenience methods.

The priority is:

1. make the existing AST structurally general;
2. add a small number of missing universal semantics;
3. add cheap renderers that naturally map from the AST;
4. improve backend configuration without creating a universal styling system;
5. stop expanding once the focused feature set is complete.

---

# 7. Common Node Attributes

Introduce common semantic attributes for relevant nodes.

Suggested representation:

```cpp
struct NodeAttributes {
    std::string id;
    std::vector<std::string> roles;
};
```

Provide fluent APIs where appropriate:

```cpp
node.label("results");
node.role("primary");
node.role("parameter-table");
```

Multiple roles MUST be supported.

Roles describe semantic purpose, not appearance.

Good:

```text
parameters
results
summary
primary
appendix
warning
```

Bad:

```text
blue
large-font
red-border
2cm-margin
```

Renderers may map roles to native styling hooks:

```text
HTML      -> class=""
RST/MyST  -> classes where supported
LaTeX     -> theme/style-specific handling
Typst     -> theme/style-specific handling
```

---

# 8. Reusable Inline Content

Rich inline content must not be restricted to paragraphs.

Introduce or consolidate a reusable abstraction similar to:

```cpp
class InlineContent {
public:
    std::vector<Inline> content;
};
```

It should support at least:

```text
Text
Emphasis
Strong
Code
Math
Link
Reference
Citation
```

Low-cost optional additions:

```text
Strikeout
LineBreak
```

Example:

```cpp
InlineContent content;

content.text("See ")
       .reference("eq-energy")
       .text(" for details.");
```

Simple string overloads MUST remain available.

This must remain easy:

```cpp
doc.paragraph("Simple text.");
```

---

# 9. Use Inline Content Consistently

Where semantically appropriate, replace permanently string-only representations with `InlineContent`.

Priority targets:

- section/heading titles;
- figure captions;
- table cells;
- list item text;
- definition-list terms/descriptions.

Target examples:

```cpp
figure.caption()
    .text("Density at ")
    .math("t = 10")
    .text(" s");
```

```cpp
table.cell()
    .text("See ")
    .reference("eq-energy");
```

```cpp
list.item()
    .strong("Warning:")
    .text(" invalid parameter.");
```

Do not force the rich API when a plain string is sufficient.

---

# 10. Generalize Lists

Do not permanently represent list items as strings.

Introduce explicit `ListItem` semantics.

The architecture should accommodate:

```text
List
├── ListItem
│   ├── Paragraph
│   └── Nested List
└── ListItem
    └── Paragraph
```

At minimum, list items MUST support rich inline content.

Nested block content may be implemented immediately if simple, but the representation MUST NOT prevent it later.

Example API:

```cpp
auto& list = doc.unordered_list();

list.item("First item");

list.item()
    .strong("Important:")
    .text(" second item.");
```

---

# 11. Add Citation Semantics

Add `Citation` as an inline semantic type.

Example:

```cpp
paragraph.text("The method follows ")
         .citation("smith2025")
         .text(".");
```

Initially a citation only needs a key.

Do NOT implement a complete bibliography-management system in this phase.

The AST should merely leave room for a later bibliography source.

---

# 12. Add Definition Lists

Introduce `DefinitionList`.

Example:

```cpp
auto& definitions = doc.definition_list();

definitions.item("float", "Floating-point parameter");
definitions.item("int", "Integer parameter");
definitions.item("str", "String parameter");
```

Prefer rich inline content internally.

Definition lists are useful for:

- parameter references;
- type documentation;
- terminology;
- API/property descriptions.

Support them in all applicable renderers.

---

# 13. Add Horizontal Rule

Introduce:

```cpp
doc.horizontal_rule();
```

Represent this as a semantic:

```text
HorizontalRule
```

Each renderer emits its native equivalent.

---

# 14. Add Page Break

Introduce:

```cpp
doc.page_break();
```

Represent this semantically as:

```text
PageBreak
```

Examples:

```text
LaTeX -> \newpage
Typst -> #pagebreak()
HTML  -> suitable print-oriented marker/class
```

Markdown/RST may emit an appropriate construct or gracefully degrade.

Do not introduce generic spacing/layout nodes such as arbitrary vertical space.

---

# 15. Consider AST Storage Before Further Expansion

The current tagged-node structure may contain fields that are meaningless for many node kinds.

Before substantially increasing the number of node types, evaluate whether the internal representation should remain a single tagged structure or move toward `std::variant`.

Possible direction:

```cpp
using NodeData = std::variant<
    Section,
    Paragraph,
    Equation,
    Figure,
    Table,
    List,
    DefinitionList,
    CodeBlock,
    Quote,
    Admonition,
    HorizontalRule,
    PageBreak,
    Raw
>;
```

Do NOT refactor merely for theoretical purity.

Refactor only if it materially improves:

- type safety;
- maintainability;
- renderer implementation;
- extensibility.

Preserve the public API where practical.

This decision should be made before the AST becomes significantly larger.

---

# 16. Add HTML Renderer

After the AST cleanup, implement a native HTML renderer.

Target:

```cpp
doc.write("report.html");
```

HTML generation MUST remain dependency-free.

Natural mappings include:

```text
Section       -> <section> + <hN>
Paragraph     -> <p>
Strong        -> <strong>
Emphasis      -> <em>
InlineCode    -> <code>
CodeBlock     -> <pre><code>
Link          -> <a>
Figure        -> <figure><img><figcaption>
Table         -> <table>
OrderedList   -> <ol>
UnorderedList -> <ul>
ListItem      -> <li>
Quote         -> <blockquote>
Admonition    -> <aside>
HorizontalRule-> <hr>
```

Common node attributes should map naturally:

```text
id    -> id=""
roles -> class=""
```

Example:

```cpp
table.label("parameters")
     .role("parameter-table");
```

may become:

```html
<table id="parameters" class="parameter-table">
```

Implement correct HTML escaping.

Do NOT implement CSS itself.

Allow external stylesheets and/or small renderer configuration hooks.

---

# 17. Add Typst Renderer

Add native Typst text generation:

```cpp
doc.write("report.typ");
```

Generation of `.typ` MUST require no dependency.

Compilation to PDF is external:

```text
report.typ -> typst compiler -> report.pdf
```

Map the common semantic AST to idiomatic Typst constructs.

Do NOT embed a Typst compiler.

Do NOT attempt to reproduce Typst's programming/styling language in C++.

Provide raw Typst output as an escape hatch if needed.

---

# 18. Add Plain-Text Renderer

Implement:

```cpp
doc.write("report.txt");
```

This renderer should produce readable plain text.

Uses include:

- terminal output;
- logs;
- CI artifacts;
- email bodies;
- debugging;
- semantic regression tests.

Unsupported presentation features should degrade gracefully rather than produce markup syntax.

---

# 19. Add JSON AST Serialization

Implement an optional native representation such as:

```cpp
doc.write("report.json");
```

Purpose:

- debugging;
- regression testing;
- AST inspection;
- interchange;
- future Python/JavaScript integration;
- caching.

Example structure:

```json
{
  "type": "section",
  "id": "results",
  "roles": ["primary"],
  "title": [
    {"type": "text", "value": "Results"}
  ],
  "children": []
}
```

Do not add a JSON dependency merely to write the library's own known structure.

Implement correct JSON escaping.

Include a small schema/version identifier at document level so the representation can evolve deliberately.

Do NOT turn briefpp into a general JSON library.

---

# 20. LaTeX Configuration

The LaTeX renderer should stop hardcoding all document styling decisions.

Provide a compact renderer configuration for high-level backend choices.

Useful configuration may include:

```cpp
briefpp::LatexRenderer latex;

latex.document_class("report")
     .package("booktabs")
     .package("siunitx")
     .style("scinumtools")
     .preamble(...);
```

A style call such as:

```cpp
latex.style("scinumtools");
```

may emit:

```latex
\usepackage{scinumtools}
```

Prefer native LaTeX styling mechanisms:

- `.sty` files;
- document classes;
- packages;
- preamble content.

Do NOT implement a giant universal C++ styling API for every LaTeX feature.

---

# 21. Styling Architecture

The common AST identifies **what something is**.

The renderer/theme decides **how it looks**.

Example:

```cpp
doc.table()
   .role("parameters");
```

The same semantic role may map to:

```text
HTML  -> CSS class
LaTeX -> style/environment/theme behavior
Typst -> show rule/theme behavior
RST   -> class/directive support
MD    -> ignored or minimally represented
```

Universal semantic selectors are useful.

Universal styling properties are NOT required.

Do not attempt to create a format-independent replacement for CSS, LaTeX styles, and Typst show rules.

---

# 22. Renderer Degradation Policy

Not every backend can represent every feature equally.

Renderers MUST prefer graceful semantic degradation over failure when possible.

Examples:

- a role unsupported by Markdown may be ignored;
- a page break may be ignored by plain text;
- sophisticated table presentation may become a basic table;
- styling hooks may disappear in formats without styling.

The semantic content should remain understandable.

Only fail when loss would make the output invalid or fundamentally misleading.

---

# 23. Raw Backend Content

Retain raw backend content as an explicit escape hatch.

Examples:

```cpp
doc.raw(briefpp::Backend::Latex, R"(\clearpage)");
doc.raw(briefpp::Backend::Html, R"(<custom-element></custom-element>)");
doc.raw(briefpp::Backend::Typst, "#...");
```

Raw nodes MUST clearly identify their target backend.

Other renderers should ignore incompatible raw nodes or follow the documented degradation policy.

Raw content is preferable to bloating the common AST with rare backend-specific constructs.

---

# 24. Document Fragments

Keep or strengthen composability through document fragments.

A component should be able to generate a reusable semantic fragment:

```cpp
briefpp::Fragment solver_documentation();
briefpp::Fragment validation_report();
briefpp::Fragment parameter_reference();
```

and compose them:

```cpp
briefpp::Document doc;

doc.append(solver_documentation());
doc.append(validation_report());
doc.append(parameter_reference());
```

This is important for applications such as SciNumTools3 where individual subsystems may generate their own documentation.

Do not turn fragments into a template programming language.

---

# 25. Testing Requirements

Every common semantic feature MUST have renderer tests.

For a feature such as `DefinitionList`, test at least:

```text
AST construction
Markdown output
RST output
LaTeX output
HTML output once implemented
Typst output once implemented
plain-text degradation where relevant
JSON representation
```

Prefer small deterministic golden/reference outputs.

Also test escaping independently for every renderer:

```text
Markdown escaping
RST escaping
LaTeX escaping
HTML escaping
Typst escaping
JSON escaping
```

Escaping correctness is a core library responsibility.

---

# 26. Documentation Requirements

Document:

- project purpose;
- scope/non-goals;
- semantic AST;
- supported nodes;
- renderer support matrix;
- degradation rules;
- raw backend content;
- node IDs and roles;
- LaTeX styling;
- HTML styling;
- Typst styling;
- dependency policy.

Include a compact example demonstrating one document rendered to multiple formats.

Example:

```cpp
briefpp::Document doc;

doc.title("Validation Report");
doc.section("Results");

doc.paragraph()
   .text("The result satisfies ")
   .strong("all validation criteria")
   .text(".");

doc.equation("E = mc^2")
   .label("eq-energy");

doc.write("report.md");
doc.write("report.rst");
doc.write("report.html");
doc.write("report.tex");
doc.write("report.typ");
doc.write("report.txt");
doc.write("report.json");
```

---

# 27. Implementation Order

Proceed in this order.

## Phase A — AST foundation

1. common `id` and `roles`;
2. reusable `InlineContent`;
3. rich captions;
4. rich table cells;
5. rich list items;
6. list representation capable of later nesting;
7. decide whether internal `std::variant` refactor is justified.

Do not add new renderers before this foundation is stable.

## Phase B — Small semantic additions

Add:

1. `Citation`;
2. `DefinitionList`;
3. `HorizontalRule`;
4. `PageBreak`.

Update existing Markdown, RST, and LaTeX renderers immediately.

## Phase C — Cheap native renderers

Implement in this order:

1. HTML;
2. Typst;
3. plain text;
4. JSON AST.

All remain dependency-free.

## Phase D — Backend configuration

Improve:

- LaTeX packages/style/preamble;
- HTML stylesheet hooks;
- Typst theme/raw hooks.

Keep these renderer-specific.

## Phase E — Stabilization

Before adding anything major:

- improve tests;
- document renderer support;
- document degradation behavior;
- simplify APIs;
- remove accidental duplication;
- evaluate compile-time/header-size impact.

---

# 28. Explicitly Deferred Features

Do NOT implement the following during this development phase unless required to fix an architectural blocker:

- Markdown parsing;
- MyST parsing;
- RST parsing;
- LaTeX parsing;
- HTML parsing;
- Typst parsing;
- DOCX generation;
- ODT generation;
- EPUB generation;
- embedded PDF generation;
- embedded TeX engine;
- embedded Typst engine;
- full bibliography database management;
- CSL processing;
- plotting;
- image manipulation;
- GUI;
- web server;
- JavaScript framework;
- general template language;
- arbitrary CSS abstraction;
- arbitrary LaTeX abstraction;
- arbitrary Typst abstraction;
- WYSIWYG editing.

These may be handled later by optional adapters or external tools if a demonstrated use case requires them.

---

# 29. Stop Condition

The project should be considered functionally broad enough when it can reliably represent:

```text
metadata
sections/headings
paragraphs
rich inline content
links
references
citations
math/equations
code
lists
definition lists
quotes
admonitions
figures
tables
horizontal rules
page breaks
raw backend content
document fragments
IDs
semantic roles
```

and render that model to:

```text
Markdown/MyST-oriented Markdown
reStructuredText
HTML
LaTeX
Typst
plain text
JSON AST
```

At that point, **do not keep adding features merely because another markup language contains them**.

New features should require a concrete technical/scientific reporting use case.

---

# 30. Long-Term Scope Principle

The value of `briefpp` comes from being small enough to embed confidently in another C++ project.

A feature that makes the library substantially larger, harder to compile, harder to understand, or dependent on another ecosystem must justify that cost.

The intended relationship is:

```text
large scientific/engineering application
                 |
                 | tiny embedded dependency
                 v
          briefpp
                 |
                 v
        semantic document
                 |
       +---------+---------+
       v         v         v
      docs     reports   artifacts
```

`briefpp` should remain infrastructure, not become an application platform.

The preferred philosophy is:

> **Small semantic core, simple composable API, multiple lightweight renderers, native escape hatches.**

When in doubt, keep the common core smaller.
