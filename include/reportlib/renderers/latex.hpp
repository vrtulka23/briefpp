#pragma once

#include "../renderer.hpp"

namespace report {

class LatexRenderer {
public:
    LatexRenderer& document_class(std::string value) { class_name_ = std::move(value); return *this; }
    LatexRenderer& paper(std::string value) { paper_ = std::move(value); return *this; }
    LatexRenderer& font_size(int value) {
        if (value != 10 && value != 11 && value != 12)
            throw std::invalid_argument("LaTeX font size must be 10, 11, or 12");
        font_size_ = value;
        return *this;
    }
    std::string render(const Document& doc) const {
        std::ostringstream out;
        out << "\\documentclass[" << font_size_ << "pt," << paper_ << "paper]{" << class_name_ << "}\n"
            << "\\usepackage[T1]{fontenc}\n\\usepackage{amsmath}\n\\usepackage{graphicx}\n"
            << "\\usepackage{hyperref}\n\\usepackage{longtable}\n\\usepackage{geometry}\n"
            << "\\geometry{margin=25mm}\n";
        if (!doc.metadata.title.empty()) out << "\\title{" << detail::escape_latex(doc.metadata.title) << "}\n";
        if (!doc.metadata.author.empty()) out << "\\author{" << detail::escape_latex(doc.metadata.author) << "}\n";
        out << "\\date{" << detail::escape_latex(doc.metadata.date) << "}\n\\begin{document}\n";
        if (!doc.metadata.title.empty()) out << "\\maketitle\n";
        if (!doc.metadata.subtitle.empty()) out << "\\begin{center}\\large " << detail::escape_latex(doc.metadata.subtitle) << "\\end{center}\n";
        if (!doc.metadata.institution.empty()) out << "\\begin{center}" << detail::escape_latex(doc.metadata.institution) << "\\end{center}\n";
        if (!doc.metadata.abstract.empty()) out << "\\begin{abstract}\n" << detail::escape_latex(doc.metadata.abstract) << "\n\\end{abstract}\n";
        for (const Node& node : doc.children) render_node(out, node, 0);
        out << "\\end{document}\n";
        return out.str();
    }

private:
    std::string class_name_ = "article";
    std::string paper_ = "a4";
    int font_size_ = 11;

    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        switch (node.kind) {
        case Kind::Section: {
            constexpr const char* levels[] = {"section", "subsection", "subsubsection", "paragraph"};
            out << "\\" << levels[std::min(depth, 3)] << "{" << detail::escape_latex(node.value) << "}";
            if (!node.id.empty()) out << "\\label{" << detail::escape_latex(node.id) << "}";
            out << "\n";
            for (const Node& child : node.children) render_node(out, child, depth + 1);
            break;
        }
        case Kind::Paragraph: out << detail::inline_text(node, Backend::Latex) << "\n\n"; break;
        case Kind::Equation:
            out << "\\begin{equation" << (node.id.empty() ? "*" : "") << "}\n" << node.value << "\n";
            if (!node.id.empty()) out << "\\label{" << detail::escape_latex(node.id) << "}\n";
            out << "\\end{equation" << (node.id.empty() ? "*" : "") << "}\n";
            break;
        case Kind::Figure:
            out << "\\begin{figure}[htbp]\n\\centering\n\\includegraphics[width=" << detail::decimal(node.width_fraction)
                << "\\linewidth]{" << detail::escape_latex(node.value) << "}\n";
            if (!node.caption_text.empty()) out << "\\caption{" << detail::escape_latex(node.caption_text) << "}\n";
            if (!node.id.empty()) out << "\\label{" << detail::escape_latex(node.id) << "}\n";
            out << "\\end{figure}\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            out << "\\begin{longtable}{" << std::string(node.headers.size(), 'l') << "}\n";
            if (!node.caption_text.empty() || !node.id.empty()) {
                if (!node.caption_text.empty()) out << "\\caption{" << detail::escape_latex(node.caption_text) << "}";
                if (!node.id.empty()) out << "\\label{" << detail::escape_latex(node.id) << "}";
                out << "\\\\\n";
            }
            write_row(out, node.headers);
            out << "\\hline\n";
            for (const auto& row : node.rows) write_row(out, row);
            out << "\\end{longtable}\n";
            break;
        case Kind::CodeBlock:
            out << "\\begin{verbatim}\n" << node.value << "\n\\end{verbatim}\n"; break;
        case Kind::List:
            out << (node.ordered ? "\\begin{enumerate}\n" : "\\begin{itemize}\n");
            for (const auto& item : node.items) out << "\\item " << detail::escape_latex(item) << "\n";
            out << (node.ordered ? "\\end{enumerate}\n" : "\\end{itemize}\n");
            break;
        case Kind::Quote: out << "\\begin{quote}\n" << detail::escape_latex(node.value) << "\n\\end{quote}\n"; break;
        case Kind::Admonition:
            out << "\\begin{quote}\\textbf{" << detail::escape_latex(node.admonition_kind) << ":} "
                << detail::escape_latex(node.value) << "\\end{quote}\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Latex) out << node.value << "\n"; break;
        }
    }
    static void write_row(std::ostringstream& out, const std::vector<std::string>& row) {
        for (std::size_t i = 0; i < row.size(); ++i) {
            if (i) out << " & ";
            out << detail::escape_latex(row[i]);
        }
        out << " \\\\\n";
    }
};

} // namespace report
