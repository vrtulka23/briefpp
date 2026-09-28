#pragma once

#include "document.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace briefpp {
namespace detail {

inline std::string replace_all(std::string value, const std::string& from, const std::string& to) {
    std::size_t pos = 0;
    while ((pos = value.find(from, pos)) != std::string::npos) {
        value.replace(pos, from.size(), to);
        pos += to.size();
    }
    return value;
}

inline std::string escape_markdown(const std::string& value) {
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        const bool line_start = i == 0 || value[i - 1] == '\n';
        if (c == '\\' || c == '*' || c == '_' || c == '[' || c == ']' || c == '`' ||
            c == '<' || c == '>' || c == '|' || c == '#' || c == '!' || c == '~' ||
            (line_start && (c == '+' || c == '-'))) out += '\\';
        out += c;
    }
    return out;
}

inline std::string escape_rst(const std::string& value) {
    std::string out;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c == '\\' || c == '*' || c == '`' || c == '|' ||
            (c == '.' && (i == 0 || value[i - 1] == '\n') && i + 1 < value.size() && value[i + 1] == '.')) out += '\\';
        out += c;
    }
    return out;
}

inline std::string escape_latex(const std::string& value) {
    std::string out;
    for (char c : value) {
        switch (c) {
        case '\\': out += "\\textbackslash{}"; break;
        case '{': out += "\\{"; break;
        case '}': out += "\\}"; break;
        case '$': out += "\\$"; break;
        case '&': out += "\\&"; break;
        case '%': out += "\\%"; break;
        case '#': out += "\\#"; break;
        case '_': out += "\\_"; break;
        case '^': out += "\\textasciicircum{}"; break;
        case '~': out += "\\textasciitilde{}"; break;
        case '<': out += "\\textless{}"; break;
        case '>': out += "\\textgreater{}"; break;
        case '|': out += "\\textbar{}"; break;
        default: out += c;
        }
    }
    return out;
}

inline std::string decimal(double value) {
    std::ostringstream stream;
    stream << std::setprecision(3) << value;
    return stream.str();
}

inline std::string bibliography_text(const BibliographyEntry& entry) {
    const auto sentence = [](const std::string& value) {
        return value + (value.back() == '.' || value.back() == '!' || value.back() == '?' ? "" : ".");
    };
    std::string out = sentence(entry.author) + " " + sentence(entry.title);
    if (!entry.year.empty()) out += " " + sentence(entry.year);
    return out;
}

inline void validate_citations(const Document& doc) {
    const auto check_inline = [&](const InlineContent& content) {
        for (const auto& part : content.content) {
            if (part.kind != InlineKind::Citation) continue;
            const auto found = std::find_if(doc.bibliography.begin(), doc.bibliography.end(),
                [&](const BibliographyEntry& entry) { return entry.key == part.target; });
            if (found == doc.bibliography.end())
                throw std::invalid_argument("citation has no bibliography entry: " + part.target);
        }
    };
    const auto check_node = [&](const auto& self, const Node& node) -> void {
        check_inline(node.inlines);
        check_inline(node.heading_content);
        check_inline(node.caption_content);
        for (const auto& cell : node.headers) check_inline(cell);
        for (const auto& row : node.rows) for (const auto& cell : row) check_inline(cell);
        for (const auto& item : node.definitions) {
            check_inline(item.term);
            check_inline(item.description);
        }
        for (const auto& item : node.items) {
            check_inline(item);
            for (const auto& child : item.children) self(self, child);
        }
        for (const auto& child : node.children) self(self, child);
    };
    for (const auto& node : doc.children) check_node(check_node, node);
}

inline std::string inline_text(const InlineContent& content, Backend backend) {
    std::string out;
    for (const Inline& part : content.content) {
        const auto esc = backend == Backend::Latex ? escape_latex :
                         backend == Backend::Rst ? escape_rst : escape_markdown;
        switch (part.kind) {
        case InlineKind::Text: out += esc(part.value); break;
        case InlineKind::Emphasis:
            out += backend == Backend::Latex ? "\\emph{" + esc(part.value) + "}" : "*" + esc(part.value) + "*"; break;
        case InlineKind::Strong:
            out += backend == Backend::Latex ? "\\textbf{" + esc(part.value) + "}" : "**" + esc(part.value) + "**"; break;
        case InlineKind::Code:
            out += backend == Backend::Latex ? "\\texttt{" + esc(part.value) + "}" :
                   backend == Backend::Rst ? "``" + part.value + "``" : "`" + part.value + "`"; break;
        case InlineKind::Math:
            out += backend == Backend::Rst ? ":math:`" + part.value + "`" :
                   backend == Backend::Latex ? "\\(" + part.value + "\\)" : "$" + part.value + "$"; break;
        case InlineKind::Link:
            out += backend == Backend::Latex ? "\\href{" + escape_latex(part.target) + "}{" + esc(part.value) + "}" :
                   backend == Backend::Rst ? "`" + esc(part.value) + " <" + part.target + ">`_" :
                   "[" + esc(part.value) + "](" + part.target + ")"; break;
        case InlineKind::Reference:
            out += backend == Backend::Latex ? "\\ref{" + escape_latex(part.target) + "}" :
                   backend == Backend::Rst ? ":ref:`" + part.target + "`" : "[](#" + part.target + ")"; break;
        case InlineKind::Citation:
            out += backend == Backend::Latex ? "\\cite{" + part.target + "}" :
                   backend == Backend::Markdown ? "[[" + escape_markdown(part.target) + "]](#bib-" + part.target + ")" :
                   "[" + part.target + "]_"; break;
        }
    }
    return out;
}

inline std::string inline_text(const Node& node, Backend backend) {
    return inline_text(node.inlines, backend);
}

inline std::string plain_inline(const InlineContent& content) {
    std::string out;
    for (const auto& part : content.content) {
        if (part.kind == InlineKind::Reference || part.kind == InlineKind::Citation)
            out += "[" + part.target + "]";
        else out += part.value;
    }
    return out;
}

inline std::size_t require_table(const Node& node) {
    if (node.table_column_count == 0) throw std::logic_error("table needs columns or rows");
    if (node.headers.empty() && node.rows.empty()) throw std::logic_error("headerless table needs a row");
    if (!node.headers.empty() && node.headers.size() != node.table_column_count)
        throw std::logic_error("table header width must match columns");
    for (const auto& row : node.rows)
        if (row.size() != node.table_column_count)
            throw std::logic_error("table row width must match columns");
    return node.table_column_count;
}

} // namespace detail
} // namespace briefpp
