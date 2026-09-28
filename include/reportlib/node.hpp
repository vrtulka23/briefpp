#pragma once

#include <list>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace report {

enum class Backend { Markdown, Rst, Latex };
enum class Kind { Section, Paragraph, Equation, Figure, Table, CodeBlock,
                  List, Quote, Admonition, Raw };
enum class InlineKind { Text, Emphasis, Strong, Code, Math, Link, Reference };

struct Inline {
    InlineKind kind = InlineKind::Text;
    std::string value;
    std::string target;
};

class Node {
public:
    Kind kind;
    std::string value;
    std::string id;
    std::string caption_text;
    std::string language_name;
    std::string admonition_kind;
    double width_fraction = 1.0;
    bool ordered = false;
    Backend raw_backend = Backend::Markdown;
    std::vector<Inline> inlines;
    std::vector<std::string> headers;
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> items;
    std::list<Node> children;

    explicit Node(Kind type, std::string content = {})
        : kind(type), value(std::move(content)) {}

    Node& label(std::string text) { id = std::move(text); return *this; }
    Node& caption(std::string text) { caption_text = std::move(text); return *this; }
    Node& width(double fraction) {
        if (fraction <= 0.0 || fraction > 1.0)
            throw std::invalid_argument("figure width must be in (0, 1]");
        width_fraction = fraction;
        return *this;
    }
    Node& language(std::string text) { language_name = std::move(text); return *this; }
    Node& text(std::string value) { return add(InlineKind::Text, std::move(value)); }
    Node& emphasis(std::string value) { return add(InlineKind::Emphasis, std::move(value)); }
    Node& strong(std::string value) { return add(InlineKind::Strong, std::move(value)); }
    Node& code(std::string value) { return add(InlineKind::Code, std::move(value)); }
    Node& math(std::string value) { return add(InlineKind::Math, std::move(value)); }
    Node& link(std::string text, std::string url) {
        inlines.push_back({InlineKind::Link, std::move(text), std::move(url)});
        return *this;
    }
    Node& reference(std::string target) {
        inlines.push_back({InlineKind::Reference, {}, std::move(target)});
        return *this;
    }
    Node& columns(std::vector<std::string> names) {
        if (kind != Kind::Table) throw std::logic_error("columns requires a table");
        if (!rows.empty()) throw std::logic_error("set columns before rows");
        headers = std::move(names);
        return *this;
    }
    template<class... Args> Node& columns(Args&&... names) {
        return columns(std::vector<std::string>{std::forward<Args>(names)...});
    }
    Node& row(std::vector<std::string> cells) {
        if (kind != Kind::Table) throw std::logic_error("row requires a table");
        if (headers.empty() || cells.size() != headers.size())
            throw std::invalid_argument("table row width must match columns");
        rows.push_back(std::move(cells));
        return *this;
    }
    template<class... Args> Node& row(Args&&... cells) {
        return row(std::vector<std::string>{std::forward<Args>(cells)...});
    }
    Node& item(std::string content) {
        if (kind != Kind::List) throw std::logic_error("item requires a list");
        items.push_back(std::move(content));
        return *this;
    }

    Node& section(std::string heading) { return child(Kind::Section, std::move(heading)); }
    Node& paragraph(std::string content = {}) {
        Node& node = child(Kind::Paragraph);
        if (!content.empty()) node.text(std::move(content));
        return node;
    }
    Node& equation(std::string math, std::string label = {}) {
        return child(Kind::Equation, std::move(math)).label(std::move(label));
    }
    Node& figure(std::string path) { return child(Kind::Figure, std::move(path)); }
    Node& table() { return child(Kind::Table); }
    Node& code_block(std::string content, std::string language = {}) {
        return child(Kind::CodeBlock, std::move(content)).language(std::move(language));
    }
    Node& list(bool numbered = false) {
        Node& node = child(Kind::List);
        node.ordered = numbered;
        return node;
    }
    Node& quote(std::string content) { return child(Kind::Quote, std::move(content)); }
    Node& admonition(std::string type, std::string content) {
        Node& node = child(Kind::Admonition, std::move(content));
        node.admonition_kind = std::move(type);
        return node;
    }
    Node& warning(std::string content) { return admonition("warning", std::move(content)); }
    Node& note(std::string content) { return admonition("note", std::move(content)); }
    Node& raw(Backend backend, std::string content) {
        Node& node = child(Kind::Raw, std::move(content));
        node.raw_backend = backend;
        return node;
    }

private:
    Node& add(InlineKind type, std::string content) {
        inlines.push_back({type, std::move(content), {}});
        return *this;
    }
    Node& child(Kind type, std::string content = {}) {
        if (kind != Kind::Section) throw std::logic_error("only sections can contain blocks");
        children.emplace_back(type, std::move(content));
        return children.back();
    }
};

} // namespace report
