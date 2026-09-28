#pragma once

#include "../renderer.hpp"

namespace briefpp {

class PlainTextRenderer {
public:
    std::string render(const Document& doc) const {
        detail::validate_citations(doc);
        std::ostringstream out;
        if (!doc.metadata.title.empty()) out << doc.metadata.title << "\n\n";
        if (!doc.metadata.subtitle.empty()) out << doc.metadata.subtitle << "\n\n";
        if (!doc.metadata.author.empty()) out << "Author: " << doc.metadata.author << '\n';
        if (!doc.metadata.date.empty()) out << "Date: " << doc.metadata.date << '\n';
        if (!doc.metadata.institution.empty()) out << "Institution: " << doc.metadata.institution << '\n';
        if (!doc.metadata.abstract.empty()) out << "\nAbstract\n" << doc.metadata.abstract << "\n\n";
        for (const auto& node : doc.children) render_node(out, node, 0);
        if (!doc.bibliography.empty()) {
            out << "References\n----------\n\n";
            for (const auto& entry : doc.bibliography) {
                out << '[' << entry.key << "] " << detail::bibliography_text(entry);
                if (!entry.url.empty()) out << ' ' << entry.url;
                out << ( &entry == &doc.bibliography.back() ? "\n" : "\n\n" );
            }
        }
        return out.str();
    }
private:
    static std::string inline_text(const InlineContent& content) {
        std::string out;
        for (const auto& part : content.content) {
            switch (part.kind) {
            case InlineKind::Link: out += part.value + " (" + part.target + ")"; break;
            case InlineKind::Reference: out += "[" + part.target + "]"; break;
            case InlineKind::Citation: out += "[" + part.target + "]"; break;
            default: out += part.value;
            }
        }
        return out;
    }
    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        switch (node.kind) {
        case Kind::Section:
            out << inline_text(node.heading_content) << '\n'
                << std::string(inline_text(node.heading_content).size(), '-') << "\n\n";
            for (const auto& child : node.children) render_node(out, child, depth + 1);
            break;
        case Kind::Paragraph: out << inline_text(node.inlines) << "\n\n"; break;
        case Kind::Equation: out << node.value << "\n\n"; break;
        case Kind::Figure:
            out << "Figure: " << node.value;
            if (!node.caption_content.empty()) out << " — " << inline_text(node.caption_content);
            out << "\n\n"; break;
        case Kind::Table:
            detail::require_table(node);
            if (!node.caption_content.empty()) out << inline_text(node.caption_content) << '\n';
            if (!node.headers.empty()) write_row(out, node.headers);
            for (const auto& row : node.rows) write_row(out, row);
            out << '\n'; break;
        case Kind::CodeBlock: out << node.value << "\n\n"; break;
        case Kind::List: render_list(out, node, 0); out << '\n'; break;
        case Kind::DefinitionList:
            for (const auto& item : node.definitions)
                out << inline_text(item.term) << ": " << inline_text(item.description) << '\n';
            out << '\n'; break;
        case Kind::Quote: out << node.value << "\n\n"; break;
        case Kind::Admonition: out << node.admonition_kind << ": " << node.value << "\n\n"; break;
        case Kind::HorizontalRule: out << "--------------------\n\n"; break;
        case Kind::PageBreak: break;
        case Kind::Raw: if (node.raw_backend == Backend::PlainText) out << node.value << "\n\n"; break;
        }
    }
    static void write_row(std::ostringstream& out, const std::vector<InlineContent>& row) {
        for (std::size_t i = 0; i < row.size(); ++i) {
            if (i) out << " | ";
            out << inline_text(row[i]);
        }
        out << '\n';
    }
    static void render_list(std::ostringstream& out, const Node& node, int indent) {
        std::size_t index = 0;
        for (const auto& item : node.items) {
            out << std::string(static_cast<std::size_t>(indent), ' ')
                << (node.ordered ? std::to_string(++index) + ". " : "- ")
                << inline_text(item) << '\n';
            for (const auto& child : item.children) if (child.kind == Kind::List)
                render_list(out, child, indent + 2);
        }
    }
};

} // namespace briefpp
