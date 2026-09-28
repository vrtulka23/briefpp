file(MAKE_DIRECTORY "${REPORTLIB_OUTPUT_DIR}")
configure_file("${REPORTLIB_FIGURE}" "${REPORTLIB_OUTPUT_DIR}/density.png" COPYONLY)
file(REMOVE
  "${REPORTLIB_OUTPUT_DIR}/report.md"
  "${REPORTLIB_OUTPUT_DIR}/report.rst"
  "${REPORTLIB_OUTPUT_DIR}/report.tex"
  "${REPORTLIB_OUTPUT_DIR}/report.html"
  "${REPORTLIB_OUTPUT_DIR}/report.typ"
  "${REPORTLIB_OUTPUT_DIR}/report.txt"
  "${REPORTLIB_OUTPUT_DIR}/report.json"
  "${REPORTLIB_OUTPUT_DIR}/report.pdf"
)

execute_process(
  COMMAND "${REPORTLIB_EXAMPLE}"
  WORKING_DIRECTORY "${REPORTLIB_OUTPUT_DIR}"
  RESULT_VARIABLE example_result
)
if(NOT example_result EQUAL 0)
  message(FATAL_ERROR "The example failed: ${example_result}")
endif()

foreach(extension md rst tex html typ txt json)
  set(output "${REPORTLIB_OUTPUT_DIR}/report.${extension}")
  if(NOT EXISTS "${output}")
    message(FATAL_ERROR "Missing ${output}")
  endif()
  file(SIZE "${output}" output_size)
  if(output_size EQUAL 0)
    message(FATAL_ERROR "Empty ${output}")
  endif()
endforeach()

file(READ "${REPORTLIB_OUTPUT_DIR}/report.md" markdown)
file(READ "${REPORTLIB_OUTPUT_DIR}/report.rst" rst)
file(READ "${REPORTLIB_OUTPUT_DIR}/report.tex" latex)
file(READ "${REPORTLIB_OUTPUT_DIR}/report.html" html)
file(READ "${REPORTLIB_OUTPUT_DIR}/report.typ" typst)
file(READ "${REPORTLIB_OUTPUT_DIR}/report.txt" plaintext)
file(READ "${REPORTLIB_OUTPUT_DIR}/report.json" json)
string(FIND "${markdown}" "```{figure} density.png" markdown_figure)
string(FIND "${rst}" ".. figure:: density.png" rst_figure)
string(FIND "${latex}" "\\includegraphics" latex_figure)
string(FIND "${html}" "<figure" html_figure)
string(FIND "${typst}" "#figure(image(" typst_figure)
string(FIND "${plaintext}" "Figure: density.png" text_figure)
string(FIND "${json}" "\"path\":\"density.png\"" json_figure)
if(markdown_figure EQUAL -1 OR rst_figure EQUAL -1 OR latex_figure EQUAL -1
   OR html_figure EQUAL -1 OR typst_figure EQUAL -1 OR text_figure EQUAL -1
   OR json_figure EQUAL -1)
  message(FATAL_ERROR "The generated files do not all contain the example figure")
endif()

if(REPORTLIB_PDFLATEX)
  # Two passes resolve LaTeX cross-references.
  foreach(pass RANGE 1 2)
    execute_process(
      COMMAND "${REPORTLIB_PDFLATEX}" -interaction=nonstopmode -halt-on-error report.tex
      WORKING_DIRECTORY "${REPORTLIB_OUTPUT_DIR}"
      RESULT_VARIABLE latex_result
      OUTPUT_QUIET
      ERROR_VARIABLE latex_error
    )
    if(NOT latex_result EQUAL 0)
      message(FATAL_ERROR "pdflatex failed on pass ${pass}: ${latex_error}")
    endif()
  endforeach()

  if(NOT EXISTS "${REPORTLIB_OUTPUT_DIR}/report.pdf")
    message(FATAL_ERROR "Missing report.pdf")
  endif()
  file(SIZE "${REPORTLIB_OUTPUT_DIR}/report.pdf" pdf_size)
  if(pdf_size LESS 1000)
    message(FATAL_ERROR "report.pdf is unexpectedly small")
  endif()
  message(STATUS "Generated seven text formats and PDF in ${REPORTLIB_OUTPUT_DIR}")
else()
  message(STATUS "Generated seven text formats in ${REPORTLIB_OUTPUT_DIR}")
endif()
