if(BRIEFPP_REQUIRE_PDF AND NOT BRIEFPP_PDFLATEX)
  message(FATAL_ERROR "briefpp_demo needs pdflatex to produce report.pdf")
endif()

file(MAKE_DIRECTORY "${BRIEFPP_OUTPUT_DIR}")
configure_file("${BRIEFPP_FIGURE}" "${BRIEFPP_OUTPUT_DIR}/density.png" COPYONLY)
configure_file("${BRIEFPP_STYLESHEET}" "${BRIEFPP_OUTPUT_DIR}/report.css" COPYONLY)
file(REMOVE
  "${BRIEFPP_OUTPUT_DIR}/report.md"
  "${BRIEFPP_OUTPUT_DIR}/report.rst"
  "${BRIEFPP_OUTPUT_DIR}/report.tex"
  "${BRIEFPP_OUTPUT_DIR}/report.html"
  "${BRIEFPP_OUTPUT_DIR}/report.typ"
  "${BRIEFPP_OUTPUT_DIR}/report.txt"
  "${BRIEFPP_OUTPUT_DIR}/report.json"
  "${BRIEFPP_OUTPUT_DIR}/report.pdf"
)

execute_process(
  COMMAND "${BRIEFPP_EXAMPLE}"
  WORKING_DIRECTORY "${BRIEFPP_OUTPUT_DIR}"
  RESULT_VARIABLE example_result
)
if(NOT example_result EQUAL 0)
  message(FATAL_ERROR "The example failed: ${example_result}")
endif()

foreach(extension md rst tex html typ txt json)
  set(output "${BRIEFPP_OUTPUT_DIR}/report.${extension}")
  if(NOT EXISTS "${output}")
    message(FATAL_ERROR "Missing ${output}")
  endif()
  file(SIZE "${output}" output_size)
  if(output_size EQUAL 0)
    message(FATAL_ERROR "Empty ${output}")
  endif()
endforeach()

file(READ "${BRIEFPP_OUTPUT_DIR}/report.md" markdown)
file(READ "${BRIEFPP_OUTPUT_DIR}/report.rst" rst)
file(READ "${BRIEFPP_OUTPUT_DIR}/report.tex" latex)
file(READ "${BRIEFPP_OUTPUT_DIR}/report.html" html)
file(READ "${BRIEFPP_OUTPUT_DIR}/report.typ" typst)
file(READ "${BRIEFPP_OUTPUT_DIR}/report.txt" plaintext)
file(READ "${BRIEFPP_OUTPUT_DIR}/report.json" json)
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

string(FIND "${markdown}" "[project source](https://github.com/vrtulka23/briefpp)" markdown_link)
string(FIND "${markdown}" "```{math}" markdown_math)
string(FIND "${markdown}" "| Parameter | Value | Unit |" markdown_table)
string(FIND "${markdown}" "1. Set the starting conditions" markdown_list)
string(FIND "${html}" "href=\"report.css\"" html_stylesheet)
string(FIND "${html}" "mathjax@4/tex-chtml.js" html_mathjax)
string(FIND "${html}" "\\[\\frac{dP}{dz} = -\\rho g\\]" html_equation)
string(FIND "${html}" "class=\"backend-note\"" html_raw)
string(FIND "${latex}" "\\begin{equation}" latex_math)
string(FIND "${json}" "\"type\":\"definition_list\"" json_definitions)
string(FIND "${markdown}" "#bib-example2026" markdown_citation)
string(FIND "${rst}" ".. [example2026]" rst_bibliography)
string(FIND "${latex}" "\\bibitem{example2026}" latex_bibliography)
string(FIND "${html}" "id=\"bib-example2026\"" html_bibliography)
string(FIND "${typst}" "<bib-example2026>" typst_bibliography)
string(FIND "${plaintext}" "[example2026] Brief++ contributors" text_bibliography)
string(FIND "${json}" "\"key\":\"example2026\"" json_bibliography)
if(markdown_link EQUAL -1 OR markdown_math EQUAL -1 OR markdown_table EQUAL -1
   OR markdown_list EQUAL -1 OR html_stylesheet EQUAL -1 OR html_mathjax EQUAL -1
   OR html_equation EQUAL -1 OR html_raw EQUAL -1
   OR latex_math EQUAL -1 OR json_definitions EQUAL -1
   OR markdown_citation EQUAL -1 OR rst_bibliography EQUAL -1 OR latex_bibliography EQUAL -1
   OR html_bibliography EQUAL -1 OR typst_bibliography EQUAL -1 OR text_bibliography EQUAL -1
   OR json_bibliography EQUAL -1)
  message(FATAL_ERROR "The example is missing expected report features")
endif()

if(BRIEFPP_PDFLATEX)
  # Two passes resolve LaTeX cross-references.
  foreach(pass RANGE 1 2)
    execute_process(
      COMMAND "${BRIEFPP_PDFLATEX}" -interaction=nonstopmode -halt-on-error report.tex
      WORKING_DIRECTORY "${BRIEFPP_OUTPUT_DIR}"
      RESULT_VARIABLE latex_result
      OUTPUT_QUIET
      ERROR_VARIABLE latex_error
    )
    if(NOT latex_result EQUAL 0)
      message(FATAL_ERROR "pdflatex failed on pass ${pass}: ${latex_error}")
    endif()
  endforeach()

  if(NOT EXISTS "${BRIEFPP_OUTPUT_DIR}/report.pdf")
    message(FATAL_ERROR "Missing report.pdf")
  endif()
  file(SIZE "${BRIEFPP_OUTPUT_DIR}/report.pdf" pdf_size)
  if(pdf_size LESS 1000)
    message(FATAL_ERROR "report.pdf is unexpectedly small")
  endif()
  file(REMOVE
    "${BRIEFPP_OUTPUT_DIR}/report.aux"
    "${BRIEFPP_OUTPUT_DIR}/report.log"
    "${BRIEFPP_OUTPUT_DIR}/report.out"
  )
  message(STATUS "Generated seven text formats and PDF in ${BRIEFPP_OUTPUT_DIR}")
else()
  message(STATUS "Generated seven text formats in ${BRIEFPP_OUTPUT_DIR}")
endif()
