C++ guide
=========

Brief++ is a header-only C++17 library. Add this repository to your build or
put ``include/`` on your compiler's include path. Your application creates a
``Document``, adds content, and writes one or more output files.

Build your first report
-----------------------

With CMake, add Brief++ as a subdirectory and link its interface target:

.. code-block:: cmake

   add_subdirectory(external/briefpp)
   add_executable(report main.cpp)
   target_link_libraries(report PRIVATE briefpp::briefpp)

Create ``main.cpp``:

.. code-block:: cpp

   #include <briefpp/report.hpp>

   int main() {
       briefpp::Document doc;
       doc.title("Field measurements").author("Research team");

       auto& results = doc.section("Results").label("results");
       results.paragraph().text("The measured value was ").strong("42").text(" units.");
       results.table().columns("Parameter", "Value")
           .row("Sample count", "12")
           .row("Mean", "42");
       results.note("Values are rounded to whole units.");

       doc.write("report.md");
       doc.write("report.html");
   }

The two files contain the same content in different formats. ``write()``
chooses a renderer from the filename extension. To get output as a string,
call a renderer directly:

.. code-block:: cpp

   const std::string html = briefpp::HtmlRenderer{}.render(doc);

Use Brief++ without CMake
-------------------------

For a single source file, compile with a C++17 compiler and the include path:

.. code-block:: console

   c++ -std=c++17 -I path/to/briefpp/include main.cpp -o report
   ./report

The PyPI package also installs the C++ headers. If you have already installed
it, ``python -c 'import briefpp; print(briefpp.get_include())'`` prints the
path to pass with ``-I``.

Add common report content
-------------------------

A section can contain subsections and blocks. Keep its returned reference
when you want to add several blocks under the same heading:

.. code-block:: cpp

   auto& methods = doc.section("Methods");
   methods.paragraph("We sampled the site once per hour.");
   methods.equation(R"(E = mc^2)", "energy");

   auto& observations = methods.section("Observations");
   observations.figure("chart.png").caption("Hourly readings")
       .label("readings").width(0.7);
   observations.paragraph().text("See ").reference("readings")
       .text(" for the trend.");

``label()`` names a block; ``reference()`` links to that label in formats that
support references. Figure paths are written into the output as given. Put the
image where the generated document or its compiler can find it.

Run the repository example
--------------------------

The full example at ``examples/atmospheric.cpp`` demonstrates metadata,
hyperlinks, rich text, references, two formulas, a figure, headered and
headerless tables, nested lists, definitions, code, notes, fragments, and
backend-specific content. Install ``pdflatex``, then run from the repository
root:

.. code-block:: console

   cmake -S . -B build
   cmake --build build --target briefpp_demo

The seven text exports, ``report.pdf``, ``density.png``, and ``report.css``
appear in ``examples/output`` and can be committed with the example. The demo
target requires ``pdflatex`` so a successful run always includes the PDF.
Open ``report.html`` in a browser for styled output, ``report.md`` for the
MyST representation, or ``report.json`` to inspect the semantic tree. To run
the C++ test suite, use
``ctest --test-dir build --output-on-failure``. The standalone test build
downloads doctest v2.5.3; when Brief++ is embedded with ``add_subdirectory()``,
tests and examples default to off.

Next, see :doc:`model` for lists, rich text, fragments, and tables, or
:doc:`backends` for output formats and renderer settings.
