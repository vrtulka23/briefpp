#pragma once

#include "renderers/markdown.hpp"
#include "renderers/rst.hpp"
#include "renderers/latex.hpp"
#include "renderers/html.hpp"
#include "renderers/typst.hpp"
#include "renderers/plain_text.hpp"
#include "renderers/json.hpp"

namespace report {

inline void Document::write(const std::string& path) const {
    const std::size_t dot = path.find_last_of('.');
    const std::string extension = dot == std::string::npos ? "" : path.substr(dot);
    std::string content;
    if (extension == ".md") content = MarkdownRenderer{}.render(*this);
    else if (extension == ".rst") content = RstRenderer{}.render(*this);
    else if (extension == ".tex") content = LatexRenderer{}.render(*this);
    else if (extension == ".html") content = HtmlRenderer{}.render(*this);
    else if (extension == ".typ") content = TypstRenderer{}.render(*this);
    else if (extension == ".txt") content = PlainTextRenderer{}.render(*this);
    else if (extension == ".json") content = JsonRenderer{}.render(*this);
    else throw std::invalid_argument("unsupported report extension: " + extension);
    std::ofstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("cannot open report: " + path);
    file << content;
    if (!file) throw std::runtime_error("cannot write report: " + path);
}

} // namespace report
