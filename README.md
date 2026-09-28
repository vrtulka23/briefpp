# cpp-reportlib

A lightweight, embeddable C++17 library for generating technical and scientific reports in Markdown/MyST, reStructuredText/Sphinx, HTML, LaTeX, Typst, plain text, and JSON AST. The C++ core is header-only and has no external dependencies.

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
    doc.write("report.html");
    doc.write("report.typ");
    doc.write("report.txt");
    doc.write("report.json");
}
```

Run `cmake -S . -B build && cmake --build build && ctest --test-dir build` to build and test. The complete example is in [`examples/atmospheric.cpp`](examples/atmospheric.cpp).

To generate files you can inspect, run `cmake --build build --target reportlib_demo` after configuring. This creates all seven text formats in `build/demo` alongside the sample figure; with `pdflatex` installed, it also creates a PDF. Edit `examples/atmospheric.cpp` and run the target again to see your changes.

The standalone test build downloads doctest v2.5.3 through CMake FetchContent. When this repository is added as a subdirectory of another project, tests and examples default to off, so the header-only C++ library needs no download.

## Supported subset

The semantic model supports metadata, nested sections, reusable inline content, citations, equations, figures, tables, nested lists, definition lists, code blocks, quotes, admonitions, horizontal rules, page breaks, IDs, semantic roles, backend-specific raw blocks, and reusable document fragments. Markdown output uses MyST directives for equations, figures, and admonitions. RST output targets Sphinx. LaTeX, HTML, and Typst renderers have small backend-specific configuration APIs.

PDF compilation, parsing, and bibliography database management are outside the core. A `.tex` or `.typ` file can be compiled with an installed external tool. Figure files must exist at the paths used when rendering or compiling the report. See [renderer behavior and degradation](docs/backends.rst) for format-specific details.

The C++ API is documented in the [Sphinx documentation](docs/index.rst).

GitHub Actions runs the doctest suite and builds the Sphinx HTML website on pushes and pull requests. On pushes to `main`, it publishes the site to GitHub Pages after the tests pass. Enable **Settings → Pages → Build and deployment → GitHub Actions** in the repository once; the site will then be available at [vrtulka23.github.io/cpp-reportlib](https://vrtulka23.github.io/cpp-reportlib/). The `sphinx-site` artifact remains available from each workflow run.
