#pragma once

#include "../renderer.hpp"

namespace report {

class MarkdownRenderer {
public:
    std::string render(const Document& doc) const {
        std::ostringstream out;
        if (!doc.metadata.title.empty()) out << "# " << detail::escape_markdown(doc.metadata.title) << "\n\n";
        if (!doc.metadata.subtitle.empty()) out << "*" << detail::escape_markdown(doc.metadata.subtitle) << "*\n\n";
        if (!doc.metadata.author.empty()) out << "**Author:** " << detail::escape_markdown(doc.metadata.author) << "\n\n";
        if (!doc.metadata.date.empty()) out << "**Date:** " << detail::escape_markdown(doc.metadata.date) << "\n\n";
        if (!doc.metadata.institution.empty()) out << "**Institution:** " << detail::escape_markdown(doc.metadata.institution) << "\n\n";
        if (!doc.metadata.abstract.empty()) out << "## Abstract\n\n" << detail::escape_markdown(doc.metadata.abstract) << "\n\n";
        for (const Node& node : doc.children) render_node(out, node, doc.metadata.title.empty() ? 1 : 2);
        return out.str();
    }

private:
    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        if (!node.id.empty() && node.kind != Kind::Figure && node.kind != Kind::Equation)
            out << "(" << node.id << ")=\n";
        switch (node.kind) {
        case Kind::Section:
            out << std::string(static_cast<std::size_t>(std::min(depth, 6)), '#') << " " << detail::escape_markdown(node.value) << "\n\n";
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
            out << "\n" << detail::escape_markdown(node.caption_text) << "\n```\n\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            out << "|";
            for (const auto& cell : node.headers) out << " " << detail::escape_markdown(cell) << " |";
            out << "\n|";
            for (std::size_t i = 0; i < node.headers.size(); ++i) out << " --- |";
            out << "\n";
            for (const auto& row : node.rows) {
                out << "|";
                for (const auto& cell : row) out << " " << detail::escape_markdown(cell) << " |";
                out << "\n";
            }
            if (!node.caption_text.empty()) out << "\n*" << detail::escape_markdown(node.caption_text) << "*\n";
            out << "\n";
            break;
        case Kind::CodeBlock:
            out << "```" << node.language_name << "\n" << node.value << "\n```\n\n"; break;
        case Kind::List:
            for (std::size_t i = 0; i < node.items.size(); ++i)
                out << (node.ordered ? std::to_string(i + 1) + ". " : "- ") << detail::escape_markdown(node.items[i]) << "\n";
            out << "\n"; break;
        case Kind::Quote: out << "> " << detail::replace_all(detail::escape_markdown(node.value), "\n", "\n> ") << "\n\n"; break;
        case Kind::Admonition:
            out << "```{" << node.admonition_kind << "}\n" << detail::escape_markdown(node.value) << "\n```\n\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Markdown) out << node.value << "\n\n"; break;
        }
    }
};

} // namespace report
