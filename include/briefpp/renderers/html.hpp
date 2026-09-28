#pragma once

#include "../renderer.hpp"

namespace briefpp {
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

inline std::string html_inline(const InlineContent& content, bool typeset_math = true) {
    std::string out;
    for (const auto& part : content.content) {
        const std::string value = escape_html(part.value);
        switch (part.kind) {
        case InlineKind::Text: out += value; break;
        case InlineKind::Emphasis: out += "<em>" + value + "</em>"; break;
        case InlineKind::Strong: out += "<strong>" + value + "</strong>"; break;
        case InlineKind::Code: out += "<code>" + value + "</code>"; break;
        case InlineKind::Math:
            out += typeset_math ? "<span class=\"math\">\\(" + value + "\\)</span>" :
                   "<code class=\"math\">" + value + "</code>";
            break;
        case InlineKind::Link:
            out += "<a href=\"" + escape_html(part.target) + "\">" + value + "</a>"; break;
        case InlineKind::Reference:
            out += "<a href=\"#" + escape_html(part.target) + "\">[" + escape_html(part.target) + "]</a>"; break;
        case InlineKind::Citation:
            out += "<a href=\"#bib-" + escape_html(part.target) + "\" class=\"citation\">[" + escape_html(part.target) + "]</a>"; break;
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
    HtmlRenderer& clean_html(bool enabled = true) { clean_ = enabled; return *this; }
    HtmlRenderer& mathjax_source(std::string path) { mathjax_source_ = std::move(path); return *this; }

    std::string render(const Document& doc) const {
        detail::validate_citations(doc);
        std::ostringstream out;
        out << "<!doctype html>\n<html lang=\"en\">\n<head>\n<meta charset=\"utf-8\">\n"
            << "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>"
            << detail::escape_html(doc.metadata.title) << "</title>\n";
        if (!clean_) {
            out << "<style>\n" << default_style() << "</style>\n";
            for (const auto& path : stylesheets_)
                out << "<link rel=\"stylesheet\" href=\"" << detail::escape_html(path) << "\">\n";
            out << "<script defer src=\"" << detail::escape_html(mathjax_source_) << "\"></script>\n";
        }
        out << "</head>\n<body>\n<main class=\"briefpp-report\">\n";
        if (!doc.metadata.title.empty() || !doc.metadata.subtitle.empty() || !doc.metadata.author.empty() ||
            !doc.metadata.date.empty() || !doc.metadata.institution.empty()) out << "<header class=\"report-header\">\n";
        if (!doc.metadata.title.empty()) out << "<h1>" << detail::escape_html(doc.metadata.title) << "</h1>\n";
        if (!doc.metadata.subtitle.empty()) out << "<p class=\"subtitle\">" << detail::escape_html(doc.metadata.subtitle) << "</p>\n";
        if (!doc.metadata.author.empty()) out << "<p class=\"author\">" << detail::escape_html(doc.metadata.author) << "</p>\n";
        if (!doc.metadata.date.empty()) out << "<p class=\"date\">" << detail::escape_html(doc.metadata.date) << "</p>\n";
        if (!doc.metadata.institution.empty()) out << "<p class=\"institution\">" << detail::escape_html(doc.metadata.institution) << "</p>\n";
        if (!doc.metadata.title.empty() || !doc.metadata.subtitle.empty() || !doc.metadata.author.empty() ||
            !doc.metadata.date.empty() || !doc.metadata.institution.empty()) out << "</header>\n";
        if (!doc.metadata.abstract.empty()) out << "<section class=\"abstract\"><h2>Abstract</h2><p>" << detail::escape_html(doc.metadata.abstract) << "</p></section>\n";
        for (const auto& node : doc.children) render_node(out, node, doc.metadata.title.empty() ? 1 : 2, !clean_);
        if (!doc.bibliography.empty()) {
            out << "<section class=\"references\"><h2>References</h2><ol>\n";
            for (const auto& entry : doc.bibliography) {
                out << "<li id=\"bib-" << entry.key << "\"><span>[" << detail::escape_html(entry.key) << "]</span> "
                    << detail::escape_html(detail::bibliography_text(entry));
                if (!entry.url.empty()) out << " <a href=\"" << detail::escape_html(entry.url) << "\">" << detail::escape_html(entry.url) << "</a>";
                out << "</li>\n";
            }
            out << "</ol></section>\n";
        }
        out << "</main>\n</body>\n</html>\n";
        return out.str();
    }

private:
    std::vector<std::string> stylesheets_;
    bool clean_ = false;
    std::string mathjax_source_ = "https://cdn.jsdelivr.net/npm/mathjax@4/tex-chtml.js";

    static const char* default_style() {
        return R"CSS(:root { color-scheme: light; --ink: #243347; --muted: #586b7d; --accent: #155c83; --line: #dbe4ea; --soft: #f2f7fa; }
* { box-sizing: border-box; }
html { scroll-behavior: smooth; }
body { margin: 0; background: #f4f7f9; color: var(--ink); font: 1rem/1.7 system-ui, -apple-system, BlinkMacSystemFont, "Segoe UI", sans-serif; }
.briefpp-report { max-width: 58rem; margin: 2.5rem auto; padding: 2.5rem 3rem; background: white; box-shadow: 0 8px 40px #23394b12; border-radius: 12px; }
.report-header { padding-bottom: 1.6rem; margin-bottom: 2.2rem; border-bottom: 2px solid var(--line); }
h1, h2, h3, h4, h5, h6 { color: #17364d; line-height: 1.25; margin: 1.8em 0 .6em; }
h1 { font-size: clamp(2rem, 5vw, 3rem); margin: 0 0 .25rem; }
h2 { font-size: 1.55rem; }
h3 { font-size: 1.2rem; }
p { margin: .6rem 0 1rem; }
.subtitle { color: var(--accent); font-size: 1.2rem; margin: 0 0 1.2rem; }
.author, .date, .institution { color: var(--muted); margin: .15rem 0; }
.abstract { padding: .5rem 1.2rem; background: var(--soft); border-left: 4px solid var(--accent); border-radius: 4px; }
.abstract h2 { margin-top: .7rem; }
a { color: var(--accent); text-decoration-thickness: 1px; text-underline-offset: 2px; }
a:hover { color: #0d3d58; }
figure { margin: 1.5rem 0; text-align: center; }
img { max-width: 100%; height: auto; border-radius: 6px; }
figcaption, caption { color: var(--muted); font-size: .92rem; }
table { width: 100%; border-collapse: collapse; margin: 1.25rem 0; font-variant-numeric: tabular-nums; }
.table-scroll { overflow-x: auto; }
caption { text-align: left; padding-bottom: .4rem; }
th, td { border-bottom: 1px solid var(--line); padding: .6rem .75rem; text-align: left; vertical-align: top; }
th { background: var(--soft); color: #17364d; }
pre { overflow-x: auto; padding: 1rem 1.2rem; background: #152b3d; color: #f4f8fb; border-radius: 6px; line-height: 1.5; }
code { font: .9em/1.5 ui-monospace, SFMono-Regular, Menlo, Consolas, monospace; }
:not(pre) > code { background: var(--soft); padding: .12em .3em; border-radius: 3px; }
.equation { margin: 1.5rem 0; overflow-x: auto; text-align: center; }
blockquote { margin: 1.5rem 0; padding: .4rem 1rem; border-left: 4px solid var(--accent); color: var(--muted); background: var(--soft); }
aside { margin: 1.25rem 0; padding: .7rem 1rem; background: #fff8e9; border-left: 4px solid #c78418; border-radius: 4px; }
aside p { margin: .2rem 0; }
dl dt { font-weight: 700; }
dl dd { margin: 0 0 .8rem 1rem; }
.references { border-top: 2px solid var(--line); margin-top: 3rem; }
.references ol { padding-left: 1.5rem; }
.references li { margin: .7rem 0; overflow-wrap: anywhere; }
.page-break { break-before: page; }
@media (max-width: 640px) { .briefpp-report { margin: 0; padding: 1.25rem; border-radius: 0; box-shadow: none; } }
@media print { body { background: white; } .briefpp-report { margin: 0; max-width: none; padding: 0; box-shadow: none; } a { color: inherit; } }
)CSS";
    }

    static void render_node(std::ostringstream& out, const Node& node, int depth, bool typeset_math) {
        const auto attrs = detail::html_attributes(node);
        switch (node.kind) {
        case Kind::Section:
            out << "<section" << attrs << "><h" << std::min(depth, 6) << ">"
                << detail::html_inline(node.heading_content, typeset_math) << "</h" << std::min(depth, 6) << ">\n";
            for (const auto& child : node.children) render_node(out, child, depth + 1, typeset_math);
            out << "</section>\n";
            break;
        case Kind::Paragraph:
            out << "<p" << attrs << ">" << detail::html_inline(node.inlines, typeset_math) << "</p>\n"; break;
        case Kind::Equation:
            out << "<div" << detail::html_attributes(node, "equation") << ">";
            if (typeset_math) out << "\\[" << detail::escape_html(node.value) << "\\]";
            else out << "<code>" << detail::escape_html(node.value) << "</code>";
            out << "</div>\n"; break;
        case Kind::Figure:
            out << "<figure" << attrs << "><img src=\"" << detail::escape_html(node.value)
                << "\" alt=\"" << detail::escape_html(detail::plain_inline(node.caption_content)) << "\">";
            if (!node.caption_content.empty()) out << "<figcaption>" << detail::html_inline(node.caption_content, typeset_math) << "</figcaption>";
            out << "</figure>\n";
            break;
        case Kind::Table:
            detail::require_table(node);
            if (typeset_math) out << "<div class=\"table-scroll\">";
            out << "<table" << attrs << ">";
            if (!node.caption_content.empty()) out << "<caption>" << detail::html_inline(node.caption_content, typeset_math) << "</caption>";
            if (!node.headers.empty()) {
                out << "<thead><tr>";
                for (const auto& cell : node.headers) out << "<th scope=\"col\">" << detail::html_inline(cell, typeset_math) << "</th>";
                out << "</tr></thead>";
            }
            out << "<tbody>";
            for (const auto& row : node.rows) {
                out << "<tr>";
                for (const auto& cell : row) out << "<td>" << detail::html_inline(cell, typeset_math) << "</td>";
                out << "</tr>";
            }
            out << "</tbody></table>";
            if (typeset_math) out << "</div>";
            out << "\n";
            break;
        case Kind::CodeBlock:
            out << "<pre" << attrs << "><code";
            if (!node.language_name.empty()) out << " class=\"language-" << detail::escape_html(node.language_name) << "\"";
            out << ">" << detail::escape_html(node.value) << "</code></pre>\n"; break;
        case Kind::List: render_list(out, node, typeset_math); break;
        case Kind::DefinitionList:
            out << "<dl" << attrs << ">";
            for (const auto& item : node.definitions)
                out << "<dt>" << detail::html_inline(item.term, typeset_math) << "</dt><dd>"
                    << detail::html_inline(item.description, typeset_math) << "</dd>";
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

    static void render_list(std::ostringstream& out, const Node& node, bool typeset_math) {
        const char* tag = node.ordered ? "ol" : "ul";
        out << "<" << tag << detail::html_attributes(node) << ">";
        for (const auto& item : node.items) {
            out << "<li>" << detail::html_inline(item, typeset_math);
            for (const auto& child : item.children) if (child.kind == Kind::List) render_list(out, child, typeset_math);
            out << "</li>";
        }
        out << "</" << tag << ">\n";
    }
};

} // namespace briefpp
