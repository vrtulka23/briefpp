#include <reportlib/report.hpp>

int main() {
    report::Document doc;
    doc.title("Atmospheric Simulation").author("Simulation Team");
    doc.section("Introduction").paragraph("This report contains the results of the atmospheric simulation.");
    doc.section("Model").equation(R"(\frac{dP}{dz} = -\rho g)", "hydrostatic");

    auto& results = doc.section("Results");
    results.figure("density.png").caption("Atmospheric density profile").label("density-profile").width(0.8);
    results.table().columns("Parameter", "Value", "Unit")
        .row("Temperature", "273.15", "K")
        .row("Pressure", "101325", "Pa");
    results.paragraph().text("See ").reference("density-profile").text(" for the profile.");

    doc.write("report.md");
    doc.write("report.rst");
    doc.write("report.tex");
    doc.write("report.html");
    doc.write("report.typ");
    doc.write("report.txt");
    doc.write("report.json");
}
