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
    for (char c : value) {
        if (c == '\\' || c == '*' || c == '_' || c == '[' || c == ']' || c == '`' || c == '<' || c == '>' || c == '|') out += '\\';
        out += c;
    }
    return out;
}

inline std::string escape_rst(const std::string& value) {
    std::string out;
    for (char c : value) {
        if (c == '\\' || c == '*' || c == '`' || c == '|') out += '\\';
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

inline std::string inline_text(const Node& node, Backend backend) {
    std::string out;
    for (const Inline& part : node.inlines) {
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
        }
    }
    return out;
}

inline void require_table(const Node& node) {
    if (node.headers.empty()) throw std::logic_error("table needs columns");
}

} // namespace detail
} // namespace report
