#pragma once

#include "../renderer.hpp"

namespace briefpp {

class MarkdownRenderer {
public:
    std::string render(const Document& doc) const {
        detail::validate_citations(doc);
        std::ostringstream out;
        if (!doc.metadata.title.empty()) out << "# " << detail::escape_markdown(doc.metadata.title) << "\n\n";
        if (!doc.metadata.subtitle.empty()) out << "*" << detail::escape_markdown(doc.metadata.subtitle) << "*\n\n";
        if (!doc.metadata.author.empty()) out << "**Author:** " << detail::escape_markdown(doc.metadata.author) << "\n\n";
        if (!doc.metadata.date.empty()) out << "**Date:** " << detail::escape_markdown(doc.metadata.date) << "\n\n";
        if (!doc.metadata.institution.empty()) out << "**Institution:** " << detail::escape_markdown(doc.metadata.institution) << "\n\n";
        if (!doc.metadata.abstract.empty()) out << "## Abstract\n\n" << detail::escape_markdown(doc.metadata.abstract) << "\n\n";
        for (const Node& node : doc.children) render_node(out, node, doc.metadata.title.empty() ? 1 : 2);
        if (!doc.bibliography.empty()) {
            out << "## References\n\n";
            for (const auto& entry : doc.bibliography) {
                out << "<a id=\"bib-" << entry.key << "\"></a>\n\n[" << entry.key << "] "
                    << detail::escape_markdown(detail::bibliography_text(entry));
                if (!entry.url.empty()) out << " " << "[" << detail::escape_markdown(entry.url) << "](" << entry.url << ")";
                out << "\n\n";
            }
        }
        return out.str();
    }

private:
    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        if (!node.id.empty() && node.kind != Kind::Figure && node.kind != Kind::Equation &&
            (node.kind != Kind::Table || !node.headers.empty()))
            out << "(" << node.id << ")=\n";
        switch (node.kind) {
        case Kind::Section:
            out << std::string(static_cast<std::size_t>(std::min(depth, 6)), '#') << " " << detail::inline_text(node.heading_content, Backend::Markdown) << "\n\n";
            for (const Node& child : node.children) render_node(out, child, depth + 1);
            break;
        case Kind::Paragraph: out << detail::inline_text(node, Backend::Markdown) << "\n\n"; break;
        case Kind::Equation:
            out << "```{math}";
            if (!node.id.empty()) out << "\n:label: " << node.id;
            out << "\n" << node.value << "\n```\n\n";
            break;
        case Kind::Figure:
            out << "```{figure} " << node.value << "\n";
            if (!node.id.empty()) out << ":name: " << node.id << "\n";
            if (node.width_fraction != 1.0) out << ":width: " << detail::decimal(node.width_fraction * 100) << "%\n";
            out << "\n" << detail::inline_text(node.caption_content, Backend::Markdown) << "\n```\n\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            if (node.headers.empty()) {
                out << "```{list-table}";
                if (!node.caption_content.empty())
                    out << ' ' << detail::inline_text(node.caption_content, Backend::Markdown);
                out << "\n:header-rows: 0\n";
                if (!node.id.empty()) out << ":name: " << node.id << '\n';
                out << '\n';
                for (const auto& row : node.rows) {
                    for (std::size_t i = 0; i < row.size(); ++i)
                        out << (i == 0 ? "* - " : "  - ")
                            << detail::inline_text(row[i], Backend::Markdown) << '\n';
                }
                out << "```\n\n";
                break;
            }
            out << "|";
            for (const auto& cell : node.headers) out << " " << detail::inline_text(cell, Backend::Markdown) << " |";
            out << "\n|";
            for (std::size_t i = 0; i < node.headers.size(); ++i) out << " --- |";
            out << "\n";
            for (const auto& row : node.rows) {
                out << "|";
                for (const auto& cell : row) out << " " << detail::inline_text(cell, Backend::Markdown) << " |";
                out << "\n";
            }
            if (!node.caption_content.empty()) out << "\n*" << detail::inline_text(node.caption_content, Backend::Markdown) << "*\n";
            out << "\n";
            break;
        case Kind::CodeBlock:
            out << "```" << node.language_name << "\n" << node.value << "\n```\n\n"; break;
        case Kind::List: render_list(out, node, 0); out << "\n"; break;
        case Kind::DefinitionList:
            for (const auto& item : node.definitions)
                out << detail::inline_text(item.term, Backend::Markdown) << "\n: "
                    << detail::inline_text(item.description, Backend::Markdown) << "\n\n";
            break;
        case Kind::Quote: out << "> " << detail::replace_all(detail::escape_markdown(node.value), "\n", "\n> ") << "\n\n"; break;
        case Kind::Admonition:
            out << "```{" << node.admonition_kind << "}\n" << detail::escape_markdown(node.value) << "\n```\n\n"; break;
        case Kind::HorizontalRule: out << "---\n\n"; break;
        case Kind::PageBreak: out << "<div class=\"page-break\"></div>\n\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Markdown) out << node.value << "\n\n"; break;
        }
    }
    static void render_list(std::ostringstream& out, const Node& node, int indent) {
        std::size_t index = 0;
        for (const auto& item : node.items) {
            out << std::string(static_cast<std::size_t>(indent), ' ')
                << (node.ordered ? std::to_string(++index) + ". " : "- ")
                << detail::inline_text(item, Backend::Markdown) << "\n";
            for (const auto& child : item.children) if (child.kind == Kind::List)
                render_list(out, child, indent + 2);
        }
    }
};

} // namespace briefpp
