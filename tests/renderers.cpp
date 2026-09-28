#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <reportlib/report.hpp>

static report::Document sample_document() {
    report::Document fragment;
    fragment.paragraph().text("See ").reference("plot").text(" and ").math("x^2");

    report::Document doc;
    doc.title("A & B").author("Ada").abstract("Results overview");
    auto& section = doc.section("Results").label("results");
    section.equation("E = mc^2", "energy");
    section.figure("plot.png").caption("A plot").label("plot").width(0.5);
    section.table().columns("Name", "Value").row("speed", "42").caption("Parameters");
    section.list(true).item("first").item("second");
    section.warning("Check units");
    doc << fragment;
    doc.raw(report::Backend::Latex, "\\newpage");
    return doc;
}

TEST_CASE("Markdown renders semantic content") {
    const auto output = report::MarkdownRenderer{}.render(sample_document());
    CHECK(output.find("# A & B") != std::string::npos);
    CHECK(output.find("```{math}\n:label: energy") != std::string::npos);
    CHECK(output.find(":name: plot") != std::string::npos);
    CHECK(output.find("| Name | Value |") != std::string::npos);
    CHECK(output.find("[](#plot)") != std::string::npos);
    CHECK(output.find("\\newpage") == std::string::npos);
}

TEST_CASE("RST renders Sphinx directives and references") {
    const auto output = report::RstRenderer{}.render(sample_document());
    CHECK(output.find(".. math::\n   :label: energy") != std::string::npos);
    CHECK(output.find(".. figure:: plot.png") != std::string::npos);
    CHECK(output.find(".. list-table:: Parameters") != std::string::npos);
    CHECK(output.find(":ref:`plot`") != std::string::npos);
    CHECK(output.find("\\newpage") == std::string::npos);
}

TEST_CASE("LaTeX renders a complete report") {
    const auto output = report::LatexRenderer{}.render(sample_document());
    CHECK(output.find("\\title{A \\& B}") != std::string::npos);
    CHECK(output.find("\\begin{equation}") != std::string::npos);
    CHECK(output.find("\\includegraphics[width=0.5\\linewidth]{plot.png}") != std::string::npos);
    CHECK(output.find("\\begin{longtable}{ll}") != std::string::npos);
    CHECK(output.find("\\ref{plot}") != std::string::npos);
    CHECK(output.find("\\newpage") != std::string::npos);
}

TEST_CASE("table row width is validated") {
    report::Document doc;
    CHECK_THROWS_AS(doc.table().columns("a", "b").row("one"), std::invalid_argument);
}

TEST_CASE("rich inline content is reusable across captions, cells, headings, and lists") {
    report::Document doc;
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

    const auto md = report::MarkdownRenderer{}.render(doc);
    CHECK(md.find("# Results at $t=1$") != std::string::npos);
    CHECK(md.find("Plot of **density**") != std::string::npos);
    CHECK(md.find("| **Density** | See [](#results) |") != std::string::npos);
    CHECK(md.find("- **Warning:** check units\n  - nested") != std::string::npos);

    const auto rst = report::RstRenderer{}.render(doc);
    CHECK(rst.find("Plot of **density**") != std::string::npos);
    CHECK(rst.find("     - See :ref:`results`") != std::string::npos);
    CHECK(rst.find("- **Warning:** check units\n  - nested") != std::string::npos);

    const auto tex = report::LatexRenderer{}.render(doc);
    CHECK(tex.find("\\caption{Plot of \\textbf{density}}") != std::string::npos);
    CHECK(tex.find("\\textbf{Density} & See \\ref{results}") != std::string::npos);
    CHECK(tex.find("\\item \\textbf{Warning:} check units") != std::string::npos);
}

TEST_CASE("incomplete rich table rows are rejected by renderers") {
    report::Document doc;
    doc.table().columns("A", "B").cell().text("only one");
    CHECK_THROWS_AS(report::MarkdownRenderer{}.render(doc), std::logic_error);
    CHECK_THROWS_AS(report::RstRenderer{}.render(doc), std::logic_error);
    CHECK_THROWS_AS(report::LatexRenderer{}.render(doc), std::logic_error);
}

static report::Document extended_document() {
    report::Document doc;
    doc.title("A < B & C");
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
    section.raw(report::Backend::Html, "<custom-element></custom-element>");
    section.raw(report::Backend::Typst, "#align(center)[Custom]");
    return doc;
}

TEST_CASE("new semantics render in existing formats") {
    const auto doc = extended_document();
    const auto md = report::MarkdownRenderer{}.render(doc);
    CHECK(md.find("[@smith2025]") != std::string::npos);
    CHECK(md.find("float\n: Floating-point parameter") != std::string::npos);
    CHECK(md.find("---\n\n") != std::string::npos);
    CHECK(md.find("class=\"page-break\"") != std::string::npos);

    const auto rst = report::RstRenderer{}.render(doc);
    CHECK(rst.find("[smith2025]") != std::string::npos);
    CHECK(rst.find("float\n   Floating-point parameter") != std::string::npos);
    CHECK(rst.find(".. raw:: html") != std::string::npos);

    const auto tex = report::LatexRenderer{}.package("booktabs").style("scinumtools")
        .preamble("\\newcommand{\\foo}{bar}").render(doc);
    CHECK(tex.find("\\cite{smith2025}") != std::string::npos);
    CHECK(tex.find("\\begin{description}") != std::string::npos);
    CHECK(tex.find("\\newpage") != std::string::npos);
    CHECK(tex.find("\\usepackage{booktabs}") != std::string::npos);
    CHECK(tex.find("\\usepackage{scinumtools}") != std::string::npos);
    CHECK(tex.find("\\newcommand{\\foo}{bar}") != std::string::npos);
}

TEST_CASE("HTML maps semantics and escapes text and attributes") {
    const auto output = report::HtmlRenderer{}.stylesheet("site.css?a=1&b=2").render(extended_document());
    CHECK(output.find("<title>A &lt; B &amp; C</title>") != std::string::npos);
    CHECK(output.find("href=\"site.css?a=1&amp;b=2\"") != std::string::npos);
    CHECK(output.find("<section id=\"results\" class=\"summary primary\">") != std::string::npos);
    CHECK(output.find("<cite data-cite-key=\"smith2025\">[smith2025]</cite>") != std::string::npos);
    CHECK(output.find("<dl><dt>float</dt><dd>Floating-point parameter</dd>") != std::string::npos);
    CHECK(output.find("<figure id=\"plot\" class=\"primary\">") != std::string::npos);
    CHECK(output.find("src=\"plot&lt;&amp;&quot;.png\"") != std::string::npos);
    CHECK(output.find("<table id=\"parameters\" class=\"parameter-table\">") != std::string::npos);
    CHECK(output.find("<div class=\"page-break\"></div>") != std::string::npos);
    CHECK(output.find("<custom-element></custom-element>") != std::string::npos);
    CHECK(output.find("#align(center)") == std::string::npos);
}

TEST_CASE("Typst and plain text retain semantic content") {
    const auto doc = extended_document();
    const auto typ = report::TypstRenderer{}.preamble("#set text(size: 11pt)").render(doc);
    CHECK(typ.find("#set text(size: 11pt)") != std::string::npos);
    CHECK(typ.find("== Results<results>") != std::string::npos);
    CHECK(typ.find("/ float: Floating-point parameter") != std::string::npos);
    CHECK(typ.find("#pagebreak()") != std::string::npos);
    CHECK(typ.find("#figure(image(") != std::string::npos);
    CHECK(typ.find("#figure(table(columns: 2, table.header(") != std::string::npos);
    CHECK(typ.find("#align(center)[Custom]") != std::string::npos);
    CHECK(typ.find("<custom-element>") == std::string::npos);

    const auto txt = report::PlainTextRenderer{}.render(doc);
    CHECK(txt.find("float: Floating-point parameter") != std::string::npos);
    CHECK(txt.find("See [results] and [smith2025]") != std::string::npos);
    CHECK(txt.find("Name | Value") != std::string::npos);
    CHECK(txt.find("#pagebreak") == std::string::npos);
}

TEST_CASE("JSON AST includes schema, roles, nested content, and escapes") {
    const auto json = report::JsonRenderer{}.render(extended_document());
    CHECK(json.find("\"schema\":\"cpp-reportlib/1\"") != std::string::npos);
    CHECK(json.find("\"title\":\"A < B & C\"") != std::string::npos);
    CHECK(json.find("\"roles\":[\"summary\",\"primary\"]") != std::string::npos);
    CHECK(json.find("\"type\":\"citation\",\"target\":\"smith2025\"") != std::string::npos);
    CHECK(json.find("\"type\":\"definition_list\"") != std::string::npos);
    CHECK(json.find("\"type\":\"page_break\"") != std::string::npos);
    CHECK(json.find("\"backend\":\"html\"") != std::string::npos);
    CHECK(report::detail::json_quote("\"\\\n\x01") == "\"\\\"\\\\\\n\\u0001\"");
}

TEST_CASE("ordinary text is escaped independently in each markup renderer") {
    report::Document doc;
    doc.paragraph("A * B & < C # [x] _ z");
    CHECK(report::MarkdownRenderer{}.render(doc).find("A \\* B & \\< C \\# \\[x\\] \\_ z") != std::string::npos);
    CHECK(report::RstRenderer{}.render(doc).find("A \\* B & < C # [x] _ z") != std::string::npos);
    CHECK(report::LatexRenderer{}.render(doc).find("A * B \\& \\textless{} C \\# [x] \\_ z") != std::string::npos);
    CHECK(report::HtmlRenderer{}.render(doc).find("A * B &amp; &lt; C # [x] _ z") != std::string::npos);
    CHECK(report::TypstRenderer{}.render(doc).find("A \\* B & \\< C \\# \\[x\\] \\_ z") != std::string::npos);
    CHECK(report::PlainTextRenderer{}.render(doc).find("A * B & < C # [x] _ z") != std::string::npos);
    CHECK(report::JsonRenderer{}.render(doc).find("\"value\":\"A * B & < C # [x] _ z\"") != std::string::npos);

    report::Document leading;
    leading.paragraph("- list\n.. raw:: html\n= heading");
    CHECK(report::MarkdownRenderer{}.render(leading).find("\\- list") != std::string::npos);
    CHECK(report::RstRenderer{}.render(leading).find("\\.. raw:: html") != std::string::npos);
    CHECK(report::TypstRenderer{}.render(leading).find("\\- list") != std::string::npos);
    CHECK(report::TypstRenderer{}.render(leading).find("\\= heading") != std::string::npos);
}
