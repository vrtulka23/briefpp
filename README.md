# Brief++

A lightweight, embeddable C++17 library for generating technical and scientific reports in Markdown/MyST, reStructuredText/Sphinx, HTML, LaTeX, Typst, plain text, and JSON AST. The C++ core is header-only and has no external dependencies.

## Quick start

Add `include/` to your compiler's include path, or use the optional CMake target:

```cmake
add_subdirectory(external/briefpp)
target_link_libraries(application PRIVATE briefpp::briefpp)
```

```cpp
#include <briefpp/report.hpp>

int main() {
    briefpp::Document doc;
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

To generate files you can inspect, run `cmake --build build --target briefpp_demo` after configuring. This creates all seven text formats in `build/demo` alongside the sample figure; with `pdflatex` installed, it also creates a PDF. Edit `examples/atmospheric.cpp` and run the target again to see your changes.

The standalone test build downloads doctest v2.5.3 through CMake FetchContent. When this repository is added as a subdirectory of another project, tests and examples default to off, so the header-only C++ library needs no download.

### PyPI header package

The `briefpp` Python distribution includes native bindings and the C++ headers. Install it with `pip install briefpp`, then create and render a report:

```python
from briefpp import Document

doc = Document().title("Experiment").author("Research team")
results = doc.section("Results")
results.paragraph().text("The answer is ").strong("42")
results.table().columns(["Measure", "Value"]).row(["Answer", "42"])
doc.write("report.md")
print(doc.render("html"))
```

See the [Python guide](docs/python.rst) for the full binding API, supported formats, and source builds. C++ users can obtain the installed header directory with `python -c 'import briefpp; print(briefpp.get_include())'` and pass it to their compiler with `-I`.

To publish a version, update both `project.version` in `pyproject.toml` and the version in `CMakeLists.txt` and push the changes. For automatic publishing, create a tag `v<version>` (for example, `v0.2.0`) and publish a GitHub release for it. For manual publishing without a tag, choose **Actions → Publish to PyPI → Run workflow** and select the branch containing the version you want to publish. The workflow builds the selected commit, verifies that the CMake and Python versions match (and checks the tag for release runs), builds platform wheels and a source archive, checks the package, and uploads the distributions to PyPI using Trusted Publishing. PyPI does not accept a second upload of the same version. Configure the PyPI trusted publisher for this repository and `.github/workflows/publish-pypi.yml`; no GitHub Actions API token secret is needed. If the PyPI publisher specifies a GitHub environment, set the same environment on the `publish` job.

## Supported subset

The semantic model supports metadata, nested sections, reusable inline content, citations, equations, figures, headered and headerless tables, nested lists, definition lists, code blocks, quotes, admonitions, horizontal rules, page breaks, IDs, semantic roles, backend-specific raw blocks, and reusable document fragments. Markdown output uses MyST directives for equations, figures, and admonitions. RST output targets Sphinx. LaTeX, HTML, and Typst renderers have small backend-specific configuration APIs; LaTeX can map roles to style-defined environments.

PDF compilation, parsing, and bibliography database management are outside the core. A `.tex` or `.typ` file can be compiled with an installed external tool. Figure files must exist at the paths used when rendering or compiling the report. See [renderer behavior and degradation](docs/backends.rst) for format-specific details.

The C++ API is documented in the [Sphinx documentation](docs/index.rst).

GitHub Actions runs the doctest suite and builds the Sphinx HTML website on pushes and pull requests. On pushes to `main`, it publishes the site to GitHub Pages after the tests pass. Enable **Settings → Pages → Build and deployment → GitHub Actions** in the repository once; the site will then be available at [vrtulka23.github.io/briefpp](https://vrtulka23.github.io/briefpp/). The `sphinx-site` artifact remains available from each workflow run.
