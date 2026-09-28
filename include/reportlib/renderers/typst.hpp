#pragma once

#include "../renderer.hpp"

namespace report {
namespace detail {

inline std::string escape_typst(const std::string& value) {
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        const bool line_start = i == 0 || value[i - 1] == '\n';
        if (c == '\\' || c == '#' || c == '$' || c == '[' || c == ']' ||
            c == '*' || c == '_' || c == '@' || c == '<' || c == '>' || c == '`' ||
            (line_start && (c == '=' || c == '-' || c == '+' || c == '/'))) out += '\\';
        out += c;
    }
    return out;
}

inline std::string typst_string(const std::string& value) {
    std::string out = "\"";
    for (char c : value) {
        switch (c) {
        case '\\': out += "\\\\"; break;
        case '"': out += "\\\""; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        default: out += c;
        }
    }
    return out + '"';
}

inline std::string typst_inline(const InlineContent& content) {
    std::string out;
    for (const auto& part : content.content) {
        switch (part.kind) {
        case InlineKind::Text: out += escape_typst(part.value); break;
        case InlineKind::Emphasis: out += "_" + escape_typst(part.value) + "_"; break;
        case InlineKind::Strong: out += "*" + escape_typst(part.value) + "*"; break;
        case InlineKind::Code: out += "#raw(" + typst_string(part.value) + ")"; break;
        case InlineKind::Math:
            out += part.value.find('\\') == std::string::npos ? "$" + part.value + "$" :
                   "#raw(" + typst_string(part.value) + ")";
            break;
        case InlineKind::Link:
            out += "#link(" + typst_string(part.target) + ")[" + escape_typst(part.value) + "]"; break;
        case InlineKind::Reference: out += "@" + part.target; break;
        case InlineKind::Citation: out += "[" + escape_typst(part.target) + "]"; break;
        }
    }
    return out;
}

} // namespace detail

class TypstRenderer {
public:
    TypstRenderer& preamble(std::string content) { preamble_ += std::move(content) + "\n"; return *this; }

    std::string render(const Document& doc) const {
        std::ostringstream out;
        out << preamble_;
        if (!doc.metadata.title.empty()) out << "= " << detail::escape_typst(doc.metadata.title) << "\n\n";
        if (!doc.metadata.subtitle.empty()) out << "_" << detail::escape_typst(doc.metadata.subtitle) << "_\n\n";
        if (!doc.metadata.author.empty()) out << detail::escape_typst(doc.metadata.author) << "\n\n";
        if (!doc.metadata.date.empty()) out << detail::escape_typst(doc.metadata.date) << "\n\n";
        if (!doc.metadata.institution.empty()) out << detail::escape_typst(doc.metadata.institution) << "\n\n";
        if (!doc.metadata.abstract.empty()) out << "== Abstract\n\n" << detail::escape_typst(doc.metadata.abstract) << "\n\n";
        for (const auto& node : doc.children) render_node(out, node, doc.metadata.title.empty() ? 1 : 2);
        return out.str();
    }

private:
    std::string preamble_;

    static void label(std::ostringstream& out, const Node& node) {
        if (!node.id.empty()) out << '<' << node.id << '>';
    }
    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        switch (node.kind) {
        case Kind::Section:
            out << std::string(static_cast<std::size_t>(depth), '=') << ' '
                << detail::typst_inline(node.heading_content);
            label(out, node); out << "\n\n";
            for (const auto& child : node.children) render_node(out, child, depth + 1);
            break;
        case Kind::Paragraph:
            out << detail::typst_inline(node.inlines); label(out, node); out << "\n\n"; break;
        case Kind::Equation:
            if (node.value.find('\\') == std::string::npos) out << "$ " << node.value << " $";
            else out << "#raw(" << detail::typst_string(node.value) << ", block: true)";
            label(out, node); out << "\n\n"; break;
        case Kind::Figure:
            out << "#figure(image(" << detail::typst_string(node.value);
            if (node.width_fraction != 1.0) out << ", width: " << detail::decimal(node.width_fraction * 100) << '%';
            out << ')';
            if (!node.caption_content.empty()) out << ", caption: [" << detail::typst_inline(node.caption_content) << ']';
            out << ')'; label(out, node); out << "\n\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            if (!node.caption_content.empty() || !node.id.empty()) out << "#figure(";
            out << (node.caption_content.empty() && node.id.empty() ? "#table(columns: " : "table(columns: ")
                << node.headers.size() << ", table.header(";
            write_cells(out, node.headers);
            out << ')';
            for (const auto& row : node.rows) { out << ", "; write_cells(out, row); }
            out << ')';
            if (!node.caption_content.empty() || !node.id.empty()) {
                if (!node.caption_content.empty()) out << ", caption: [" << detail::typst_inline(node.caption_content) << ']';
                out << ')';
            }
            label(out, node); out << "\n\n";
            break;
        case Kind::CodeBlock:
            out << "#raw(" << detail::typst_string(node.value) << ", block: true";
            if (!node.language_name.empty()) out << ", lang: " << detail::typst_string(node.language_name);
            out << ')'; label(out, node); out << "\n\n"; break;
        case Kind::List: render_list(out, node, 0); out << '\n'; break;
        case Kind::DefinitionList:
            for (const auto& item : node.definitions)
                out << "/ " << detail::typst_inline(item.term) << ": "
                    << detail::typst_inline(item.description) << '\n';
            out << '\n'; break;
        case Kind::Quote:
            out << "#quote(block: true)[" << detail::escape_typst(node.value) << "]\n\n"; break;
        case Kind::Admonition:
            out << '*' << detail::escape_typst(node.admonition_kind) << ":* "
                << detail::escape_typst(node.value) << "\n\n"; break;
        case Kind::HorizontalRule: out << "#line(length: 100%)\n\n"; break;
        case Kind::PageBreak: out << "#pagebreak()\n\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Typst) out << node.value << "\n\n"; break;
        }
    }
    static void write_cells(std::ostringstream& out, const std::vector<InlineContent>& cells) {
        for (std::size_t i = 0; i < cells.size(); ++i) {
            if (i) out << ", ";
            out << '[' << detail::typst_inline(cells[i]) << ']';
        }
    }
    static void render_list(std::ostringstream& out, const Node& node, int indent) {
        for (const auto& item : node.items) {
            out << std::string(static_cast<std::size_t>(indent), ' ')
                << (node.ordered ? "+ " : "- ") << detail::typst_inline(item) << '\n';
            for (const auto& child : item.children) if (child.kind == Kind::List)
                render_list(out, child, indent + 2);
        }
    }
};

} // namespace report
