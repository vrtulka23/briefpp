#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <briefpp/report.hpp>

static briefpp::Document sample_document() {
    briefpp::Document fragment;
    fragment.paragraph().text("See ").reference("plot").text(" and ").math("x^2");

    briefpp::Document doc;
    doc.title("A & B").author("Ada").abstract("Results overview");
    auto& section = doc.section("Results").label("results");
    section.equation("E = mc^2", "energy");
    section.figure("plot.png").caption("A plot").label("plot").width(0.5);
    section.table().columns("Name", "Value").row("speed", "42").caption("Parameters");
    section.list(true).item("first").item("second");
    section.warning("Check units");
    doc << fragment;
    doc.raw(briefpp::Backend::Latex, "\\newpage");
    return doc;
}

TEST_CASE("Markdown renders semantic content") {
    const auto output = briefpp::MarkdownRenderer{}.render(sample_document());
    CHECK(output.find("# A & B") != std::string::npos);
    CHECK(output.find("```{math}\n:label: energy") != std::string::npos);
    CHECK(output.find(":name: plot") != std::string::npos);
    CHECK(output.find("| Name | Value |") != std::string::npos);
    CHECK(output.find("[](#plot)") != std::string::npos);
    CHECK(output.find("\\newpage") == std::string::npos);
}

TEST_CASE("RST renders Sphinx directives and references") {
    const auto output = briefpp::RstRenderer{}.render(sample_document());
    CHECK(output.find(".. math::\n   :label: energy") != std::string::npos);
    CHECK(output.find(".. figure:: plot.png") != std::string::npos);
    CHECK(output.find(".. list-table:: Parameters") != std::string::npos);
    CHECK(output.find(":ref:`plot`") != std::string::npos);
    CHECK(output.find("\\newpage") == std::string::npos);
}

TEST_CASE("LaTeX renders a complete report") {
    const auto output = briefpp::LatexRenderer{}.render(sample_document());
    CHECK(output.find("\\title{A \\& B}") != std::string::npos);
    CHECK(output.find("\\begin{equation}") != std::string::npos);
    CHECK(output.find("\\includegraphics[width=0.5\\linewidth]{plot.png}") != std::string::npos);
    CHECK(output.find("\\begin{longtable}{ll}") != std::string::npos);
    CHECK(output.find("\\ref{plot}") != std::string::npos);
    CHECK(output.find("\\newpage") != std::string::npos);
}

TEST_CASE("table row width is validated") {
    briefpp::Document doc;
    CHECK_THROWS_AS(doc.table().columns("a", "b").row("one"), std::invalid_argument);
}

TEST_CASE("rich inline content is reusable across captions, cells, headings, and lists") {
    briefpp::Document doc;
    auto& section = doc.section("Results").label("results").role("summary").role("primary");
    section.heading().text(" at ").math("t=1");
    auto& figure = section.figure("plot.png");
    figure.caption().text("Plot of ").strong("density");
    auto& table = section.table().columns("Parameter", "Value");
    table.cell().strong("Density");
    table.cell().text("See ").reference("results");
    auto& list = section.list();
    auto& item = list.item();
    item.strong("Warning:").text(" check units");
    item.list().item("nested");

    CHECK(section.roles == std::vector<std::string>{"summary", "primary"});
    CHECK(table.rows.size() == 1);
    CHECK(list.items.front().children.size() == 1);

    const auto md = briefpp::MarkdownRenderer{}.render(doc);
    CHECK(md.find("# Results at $t=1$") != std::string::npos);
    CHECK(md.find("Plot of **density**") != std::string::npos);
    CHECK(md.find("| **Density** | See [](#results) |") != std::string::npos);
    CHECK(md.find("- **Warning:** check units\n  - nested") != std::string::npos);

    const auto rst = briefpp::RstRenderer{}.render(doc);
    CHECK(rst.find("Plot of **density**") != std::string::npos);
    CHECK(rst.find("     - See :ref:`results`") != std::string::npos);
    CHECK(rst.find("- **Warning:** check units\n  - nested") != std::string::npos);

    const auto tex = briefpp::LatexRenderer{}.render(doc);
    CHECK(tex.find("\\caption{Plot of \\textbf{density}}") != std::string::npos);
    CHECK(tex.find("\\textbf{Density} & See \\ref{results}") != std::string::npos);
    CHECK(tex.find("\\item \\textbf{Warning:} check units") != std::string::npos);
}

TEST_CASE("incomplete rich table rows are rejected by renderers") {
    briefpp::Document doc;
    doc.table().columns("A", "B").cell().text("only one");
    CHECK_THROWS_AS(briefpp::MarkdownRenderer{}.render(doc), std::logic_error);
    CHECK_THROWS_AS(briefpp::RstRenderer{}.render(doc), std::logic_error);
    CHECK_THROWS_AS(briefpp::LatexRenderer{}.render(doc), std::logic_error);
}

static briefpp::Document extended_document() {
    briefpp::Document doc;
    doc.title("A < B & C");
    doc.bibliography_entry("smith2025", "Smith, A.", "An example study", "2025", "https://example.org/study");
    auto& section = doc.section("Results").label("results").role("summary").role("primary");
    section.paragraph().text("See ").reference("results").text(" and ")
        .citation("smith2025").text(". ").link("source", "https://example.org/?a=1&b=2");
    auto& definitions = section.definition_list();
    definitions.item("float", "Floating-point parameter");
    auto& integer = definitions.definition_item();
    integer.term.strong("int");
    integer.description.text("Integer ").code("value");
    section.horizontal_rule();
    section.page_break();
    section.figure("plot<&\".png").caption("Plot <density>").label("plot").role("primary");
    section.table().columns("Name", "Value").row("density", "42").label("parameters").role("parameter-table");
    section.raw(briefpp::Backend::Html, "<custom-element></custom-element>");
    section.raw(briefpp::Backend::Typst, "#align(center)[Custom]");
    return doc;
}

TEST_CASE("new semantics render in existing formats") {
    const auto doc = extended_document();
    const auto md = briefpp::MarkdownRenderer{}.render(doc);
    CHECK(md.find("[[smith2025]](#bib-smith2025)") != std::string::npos);
    CHECK(md.find("<a id=\"bib-smith2025\"></a>") != std::string::npos);
    CHECK(md.find("float\n: Floating-point parameter") != std::string::npos);
    CHECK(md.find("---\n\n") != std::string::npos);
    CHECK(md.find("class=\"page-break\"") != std::string::npos);

    const auto rst = briefpp::RstRenderer{}.render(doc);
    CHECK(rst.find("[smith2025]_") != std::string::npos);
    CHECK(rst.find(".. [smith2025] Smith, A.") != std::string::npos);
    CHECK(rst.find("float\n   Floating-point parameter") != std::string::npos);
    CHECK(rst.find(".. raw:: html") != std::string::npos);

    const auto tex = briefpp::LatexRenderer{}.package("booktabs").style("scinumtools")
        .preamble("\\newcommand{\\foo}{bar}").render(doc);
    CHECK(tex.find("\\cite{smith2025}") != std::string::npos);
    CHECK(tex.find("\\bibitem{smith2025}") != std::string::npos);
    CHECK(tex.find("\\begin{description}") != std::string::npos);
    CHECK(tex.find("\\newpage") != std::string::npos);
    CHECK(tex.find("\\usepackage{booktabs}") != std::string::npos);
    CHECK(tex.find("\\usepackage{scinumtools}") != std::string::npos);
    CHECK(tex.find("\\newcommand{\\foo}{bar}") != std::string::npos);
}

TEST_CASE("citations require matching bibliography entries in nested rich text") {
    briefpp::Document doc;
    auto& section = doc.section("Sources");
    section.table().columns("Name").cell().citation("missing");
    CHECK_THROWS_AS(briefpp::MarkdownRenderer{}.render(doc), std::invalid_argument);
    CHECK_THROWS_AS(briefpp::JsonRenderer{}.render(doc), std::invalid_argument);
    doc.bibliography_entry("missing", "Author", "Title", "2026");
    CHECK(briefpp::MarkdownRenderer{}.render(doc).find("#bib-missing") != std::string::npos);
    CHECK_THROWS_AS(doc.bibliography_entry("missing", "Other", "Title", "2026"), std::invalid_argument);

    briefpp::Document fragment;
    fragment.bibliography_entry("fragment", "Author", "Fragment title", "2025");
    fragment.paragraph().citation("fragment");
    doc.append(fragment);
    CHECK(briefpp::LatexRenderer{}.render(doc).find("\\bibitem{fragment}") != std::string::npos);
}

TEST_CASE("HTML maps semantics and escapes text and attributes") {
    const auto output = briefpp::HtmlRenderer{}.stylesheet("site.css?a=1&b=2").render(extended_document());
    CHECK(output.find("cdn.jsdelivr.net/npm/mathjax@4/tex-chtml.js") != std::string::npos);
    CHECK(output.find("<style>") != std::string::npos);
    CHECK(output.find("<main class=\"briefpp-report\">") != std::string::npos);
    CHECK(output.find("<title>A &lt; B &amp; C</title>") != std::string::npos);
    CHECK(output.find("href=\"site.css?a=1&amp;b=2\"") != std::string::npos);
    CHECK(output.find("<section id=\"results\" class=\"summary primary\">") != std::string::npos);
    CHECK(output.find("href=\"#bib-smith2025\" class=\"citation\"") != std::string::npos);
    CHECK(output.find("<li id=\"bib-smith2025\">") != std::string::npos);
    CHECK(output.find("<dl><dt>float</dt><dd>Floating-point parameter</dd>") != std::string::npos);
    CHECK(output.find("<figure id=\"plot\" class=\"primary\">") != std::string::npos);
    CHECK(output.find("src=\"plot&lt;&amp;&quot;.png\"") != std::string::npos);
    CHECK(output.find("<table id=\"parameters\" class=\"parameter-table\">") != std::string::npos);
    CHECK(output.find("<div class=\"table-scroll\"><table") != std::string::npos);
    CHECK(output.find("<div class=\"page-break\"></div>") != std::string::npos);
    CHECK(output.find("<custom-element></custom-element>") != std::string::npos);
    CHECK(output.find("#align(center)") == std::string::npos);
}

TEST_CASE("HTML typesets math and clean mode retains readable source") {
    briefpp::Document doc;
    doc.paragraph().text("For ").math("a < b & c").text(" see below.");
    doc.equation(R"(\frac{a}{b} = 2)", "ratio");
    doc.table().columns("A").row("B");
    const auto rich = briefpp::HtmlRenderer{}.render(doc);
    CHECK(rich.find("<div class=\"table-scroll\"><table") != std::string::npos);
    CHECK(rich.find("<span class=\"math\">\\(a &lt; b &amp; c\\)</span>") != std::string::npos);
    CHECK(rich.find("<div id=\"ratio\" class=\"equation\">\\[\\frac{a}{b} = 2\\]</div>") != std::string::npos);
    const auto clean = briefpp::HtmlRenderer{}.stylesheet("site.css").clean_html().render(doc);
    CHECK(clean.find("<style>") == std::string::npos);
    CHECK(clean.find("<script") == std::string::npos);
    CHECK(clean.find("site.css") == std::string::npos);
    CHECK(clean.find("table-scroll") == std::string::npos);
    CHECK(clean.find("<code class=\"math\">a &lt; b &amp; c</code>") != std::string::npos);
    CHECK(clean.find("<div id=\"ratio\" class=\"equation\"><code>\\frac{a}{b} = 2</code></div>") != std::string::npos);
    CHECK(briefpp::HtmlRenderer{}.mathjax_source("local.js").render(doc).find("src=\"local.js\"") != std::string::npos);
}

TEST_CASE("Typst and plain text retain semantic content") {
    const auto doc = extended_document();
    const auto typ = briefpp::TypstRenderer{}.preamble("#set text(size: 11pt)").render(doc);
    CHECK(typ.find("#set text(size: 11pt)") != std::string::npos);
    CHECK(typ.find("== Results<results>") != std::string::npos);
    CHECK(typ.find("/ float: Floating-point parameter") != std::string::npos);
    CHECK(typ.find("#pagebreak()") != std::string::npos);
    CHECK(typ.find("#figure(image(") != std::string::npos);
    CHECK(typ.find("#figure(table(columns: 2, table.header(") != std::string::npos);
    CHECK(typ.find("#align(center)[Custom]") != std::string::npos);
    CHECK(typ.find("#link(label(\"bib-smith2025\"))") != std::string::npos);
    CHECK(typ.find("<bib-smith2025>") != std::string::npos);
    CHECK(typ.find("<custom-element>") == std::string::npos);

    const auto txt = briefpp::PlainTextRenderer{}.render(doc);
    CHECK(txt.find("float: Floating-point parameter") != std::string::npos);
    CHECK(txt.find("See [results] and [smith2025]") != std::string::npos);
    CHECK(txt.find("[smith2025] Smith, A. An example study. 2025.") != std::string::npos);
    CHECK(txt.find("Name | Value") != std::string::npos);
    CHECK(txt.find("#pagebreak") == std::string::npos);
}

TEST_CASE("JSON AST includes schema, roles, nested content, and escapes") {
    const auto json = briefpp::JsonRenderer{}.render(extended_document());
    CHECK(json.find("\"schema\":\"briefpp/1\"") != std::string::npos);
    CHECK(json.find("\"title\":\"A < B & C\"") != std::string::npos);
    CHECK(json.find("\"roles\":[\"summary\",\"primary\"]") != std::string::npos);
    CHECK(json.find("\"type\":\"citation\",\"target\":\"smith2025\"") != std::string::npos);
    CHECK(json.find("\"bibliography\":[{\"key\":\"smith2025\"") != std::string::npos);
    CHECK(json.find("\"type\":\"definition_list\"") != std::string::npos);
    CHECK(json.find("\"type\":\"page_break\"") != std::string::npos);
    CHECK(json.find("\"backend\":\"html\"") != std::string::npos);
    CHECK(briefpp::detail::json_quote("\"\\\n\x01") == "\"\\\"\\\\\\n\\u0001\"");
}

TEST_CASE("ordinary text is escaped independently in each markup renderer") {
    briefpp::Document doc;
    doc.paragraph("A * B & < C # [x] _ z");
    CHECK(briefpp::MarkdownRenderer{}.render(doc).find("A \\* B & \\< C \\# \\[x\\] \\_ z") != std::string::npos);
    CHECK(briefpp::RstRenderer{}.render(doc).find("A \\* B & < C # [x] _ z") != std::string::npos);
    CHECK(briefpp::LatexRenderer{}.render(doc).find("A * B \\& \\textless{} C \\# [x] \\_ z") != std::string::npos);
    CHECK(briefpp::HtmlRenderer{}.render(doc).find("A * B &amp; &lt; C # [x] _ z") != std::string::npos);
    CHECK(briefpp::TypstRenderer{}.render(doc).find("A \\* B & \\< C \\# \\[x\\] \\_ z") != std::string::npos);
    CHECK(briefpp::PlainTextRenderer{}.render(doc).find("A * B & < C # [x] _ z") != std::string::npos);
    CHECK(briefpp::JsonRenderer{}.render(doc).find("\"value\":\"A * B & < C # [x] _ z\"") != std::string::npos);

    briefpp::Document leading;
    leading.paragraph("- list\n.. raw:: html\n= heading");
    CHECK(briefpp::MarkdownRenderer{}.render(leading).find("\\- list") != std::string::npos);
    CHECK(briefpp::RstRenderer{}.render(leading).find("\\.. raw:: html") != std::string::npos);
    CHECK(briefpp::TypstRenderer{}.render(leading).find("\\- list") != std::string::npos);
    CHECK(briefpp::TypstRenderer{}.render(leading).find("\\= heading") != std::string::npos);
}

TEST_CASE("headerless tables render without invented header rows") {
    briefpp::Document doc;
    auto& table = doc.table().row("Value", "42").row("Units", "m/s");
    table.role("parameter-entry").label("parameter");
    CHECK(table.table_column_count == 2);
    CHECK(table.headers.empty());

    const auto md = briefpp::MarkdownRenderer{}.render(doc);
    CHECK(md.find("```{list-table}") != std::string::npos);
    CHECK(md.find(":header-rows: 0") != std::string::npos);
    CHECK(md.find(":name: parameter") != std::string::npos);
    CHECK(md.find("* - Value\n  - 42") != std::string::npos);

    const auto rst = briefpp::RstRenderer{}.render(doc);
    CHECK(rst.find(":header-rows: 0") != std::string::npos);
    CHECK(rst.find("   * - Value\n     - 42") != std::string::npos);

    const auto tex = briefpp::LatexRenderer{}.render(doc);
    CHECK(tex.find("\\begin{longtable}{ll}") != std::string::npos);
    CHECK(tex.find("Value & 42") != std::string::npos);
    CHECK(tex.find("\\hline") == std::string::npos);

    const auto html = briefpp::HtmlRenderer{}.render(doc);
    CHECK(html.find("<table id=\"parameter\" class=\"parameter-entry\"><tbody>") != std::string::npos);
    CHECK(html.find("<thead>") == std::string::npos);

    const auto typst = briefpp::TypstRenderer{}.render(doc);
    CHECK(typst.find("table(columns: 2, [Value], [42], [Units], [m/s])") != std::string::npos);
    CHECK(typst.find("table.header") == std::string::npos);

    const auto plain = briefpp::PlainTextRenderer{}.render(doc);
    CHECK(plain.find("Value | 42\nUnits | m/s") != std::string::npos);
    const auto json = briefpp::JsonRenderer{}.render(doc);
    CHECK(json.find("\"column_count\":2,\"headers\":[]") != std::string::npos);
}

TEST_CASE("headerless cell builder validates width") {
    briefpp::Document doc;
    auto& table = doc.table().column_count(2);
    table.cell().strong("Value");
    table.cell().text("42");
    CHECK(table.rows.size() == 1);
    CHECK_THROWS_AS(table.row("only one"), std::invalid_argument);
    CHECK_THROWS_AS(doc.table().column_count(0), std::invalid_argument);
    doc.table().column_count(2).cell().text("partial");
    CHECK_THROWS_AS(briefpp::LatexRenderer{}.render(doc), std::logic_error);
}

TEST_CASE("LaTeX role environments wrap nodes in role order") {
    briefpp::Document doc;
    doc.table().row("Value", "42").role("outer").role("inner").role("unused");
    doc.raw(briefpp::Backend::Html, "<p>ignored</p>").role("outer");
    briefpp::LatexRenderer renderer;
    renderer.role_environment("outer", "sntouter")
        .role_environment("inner", "sntinner")
        .table_column_spec("outer", "@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}")
        .table_column_spec("inner", "rr");
    const auto tex = renderer.render(doc);
    CHECK(tex.find("\\begin{sntouter}\n\\begin{sntinner}\n\\begin{longtable}{@{}p{0.25\\linewidth}p{0.69\\linewidth}@{}}") != std::string::npos);
    CHECK(tex.find("\\end{longtable}\n\\end{sntinner}\n\\end{sntouter}") != std::string::npos);
    CHECK(tex.find("<p>ignored</p>") == std::string::npos);
    CHECK(tex.find("\\begin{sntouter}", tex.find("\\end{sntouter}") + 1) == std::string::npos);
    CHECK(briefpp::LatexRenderer{}.render(doc).find("\\begin{longtable}{ll}") != std::string::npos);
    CHECK_THROWS_AS(renderer.role_environment("", "example"), std::invalid_argument);
    CHECK_THROWS_AS(renderer.role_environment("example", ""), std::invalid_argument);
    CHECK_THROWS_AS(renderer.table_column_spec("", "ll"), std::invalid_argument);
    CHECK_THROWS_AS(renderer.table_column_spec("example", ""), std::invalid_argument);
}
