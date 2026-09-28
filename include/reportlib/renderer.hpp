#pragma once

#include "document.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace report {
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
                   backend == Backend::Markdown ? "[@" + escape_markdown(part.target) + "]" :
                   "[" + esc(part.target) + "]"; break;
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

inline void require_table(const Node& node) {
    if (node.headers.empty()) throw std::logic_error("table needs columns");
    for (const auto& row : node.rows)
        if (row.size() != node.headers.size())
            throw std::logic_error("table row width must match columns");
}

} // namespace detail
} // namespace report
