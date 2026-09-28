#pragma once

#include "../renderer.hpp"

namespace report {

class RstRenderer {
public:
    std::string render(const Document& doc) const {
        std::ostringstream out;
        if (!doc.metadata.title.empty()) heading(out, doc.metadata.title, '=');
        if (!doc.metadata.subtitle.empty()) heading(out, doc.metadata.subtitle, '-');
        if (!doc.metadata.author.empty()) out << ":Author: " << detail::escape_rst(doc.metadata.author) << "\n";
        if (!doc.metadata.date.empty()) out << ":Date: " << detail::escape_rst(doc.metadata.date) << "\n";
        if (!doc.metadata.institution.empty()) out << ":Institution: " << detail::escape_rst(doc.metadata.institution) << "\n";
        if (!doc.metadata.abstract.empty()) out << "\n.. rubric:: Abstract\n\n" << detail::escape_rst(doc.metadata.abstract) << "\n\n";
        if (!doc.metadata.author.empty() || !doc.metadata.date.empty() || !doc.metadata.institution.empty()) out << "\n";
        for (const Node& node : doc.children) render_node(out, node, 0);
        return out.str();
    }

private:
    static void heading(std::ostringstream& out, const std::string& value, char underline) {
        out << detail::escape_rst(value) << "\n" << std::string(value.size(), underline) << "\n\n";
    }
    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        if (!node.id.empty() && node.kind != Kind::Equation && node.kind != Kind::Figure)
            out << ".. _" << node.id << ":\n\n";
        switch (node.kind) {
        case Kind::Section: {
            constexpr char marks[] = {'-', '~', '^', '"', '+'};
            heading(out, node.value, marks[std::min(depth, 4)]);
            for (const Node& child : node.children) render_node(out, child, depth + 1);
            break;
        }
        case Kind::Paragraph: out << detail::inline_text(node, Backend::Rst) << "\n\n"; break;
        case Kind::Equation:
            out << ".. math::\n";
            if (!node.id.empty()) out << "   :label: " << node.id << "\n";
            out << "\n   " << detail::replace_all(node.value, "\n", "\n   ") << "\n\n";
            break;
        case Kind::Figure:
            if (!node.id.empty()) out << ".. _" << node.id << ":\n\n";
            out << ".. figure:: " << node.value << "\n";
            if (node.width_fraction != 1.0) out << "   :width: " << detail::decimal(node.width_fraction * 100) << "%\n";
            out << "\n";
            if (!node.caption_text.empty()) out << "   " << detail::escape_rst(node.caption_text) << "\n";
            out << "\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            out << ".. list-table:: " << detail::escape_rst(node.caption_text) << "\n   :header-rows: 1\n\n";
            write_row(out, node.headers);
            for (const auto& row : node.rows) write_row(out, row);
            out << "\n";
            break;
        case Kind::CodeBlock:
            out << ".. code-block:: " << node.language_name << "\n\n   "
                << detail::replace_all(node.value, "\n", "\n   ") << "\n\n"; break;
        case Kind::List:
            for (std::size_t i = 0; i < node.items.size(); ++i)
                out << (node.ordered ? std::to_string(i + 1) + ". " : "- ") << detail::escape_rst(node.items[i]) << "\n";
            out << "\n"; break;
        case Kind::Quote:
            out << "   " << detail::replace_all(detail::escape_rst(node.value), "\n", "\n   ") << "\n\n"; break;
        case Kind::Admonition:
            out << ".. " << node.admonition_kind << "::\n\n   "
                << detail::replace_all(detail::escape_rst(node.value), "\n", "\n   ") << "\n\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Rst) out << node.value << "\n\n"; break;
        }
    }
    static void write_row(std::ostringstream& out, const std::vector<std::string>& row) {
        for (std::size_t i = 0; i < row.size(); ++i)
            out << (i == 0 ? "   * - " : "     - ") << detail::escape_rst(row[i]) << "\n";
    }
};

} // namespace report
