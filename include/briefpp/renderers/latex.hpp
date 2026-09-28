#pragma once

#include "../renderer.hpp"
#include <map>

namespace briefpp {

class LatexRenderer {
public:
    LatexRenderer& document_class(std::string value) { class_name_ = std::move(value); return *this; }
    LatexRenderer& paper(std::string value) { paper_ = std::move(value); return *this; }
    LatexRenderer& package(std::string value) { packages_.push_back(std::move(value)); return *this; }
    LatexRenderer& style(std::string value) { return package(std::move(value)); }
    LatexRenderer& preamble(std::string value) { preamble_ += std::move(value) + "\n"; return *this; }
    LatexRenderer& role_environment(std::string role, std::string environment) {
        if (role.empty() || environment.empty())
            throw std::invalid_argument("role and LaTeX environment must be nonempty");
        role_environments_[std::move(role)] = std::move(environment);
        return *this;
    }
    LatexRenderer& table_column_spec(std::string role, std::string specification) {
        if (role.empty() || specification.empty())
            throw std::invalid_argument("role and LaTeX table column specification must be nonempty");
        table_column_specs_[std::move(role)] = std::move(specification);
        return *this;
    }
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
        for (const auto& package_name : packages_) out << "\\usepackage{" << package_name << "}\n";
        out << preamble_;
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
    std::vector<std::string> packages_;
    std::string preamble_;
    std::map<std::string, std::string> role_environments_;
    std::map<std::string, std::string> table_column_specs_;

    std::string table_columns(const Node& node) const {
        for (const auto& role : node.roles) {
            const auto found = table_column_specs_.find(role);
            if (found != table_column_specs_.end()) return found->second;
        }
        return std::string(node.table_column_count, 'l');
    }

    void render_node(std::ostringstream& out, const Node& node, int depth) const {
        if (node.kind == Kind::Raw && node.raw_backend != Backend::Latex) return;
        std::vector<std::string> environments;
        for (const auto& role : node.roles) {
            const auto found = role_environments_.find(role);
            if (found == role_environments_.end()) continue;
            out << "\\begin{" << found->second << "}\n";
            environments.push_back(found->second);
        }
        switch (node.kind) {
        case Kind::Section: {
            constexpr const char* levels[] = {"section", "subsection", "subsubsection", "paragraph"};
            out << "\\" << levels[std::min(depth, 3)] << "{" << detail::inline_text(node.heading_content, Backend::Latex) << "}";
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
            if (!node.caption_content.empty()) out << "\\caption{" << detail::inline_text(node.caption_content, Backend::Latex) << "}\n";
            if (!node.id.empty()) out << "\\label{" << detail::escape_latex(node.id) << "}\n";
            out << "\\end{figure}\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            out << "\\begin{longtable}{" << table_columns(node) << "}\n";
            if (!node.caption_content.empty() || !node.id.empty()) {
                if (!node.caption_content.empty()) out << "\\caption{" << detail::inline_text(node.caption_content, Backend::Latex) << "}";
                if (!node.id.empty()) out << "\\label{" << detail::escape_latex(node.id) << "}";
                out << "\\\\\n";
            }
            if (!node.headers.empty()) {
                write_row(out, node.headers);
                out << "\\hline\n";
            }
            for (const auto& row : node.rows) write_row(out, row);
            out << "\\end{longtable}\n";
            break;
        case Kind::CodeBlock:
            out << "\\begin{verbatim}\n" << node.value << "\n\\end{verbatim}\n"; break;
        case Kind::List:
            out << (node.ordered ? "\\begin{enumerate}\n" : "\\begin{itemize}\n");
            for (const auto& item : node.items) {
                out << "\\item " << detail::inline_text(item, Backend::Latex) << "\n";
                for (const auto& child : item.children) if (child.kind == Kind::List) render_node(out, child, depth);
            }
            out << (node.ordered ? "\\end{enumerate}\n" : "\\end{itemize}\n");
            break;
        case Kind::DefinitionList:
            out << "\\begin{description}\n";
            for (const auto& item : node.definitions)
                out << "\\item[" << detail::inline_text(item.term, Backend::Latex) << "] "
                    << detail::inline_text(item.description, Backend::Latex) << "\n";
            out << "\\end{description}\n";
            break;
        case Kind::Quote: out << "\\begin{quote}\n" << detail::escape_latex(node.value) << "\n\\end{quote}\n"; break;
        case Kind::Admonition:
            out << "\\begin{quote}\\textbf{" << detail::escape_latex(node.admonition_kind) << ":} "
                << detail::escape_latex(node.value) << "\\end{quote}\n"; break;
        case Kind::HorizontalRule: out << "\\par\\noindent\\rule{\\linewidth}{0.4pt}\\par\n"; break;
        case Kind::PageBreak: out << "\\newpage\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Latex) out << node.value << "\n"; break;
        }
        for (auto it = environments.rbegin(); it != environments.rend(); ++it)
            out << "\\end{" << *it << "}\n";
    }
    static void write_row(std::ostringstream& out, const std::vector<InlineContent>& row) {
        for (std::size_t i = 0; i < row.size(); ++i) {
            if (i) out << " & ";
            out << detail::inline_text(row[i], Backend::Latex);
        }
        out << " \\\\\n";
    }
};

} // namespace briefpp
