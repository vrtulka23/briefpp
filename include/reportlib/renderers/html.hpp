#pragma once

#include "../renderer.hpp"

namespace report {
namespace detail {

inline std::string escape_html(const std::string& value) {
    std::string out;
    for (char c : value) {
        switch (c) {
        case '&': out += "&amp;"; break;
        case '<': out += "&lt;"; break;
        case '>': out += "&gt;"; break;
        case '"': out += "&quot;"; break;
        case '\'': out += "&#39;"; break;
        default: out += c;
        }
    }
    return out;
}

inline std::string html_inline(const InlineContent& content) {
    std::string out;
    for (const auto& part : content.content) {
        const std::string value = escape_html(part.value);
        switch (part.kind) {
        case InlineKind::Text: out += value; break;
        case InlineKind::Emphasis: out += "<em>" + value + "</em>"; break;
        case InlineKind::Strong: out += "<strong>" + value + "</strong>"; break;
        case InlineKind::Code: out += "<code>" + value + "</code>"; break;
        case InlineKind::Math: out += "<span class=\"math\">" + value + "</span>"; break;
        case InlineKind::Link:
            out += "<a href=\"" + escape_html(part.target) + "\">" + value + "</a>"; break;
        case InlineKind::Reference:
            out += "<a href=\"#" + escape_html(part.target) + "\">[" + escape_html(part.target) + "]</a>"; break;
        case InlineKind::Citation:
            out += "<cite data-cite-key=\"" + escape_html(part.target) + "\">[" + escape_html(part.target) + "]</cite>"; break;
        }
    }
    return out;
}

inline std::string html_attributes(const Node& node, const std::string& extra_class = {}) {
    std::string out;
    if (!node.id.empty()) out += " id=\"" + escape_html(node.id) + "\"";
    if (!node.roles.empty() || !extra_class.empty()) {
        out += " class=\"";
        if (!extra_class.empty()) out += escape_html(extra_class);
        for (std::size_t i = 0; i < node.roles.size(); ++i) {
            if (i || !extra_class.empty()) out += ' ';
            out += escape_html(node.roles[i]);
        }
        out += '"';
    }
    return out;
}

} // namespace detail

class HtmlRenderer {
public:
    HtmlRenderer& stylesheet(std::string path) { stylesheets_.push_back(std::move(path)); return *this; }

    std::string render(const Document& doc) const {
        std::ostringstream out;
        out << "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n<title>"
            << detail::escape_html(doc.metadata.title) << "</title>\n";
        for (const auto& path : stylesheets_)
            out << "<link rel=\"stylesheet\" href=\"" << detail::escape_html(path) << "\">\n";
        out << "</head>\n<body>\n";
        if (!doc.metadata.title.empty()) out << "<h1>" << detail::escape_html(doc.metadata.title) << "</h1>\n";
        if (!doc.metadata.subtitle.empty()) out << "<p class=\"subtitle\">" << detail::escape_html(doc.metadata.subtitle) << "</p>\n";
        if (!doc.metadata.author.empty()) out << "<p class=\"author\">" << detail::escape_html(doc.metadata.author) << "</p>\n";
        if (!doc.metadata.date.empty()) out << "<p class=\"date\">" << detail::escape_html(doc.metadata.date) << "</p>\n";
        if (!doc.metadata.institution.empty()) out << "<p class=\"institution\">" << detail::escape_html(doc.metadata.institution) << "</p>\n";
        if (!doc.metadata.abstract.empty()) out << "<section class=\"abstract\"><h2>Abstract</h2><p>" << detail::escape_html(doc.metadata.abstract) << "</p></section>\n";
        for (const auto& node : doc.children) render_node(out, node, doc.metadata.title.empty() ? 1 : 2);
        out << "</body>\n</html>\n";
        return out.str();
    }

private:
    std::vector<std::string> stylesheets_;

    static void render_node(std::ostringstream& out, const Node& node, int depth) {
        const auto attrs = detail::html_attributes(node);
        switch (node.kind) {
        case Kind::Section:
            out << "<section" << attrs << "><h" << std::min(depth, 6) << ">"
                << detail::html_inline(node.heading_content) << "</h" << std::min(depth, 6) << ">\n";
            for (const auto& child : node.children) render_node(out, child, depth + 1);
            out << "</section>\n";
            break;
        case Kind::Paragraph:
            out << "<p" << attrs << ">" << detail::html_inline(node.inlines) << "</p>\n"; break;
        case Kind::Equation:
            out << "<div" << detail::html_attributes(node, "equation") << "><code>" << detail::escape_html(node.value) << "</code></div>\n"; break;
        case Kind::Figure:
            out << "<figure" << attrs << "><img src=\"" << detail::escape_html(node.value)
                << "\" alt=\"" << detail::escape_html(detail::plain_inline(node.caption_content)) << "\">";
            if (!node.caption_content.empty()) out << "<figcaption>" << detail::html_inline(node.caption_content) << "</figcaption>";
            out << "</figure>\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            out << "<table" << attrs << ">";
            if (!node.caption_content.empty()) out << "<caption>" << detail::html_inline(node.caption_content) << "</caption>";
            out << "<thead><tr>";
            for (const auto& cell : node.headers) out << "<th scope=\"col\">" << detail::html_inline(cell) << "</th>";
            out << "</tr></thead><tbody>";
            for (const auto& row : node.rows) {
                out << "<tr>";
                for (const auto& cell : row) out << "<td>" << detail::html_inline(cell) << "</td>";
                out << "</tr>";
            }
            out << "</tbody></table>\n";
            break;
        case Kind::CodeBlock:
            out << "<pre" << attrs << "><code";
            if (!node.language_name.empty()) out << " class=\"language-" << detail::escape_html(node.language_name) << "\"";
            out << ">" << detail::escape_html(node.value) << "</code></pre>\n"; break;
        case Kind::List: render_list(out, node); break;
        case Kind::DefinitionList:
            out << "<dl" << attrs << ">";
            for (const auto& item : node.definitions)
                out << "<dt>" << detail::html_inline(item.term) << "</dt><dd>"
                    << detail::html_inline(item.description) << "</dd>";
            out << "</dl>\n";
            break;
        case Kind::Quote: out << "<blockquote" << attrs << ">" << detail::escape_html(node.value) << "</blockquote>\n"; break;
        case Kind::Admonition:
            out << "<aside" << attrs << "><strong>" << detail::escape_html(node.admonition_kind)
                << "</strong><p>" << detail::escape_html(node.value) << "</p></aside>\n"; break;
        case Kind::HorizontalRule: out << "<hr" << attrs << ">\n"; break;
        case Kind::PageBreak: out << "<div" << detail::html_attributes(node, "page-break") << "></div>\n"; break;
        case Kind::Raw: if (node.raw_backend == Backend::Html) out << node.value << "\n"; break;
        }
    }

    static void render_list(std::ostringstream& out, const Node& node) {
        const char* tag = node.ordered ? "ol" : "ul";
        out << "<" << tag << detail::html_attributes(node) << ">";
        for (const auto& item : node.items) {
            out << "<li>" << detail::html_inline(item);
            for (const auto& child : item.children) if (child.kind == Kind::List) render_list(out, child);
            out << "</li>";
        }
        out << "</" << tag << ">\n";
    }
};

} // namespace report
