#pragma once

#include "../renderer.hpp"

namespace report {
namespace detail {

inline std::string json_quote(const std::string& value) {
    static constexpr char hex[] = "0123456789abcdef";
    std::string out = "\"";
    for (unsigned char c : value) {
        switch (c) {
        case '"': out += "\\\""; break;
        case '\\': out += "\\\\"; break;
        case '\b': out += "\\b"; break;
        case '\f': out += "\\f"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '\t': out += "\\t"; break;
        default:
            if (c < 0x20) {
                out += "\\u00";
                out += hex[c >> 4];
                out += hex[c & 15];
            } else out += static_cast<char>(c);
        }
    }
    return out + '"';
}

inline const char* kind_name(Kind kind) {
    switch (kind) {
    case Kind::Section: return "section";
    case Kind::Paragraph: return "paragraph";
    case Kind::Equation: return "equation";
    case Kind::Figure: return "figure";
    case Kind::Table: return "table";
    case Kind::CodeBlock: return "code_block";
    case Kind::List: return "list";
    case Kind::DefinitionList: return "definition_list";
    case Kind::Quote: return "quote";
    case Kind::Admonition: return "admonition";
    case Kind::HorizontalRule: return "horizontal_rule";
    case Kind::PageBreak: return "page_break";
    case Kind::Raw: return "raw";
    }
    return "unknown";
}

inline const char* inline_kind_name(InlineKind kind) {
    switch (kind) {
    case InlineKind::Text: return "text";
    case InlineKind::Emphasis: return "emphasis";
    case InlineKind::Strong: return "strong";
    case InlineKind::Code: return "code";
    case InlineKind::Math: return "math";
    case InlineKind::Link: return "link";
    case InlineKind::Reference: return "reference";
    case InlineKind::Citation: return "citation";
    }
    return "unknown";
}

inline const char* backend_name(Backend backend) {
    switch (backend) {
    case Backend::Markdown: return "markdown";
    case Backend::Rst: return "rst";
    case Backend::Latex: return "latex";
    case Backend::Html: return "html";
    case Backend::Typst: return "typst";
    case Backend::PlainText: return "plain_text";
    case Backend::Json: return "json";
    }
    return "unknown";
}

inline void json_inline(std::ostream& out, const InlineContent& content) {
    out << '[';
    bool first = true;
    for (const auto& part : content.content) {
        if (!first) out << ',';
        first = false;
        out << "{\"type\":" << json_quote(inline_kind_name(part.kind));
        if (part.kind == InlineKind::Reference || part.kind == InlineKind::Citation)
            out << ",\"target\":" << json_quote(part.target);
        else {
            out << ",\"value\":" << json_quote(part.value);
            if (part.kind == InlineKind::Link) out << ",\"target\":" << json_quote(part.target);
        }
        out << '}';
    }
    out << ']';
}

inline void json_nodes(std::ostream& out, const std::list<Node>& nodes);

inline void json_node(std::ostream& out, const Node& node) {
    out << "{\"type\":" << json_quote(kind_name(node.kind))
        << ",\"id\":" << json_quote(node.id) << ",\"roles\":[";
    for (std::size_t i = 0; i < node.roles.size(); ++i) {
        if (i) out << ',';
        out << json_quote(node.roles[i]);
    }
    out << ']';
    switch (node.kind) {
    case Kind::Section:
        out << ",\"title\":"; json_inline(out, node.heading_content);
        out << ",\"children\":"; json_nodes(out, node.children); break;
    case Kind::Paragraph:
        out << ",\"content\":"; json_inline(out, node.inlines); break;
    case Kind::Equation: case Kind::CodeBlock: case Kind::Quote: case Kind::Admonition: case Kind::Raw:
        out << ",\"value\":" << json_quote(node.value);
        if (node.kind == Kind::CodeBlock) out << ",\"language\":" << json_quote(node.language_name);
        if (node.kind == Kind::Admonition) out << ",\"kind\":" << json_quote(node.admonition_kind);
        if (node.kind == Kind::Raw) out << ",\"backend\":" << json_quote(backend_name(node.raw_backend));
        break;
    case Kind::Figure:
        out << ",\"path\":" << json_quote(node.value) << ",\"caption\":";
        json_inline(out, node.caption_content);
        out << ",\"width\":" << std::setprecision(17) << node.width_fraction; break;
    case Kind::Table:
        require_table(node);
        out << ",\"caption\":"; json_inline(out, node.caption_content);
        out << ",\"headers\":[";
        for (std::size_t i = 0; i < node.headers.size(); ++i) {
            if (i) out << ',';
            json_inline(out, node.headers[i]);
        }
        out << "],\"rows\":[";
        for (std::size_t i = 0; i < node.rows.size(); ++i) {
            if (i) out << ',';
            out << '[';
            for (std::size_t j = 0; j < node.rows[i].size(); ++j) {
                if (j) out << ',';
                json_inline(out, node.rows[i][j]);
            }
            out << ']';
        }
        out << ']'; break;
    case Kind::List: {
        out << ",\"ordered\":" << (node.ordered ? "true" : "false") << ",\"items\":[";
        bool first = true;
        for (const auto& item : node.items) {
            if (!first) out << ',';
            first = false;
            out << "{\"content\":"; json_inline(out, item);
            out << ",\"children\":"; json_nodes(out, item.children);
            out << '}';
        }
        out << ']'; break;
    }
    case Kind::DefinitionList: {
        out << ",\"items\":[";
        bool first = true;
        for (const auto& item : node.definitions) {
            if (!first) out << ',';
            first = false;
            out << "{\"term\":"; json_inline(out, item.term);
            out << ",\"description\":"; json_inline(out, item.description);
            out << '}';
        }
        out << ']'; break;
    }
    case Kind::HorizontalRule: case Kind::PageBreak: break;
    }
    out << '}';
}

inline void json_nodes(std::ostream& out, const std::list<Node>& nodes) {
    out << '[';
    bool first = true;
    for (const auto& node : nodes) {
        if (!first) out << ',';
        first = false;
        json_node(out, node);
    }
    out << ']';
}

} // namespace detail

class JsonRenderer {
public:
    std::string render(const Document& doc) const {
        std::ostringstream out;
        out << "{\"schema\":\"cpp-reportlib/1\",\"metadata\":{"
            << "\"title\":" << detail::json_quote(doc.metadata.title)
            << ",\"subtitle\":" << detail::json_quote(doc.metadata.subtitle)
            << ",\"author\":" << detail::json_quote(doc.metadata.author)
            << ",\"date\":" << detail::json_quote(doc.metadata.date)
            << ",\"institution\":" << detail::json_quote(doc.metadata.institution)
            << ",\"abstract\":" << detail::json_quote(doc.metadata.abstract)
            << ",\"keywords\":[";
        for (std::size_t i = 0; i < doc.metadata.keywords.size(); ++i) {
            if (i) out << ',';
            out << detail::json_quote(doc.metadata.keywords[i]);
        }
        out << "]},\"children\":";
        detail::json_nodes(out, doc.children);
        out << "}\n";
        return out.str();
    }
};

} // namespace report
