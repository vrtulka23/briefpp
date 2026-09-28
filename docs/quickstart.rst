Quick start
===========

Add ``include/`` to the C++ include path. No installation or separate library build is required.

.. code-block:: cmake

   add_subdirectory(external/cpp-reportlib)
   target_link_libraries(application PRIVATE reportlib::reportlib)

The example in ``examples/atmospheric.cpp`` builds a document with sections, an equation, a figure, a table, and a reference, then writes ``.md``, ``.rst``, ``.html``, ``.tex``, ``.typ``, ``.txt``, and ``.json`` files.

.. code-block:: console

   cmake -S . -B build
   cmake --build build
   ctest --test-dir build
   python3 examples/generate_density.py
   cd examples/output
   ../../build/atmospheric_report

The standalone test build fetches doctest v2.5.3. When embedded with ``add_subdirectory()``, tests and examples default to off. Set ``REPORTLIB_BUILD_TESTS=ON`` explicitly to build the doctest suite in that case.

Run ``cmake --build build --target reportlib_demo`` to save seven text formats in ``build/demo``. If ``pdflatex`` is installed, the target also creates a PDF. Edit the C++ example and rebuild the target to regenerate the files.
