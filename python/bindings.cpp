#include <briefpp/report.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace briefpp;

namespace {

std::string render_document(const Document& doc, const std::string& format) {
    if (format == "md" || format == "markdown") return MarkdownRenderer{}.render(doc);
    if (format == "rst") return RstRenderer{}.render(doc);
    if (format == "tex" || format == "latex") return LatexRenderer{}.render(doc);
    if (format == "html") return HtmlRenderer{}.render(doc);
    if (format == "typ" || format == "typst") return TypstRenderer{}.render(doc);
    if (format == "txt" || format == "text") return PlainTextRenderer{}.render(doc);
    if (format == "json") return JsonRenderer{}.render(doc);
    throw py::value_error("unsupported report format: " + format);
}

template <typename Owner>
void bind_blocks(py::class_<Owner>& cls) {
    cls.def("section", &Owner::section, py::arg("heading"), py::return_value_policy::reference_internal)
       .def("paragraph", &Owner::paragraph, py::arg("content") = "", py::return_value_policy::reference_internal)
       .def("equation", &Owner::equation, py::arg("math"), py::arg("label") = "", py::return_value_policy::reference_internal)
       .def("figure", &Owner::figure, py::arg("path"), py::return_value_policy::reference_internal)
       .def("table", &Owner::table, py::return_value_policy::reference_internal)
       .def("code_block", &Owner::code_block, py::arg("content"), py::arg("language") = "", py::return_value_policy::reference_internal)
       .def("list", &Owner::list, py::arg("numbered") = false, py::return_value_policy::reference_internal)
       .def("definition_list", &Owner::definition_list, py::return_value_policy::reference_internal)
       .def("horizontal_rule", &Owner::horizontal_rule, py::return_value_policy::reference_internal)
       .def("page_break", &Owner::page_break, py::return_value_policy::reference_internal)
       .def("quote", &Owner::quote, py::arg("content"), py::return_value_policy::reference_internal)
       .def("admonition", &Owner::admonition, py::arg("type"), py::arg("content"), py::return_value_policy::reference_internal)
       .def("warning", &Owner::warning, py::arg("content"), py::return_value_policy::reference_internal)
       .def("note", &Owner::note, py::arg("content"), py::return_value_policy::reference_internal)
       .def("raw", &Owner::raw, py::arg("backend"), py::arg("content"), py::return_value_policy::reference_internal);
}

template <typename Owner>
void bind_inlines(py::class_<Owner>& cls) {
    cls.def("text", &Owner::text, py::return_value_policy::reference_internal)
       .def("emphasis", &Owner::emphasis, py::return_value_policy::reference_internal)
       .def("strong", &Owner::strong, py::return_value_policy::reference_internal)
       .def("code", &Owner::code, py::return_value_policy::reference_internal)
       .def("math", &Owner::math, py::return_value_policy::reference_internal)
       .def("link", &Owner::link, py::arg("text"), py::arg("url"), py::return_value_policy::reference_internal)
       .def("reference", &Owner::reference, py::return_value_policy::reference_internal)
       .def("citation", &Owner::citation, py::return_value_policy::reference_internal);
}

} // namespace

PYBIND11_MODULE(_core, m) {
    m.doc() = "Native Python bindings for the Brief++ report model and renderers";

    py::enum_<Backend>(m, "Backend")
        .value("MARKDOWN", Backend::Markdown).value("RST", Backend::Rst)
        .value("LATEX", Backend::Latex).value("HTML", Backend::Html)
        .value("TYPST", Backend::Typst).value("PLAIN_TEXT", Backend::PlainText)
        .value("JSON", Backend::Json);

    py::enum_<Kind>(m, "Kind")
        .value("SECTION", Kind::Section).value("PARAGRAPH", Kind::Paragraph)
        .value("EQUATION", Kind::Equation).value("FIGURE", Kind::Figure)
        .value("TABLE", Kind::Table).value("CODE_BLOCK", Kind::CodeBlock)
        .value("LIST", Kind::List).value("DEFINITION_LIST", Kind::DefinitionList)
        .value("QUOTE", Kind::Quote).value("ADMONITION", Kind::Admonition)
        .value("HORIZONTAL_RULE", Kind::HorizontalRule)
        .value("PAGE_BREAK", Kind::PageBreak).value("RAW", Kind::Raw);

    py::class_<InlineContent> inlines(m, "InlineContent");
    inlines.def(py::init<>()).def(py::init<std::string>());
    bind_inlines(inlines);

    py::class_<ListItem, InlineContent> list_item(m, "ListItem");
    list_item.def("list", &ListItem::list, py::arg("numbered") = false,
                  py::return_value_policy::reference_internal);

    py::class_<DefinitionItem>(m, "DefinitionItem")
        .def_property_readonly("term", [](DefinitionItem& item) -> InlineContent& { return item.term; }, py::return_value_policy::reference_internal)
        .def_property_readonly("description", [](DefinitionItem& item) -> InlineContent& { return item.description; }, py::return_value_policy::reference_internal);

    py::class_<Node> node(m, "Node");
    node.def_property_readonly("kind", [](const Node& self) { return self.kind; })
        .def("label", &Node::label, py::return_value_policy::reference_internal)
        .def("role", &Node::role, py::return_value_policy::reference_internal)
        .def("heading", &Node::heading, py::return_value_policy::reference_internal)
        .def("caption", py::overload_cast<std::string>(&Node::caption), py::return_value_policy::reference_internal)
        .def("caption", py::overload_cast<>(&Node::caption), py::return_value_policy::reference_internal)
        .def("width", &Node::width, py::return_value_policy::reference_internal)
        .def("language", &Node::language, py::return_value_policy::reference_internal)
        .def("columns", [](Node& self, std::vector<std::string> names) -> Node& { return self.columns(std::move(names)); }, py::return_value_policy::reference_internal)
        .def("columns", [](Node& self, std::vector<InlineContent> names) -> Node& { return self.columns(std::move(names)); }, py::return_value_policy::reference_internal)
        .def("column_count", &Node::column_count, py::return_value_policy::reference_internal)
        .def("row", [](Node& self, std::vector<std::string> cells) -> Node& { return self.row(std::move(cells)); }, py::return_value_policy::reference_internal)
        .def("row", [](Node& self, std::vector<InlineContent> cells) -> Node& { return self.row(std::move(cells)); }, py::return_value_policy::reference_internal)
        .def("item", py::overload_cast<std::string>(&Node::item), py::return_value_policy::reference_internal)
        .def("item", py::overload_cast<>(&Node::item), py::return_value_policy::reference_internal)
        .def("item", py::overload_cast<std::string, std::string>(&Node::item), py::return_value_policy::reference_internal)
        .def("definition_item", &Node::definition_item, py::return_value_policy::reference_internal)
        .def("cell", &Node::cell, py::return_value_policy::reference_internal);
    bind_inlines(node);
    bind_blocks(node);

    py::class_<Document> doc(m, "Document");
    doc.def(py::init<>())
       .def("title", &Document::title, py::return_value_policy::reference_internal)
       .def("subtitle", &Document::subtitle, py::return_value_policy::reference_internal)
       .def("author", &Document::author, py::return_value_policy::reference_internal)
       .def("date", &Document::date, py::return_value_policy::reference_internal)
       .def("institution", &Document::institution, py::return_value_policy::reference_internal)
       .def("abstract", &Document::abstract, py::return_value_policy::reference_internal)
       .def("keywords", &Document::keywords, py::return_value_policy::reference_internal)
       .def("append", &Document::append, py::arg("fragment"), py::return_value_policy::reference_internal)
       .def("render", &render_document, py::arg("format"))
       .def("write", &Document::write, py::arg("path"));
    bind_blocks(doc);
    m.attr("DocumentFragment") = m.attr("Document");

    py::class_<LatexRenderer> latex(m, "LatexRenderer");
    latex.def(py::init<>())
        .def("document_class", &LatexRenderer::document_class, py::return_value_policy::reference_internal)
        .def("paper", &LatexRenderer::paper, py::return_value_policy::reference_internal)
        .def("package", &LatexRenderer::package, py::return_value_policy::reference_internal)
        .def("style", &LatexRenderer::style, py::return_value_policy::reference_internal)
        .def("preamble", &LatexRenderer::preamble, py::return_value_policy::reference_internal)
        .def("role_environment", &LatexRenderer::role_environment, py::return_value_policy::reference_internal)
        .def("table_column_spec", &LatexRenderer::table_column_spec, py::return_value_policy::reference_internal)
        .def("font_size", &LatexRenderer::font_size, py::return_value_policy::reference_internal)
        .def("render", &LatexRenderer::render);

    py::class_<HtmlRenderer>(m, "HtmlRenderer")
        .def(py::init<>())
        .def("stylesheet", &HtmlRenderer::stylesheet, py::return_value_policy::reference_internal)
        .def("render", &HtmlRenderer::render);

    py::class_<TypstRenderer>(m, "TypstRenderer")
        .def(py::init<>())
        .def("preamble", &TypstRenderer::preamble, py::return_value_policy::reference_internal)
        .def("render", &TypstRenderer::render);

    py::class_<MarkdownRenderer>(m, "MarkdownRenderer").def(py::init<>()).def("render", &MarkdownRenderer::render);
    py::class_<RstRenderer>(m, "RstRenderer").def(py::init<>()).def("render", &RstRenderer::render);
    py::class_<PlainTextRenderer>(m, "PlainTextRenderer").def(py::init<>()).def("render", &PlainTextRenderer::render);
    py::class_<JsonRenderer>(m, "JsonRenderer").def(py::init<>()).def("render", &JsonRenderer::render);
}
