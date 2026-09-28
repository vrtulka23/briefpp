# cpp-reportlib

A lightweight, embeddable C++17 library for generating technical and scientific reports in Markdown/MyST, reStructuredText/Sphinx, and LaTeX. The C++ core is header-only and has no external dependencies.

## Quick start

Add `include/` to your compiler's include path, or use the optional CMake target:

```cmake
add_subdirectory(external/cpp-reportlib)
target_link_libraries(application PRIVATE reportlib::reportlib)
```

```cpp
#include <reportlib/report.hpp>

int main() {
    report::Document doc;
    doc.title("Simulation Report").author("Research Team");
    auto& results = doc.section("Results");
    results.paragraph().text("The simulation converged. See ")
        .reference("density-profile").text(" for the profile.");
    results.equation(R"(E = mc^2)", "energy");
    results.figure("density.png").caption("Density profile")
        .label("density-profile").width(0.8);
    results.table().columns("Parameter", "Value", "Unit")
        .row("Temperature", "273.15", "K");
    doc.write("report.md");
    doc.write("report.rst");
    doc.write("report.tex");
}
```

Run `cmake -S . -B build && cmake --build build && ctest --test-dir build` to build and test. The complete example is in [`examples/atmospheric.cpp`](examples/atmospheric.cpp).

To generate files you can inspect, run `cmake --build build --target reportlib_demo` after configuring. With `pdflatex` installed, this creates `build/demo/report.md`, `report.rst`, `report.tex`, and `report.pdf` alongside the sample figure. Edit `examples/atmospheric.cpp` and run the target again to see your changes.

The standalone test build downloads doctest v2.5.3 through CMake FetchContent. When this repository is added as a subdirectory of another project, tests and examples default to off, so the header-only C++ library needs no download.

## Supported subset

The semantic model supports metadata (title, subtitle, author, date, institution, abstract), nested sections, rich paragraphs (text, emphasis, strong, code, math, links, references), equations, figures, tables, lists, code blocks, quotes, admonitions, backend-specific raw blocks, and reusable document fragments. Markdown output uses MyST directives for equations, figures, and admonitions. RST output targets Sphinx. LaTeX output uses standard packages and configurable class, paper, and font size.

PDF compilation, Python bindings, MyST parsing, templates, bibliography management, and custom node registration are future work. A `.tex` file can be compiled with an installed TeX tool. Figure files must exist at the paths used when rendering or compiling the report.

The C++ API is documented in the [Sphinx documentation](docs/index.rst).

GitHub Actions runs the doctest suite and builds the Sphinx HTML website on pushes and pull requests. The website is available as the `sphinx-site` artifact from each workflow run.
