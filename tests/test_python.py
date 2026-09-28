"""Exercise the installed Python binding and its bundled C++ headers."""

from pathlib import Path
from tempfile import TemporaryDirectory

import briefpp


def main():
    doc = briefpp.Document().title("Python report").author("Brief++")
    section = doc.section("Results").label("results")
    section.paragraph().text("A ").strong("result").text(" with ").reference("results")
    section.equation("E = mc^2", "energy")
    section.figure("plot.png").caption("A plot").width(0.5)
    section.table().columns(["Name", "Value"]).row(["answer", "42"])
    section.table().columns([
        briefpp.InlineContent("Metric"), briefpp.InlineContent().strong("Value")
    ]).row([briefpp.InlineContent("score"), briefpp.InlineContent("42")])
    section.list(True).item("first")
    section.definition_list().item("term", "meaning")
    section.note("Note text")
    section.code_block("print(42)", "python")
    doc.bibliography_entry("source2026", "Brief++ contributors", "Project source", "2026")
    section.paragraph().text("See ").citation("source2026")

    assert "**result**" in doc.render("md")
    assert "[[source2026]](#bib-source2026)" in doc.render("md")
    assert "\\bibitem{source2026}" in doc.render("tex")
    unresolved = briefpp.Document()
    unresolved.paragraph().citation("unknown")
    try:
        unresolved.render("html")
    except ValueError as error:
        assert "unknown" in str(error)
    else:
        raise AssertionError("unresolved citation was accepted")
    assert "<strong>result</strong>" in doc.render("html")
    assert "mathjax@4" in doc.render("html")
    assert "<script" not in briefpp.HtmlRenderer().clean_html().render(doc)
    assert "Python report" in doc.render("rst")
    assert "Python report" in doc.render("tex")
    assert "Python report" in doc.render("typ")
    assert "result" in doc.render("txt")
    assert '"schema":"briefpp/1"' in doc.render("json")
    assert "11pt" in briefpp.LatexRenderer().font_size(11).render(doc)

    # Returned nodes and inline builders must retain their parent document.
    cell = briefpp.Document().table().columns(["Value"]).cell()
    cell.strong("kept alive")
    assert briefpp.Kind.SECTION == section.kind

    with TemporaryDirectory() as directory:
        output = Path(directory) / "report.md"
        doc.write(str(output))
        assert "Python report" in output.read_text()

    assert (Path(briefpp.get_include()) / "briefpp" / "report.hpp").is_file()


if __name__ == "__main__":
    main()
