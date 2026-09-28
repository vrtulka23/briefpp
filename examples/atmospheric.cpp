#include <briefpp/report.hpp>

#include <fstream>
#include <stdexcept>
#include <string>

namespace {

void write_rendered(const std::string& path, const std::string& content) {
    std::ofstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("cannot open " + path);
    file << content;
    if (!file) throw std::runtime_error("cannot write " + path);
}

} // namespace

int main() {
    using namespace briefpp;

    Document doc;
    doc.title("Atmospheric Simulation")
        .subtitle("A Brief++ feature tour")
        .author("Simulation Team")
        .date("28 September 2026")
        .institution("Example Research Lab")
        .abstract("A small, self-contained report showing the document model and all seven output formats.")
        .keywords({"atmosphere", "simulation", "reporting"});
    doc.bibliography_entry("example2026", "Brief++ contributors", "Brief++ project source and documentation",
                           "2026", "https://github.com/vrtulka23/briefpp");

    auto& introduction = doc.section("Introduction").label("introduction");
    introduction.heading().text(" and scope");
    introduction.paragraph()
        .text("This report combines ").strong("measurements")
        .text(" with an ").emphasis("idealized")
        .text(" model. The word ").code("Brief++")
        .text(" refers to the report builder. See the ")
        .link("project source", "https://github.com/vrtulka23/briefpp")
        .text(" for the code and ").reference("density-profile")
        .text(" for the generated figure. The report builder is documented by ")
        .citation("example2026").text(".")
        .role("summary");
    introduction.quote("A report should preserve the meaning of its content across formats.");
    introduction.note("The numbers below are illustrative rather than observational data.");

    auto& model = doc.section("Model").label("model");
    model.paragraph().text("The pressure model uses a scale height ")
        .math("H = 8.4\\,\\mathrm{km}").text(" under a constant-temperature assumption.");
    model.equation(R"(\frac{dP}{dz} = -\rho g)", "hydrostatic");
    model.equation(R"(P(z) = P_0 e^{-z/H})", "pressure-profile");
    model.paragraph().text("Equation ").reference("pressure-profile")
        .text(" describes the synthetic curve used here.");
    model.code_block("double pressure(double z, double p0, double h) {\n"
                     "    return p0 * std::exp(-z / h);\n}", "cpp");

    auto& results = doc.section("Results").label("results");
    auto& figure = results.figure("density.png").label("density-profile").width(0.8);
    figure.caption().text("Synthetic atmospheric ").strong("density")
        .text(" by altitude.");

    auto& measurements = results.table().columns("Parameter", "Value", "Unit")
        .caption("Illustrative sea-level conditions").label("conditions");
    measurements.row("Temperature", "273.15", "K")
        .row("Pressure", "101325", "Pa");
    measurements.cell().strong("Scale height");
    measurements.cell().text("8.4");
    measurements.cell().text("km");

    results.paragraph().text("The values in ").reference("conditions")
        .text(" produce the trend shown in ").reference("density-profile").text(".");

    auto& steps = results.list(true);
    steps.item("Set the starting conditions");
    steps.item("Review the output in every format");
    steps.item("Compare the rendered documents");
    auto& checks = results.list();
    auto& review = checks.item();
    review.strong("Check the details");
    review.list().item("Check the figure and table")
        .item("Check the linked references");

    auto& terms = results.definition_list();
    terms.item("Density", "Mass per unit volume");
    auto& scale_height = terms.definition_item();
    scale_height.term.strong("Scale height");
    scale_height.description.text("Altitude over which pressure falls by a factor of ")
        .math("e").text(".");

    auto& observations = results.section("Interpretation");
    observations.paragraph("The example emphasizes report structure, not atmospheric accuracy.");
    observations.warning("Do not use these illustrative values for a real forecast.");
    observations.admonition("tip", "Edit this source file and rebuild the demo to compare formats.");
    observations.horizontal_rule();

    auto& layout = doc.table().column_count(2).caption("Headerless summary table");
    layout.row("Input", "Illustrative profile")
        .row("Outputs", "Seven text formats and a PDF");

    DocumentFragment closing;
    closing.paragraph().text("This paragraph was added from a ").code("DocumentFragment")
        .text(" to demonstrate reusable content.");
    doc.append(closing);
    doc.page_break();

    auto& appendix = doc.section("Backend-specific notes");
    appendix.raw(Backend::Markdown, "**Markdown-only note:** MyST directives support figures and equations.");
    appendix.raw(Backend::Rst, ".. rubric:: RST-only note\n\nThis line appears only in reStructuredText.");
    appendix.raw(Backend::Latex, R"(\noindent\textit{LaTeX-only note.})");
    appendix.raw(Backend::Html, "<p class=\"backend-note\">HTML-only note.</p>");
    appendix.raw(Backend::Typst, "#align(center)[Typst-only note.]");
    appendix.raw(Backend::PlainText, "Plain-text-only note.");
    appendix.raw(Backend::Json, "JSON-only note stored in the semantic tree.");

    doc.write("report.md");
    doc.write("report.rst");
    doc.write("report.txt");
    doc.write("report.json");
    write_rendered("report.html", HtmlRenderer{}.stylesheet("report.css").render(doc));
    write_rendered("report.tex", LatexRenderer{}.role_environment("summary", "quote").render(doc));
    write_rendered("report.typ", TypstRenderer{}.preamble("#set page(margin: 25mm)").render(doc));
}
