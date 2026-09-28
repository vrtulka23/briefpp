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
