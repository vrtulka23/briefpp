#pragma once

#include <list>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace briefpp {

enum class Backend { Markdown, Rst, Latex, Html, Typst, PlainText, Json };
enum class Kind { Section, Paragraph, Equation, Figure, Table, CodeBlock,
                  List, DefinitionList, Quote, Admonition, HorizontalRule,
                  PageBreak, Raw };
enum class InlineKind { Text, Emphasis, Strong, Code, Math, Link, Reference, Citation };

struct Inline {
    InlineKind kind = InlineKind::Text;
    std::string value;
    std::string target;
};

class InlineContent {
public:
    std::vector<Inline> content;
    InlineContent() = default;
    InlineContent(std::string value) { text(std::move(value)); }
    InlineContent& text(std::string value) { return add(InlineKind::Text, std::move(value)); }
    InlineContent& emphasis(std::string value) { return add(InlineKind::Emphasis, std::move(value)); }
    InlineContent& strong(std::string value) { return add(InlineKind::Strong, std::move(value)); }
    InlineContent& code(std::string value) { return add(InlineKind::Code, std::move(value)); }
    InlineContent& math(std::string value) { return add(InlineKind::Math, std::move(value)); }
    InlineContent& link(std::string value, std::string target) {
        content.push_back({InlineKind::Link, std::move(value), std::move(target)}); return *this;
    }
    InlineContent& reference(std::string target) {
        content.push_back({InlineKind::Reference, {}, std::move(target)}); return *this;
    }
    InlineContent& citation(std::string key) {
        content.push_back({InlineKind::Citation, {}, std::move(key)}); return *this;
    }
    bool empty() const { return content.empty(); }
private:
    InlineContent& add(InlineKind kind, std::string value) {
        content.push_back({kind, std::move(value), {}}); return *this;
    }
};

class Node;
class ListItem : public InlineContent {
public:
    using InlineContent::InlineContent;
    std::list<Node> children;
    Node& list(bool numbered = false);
};

struct DefinitionItem {
    InlineContent term;
    InlineContent description;
};

class Node {
public:
    Kind kind;
    std::string value;
    std::string id;
    std::vector<std::string> roles;
    std::string language_name;
    std::string admonition_kind;
    double width_fraction = 1.0;
    bool ordered = false;
    Backend raw_backend = Backend::Markdown;
    InlineContent inlines;
    InlineContent heading_content;
    InlineContent caption_content;
    std::size_t table_column_count = 0;
    std::vector<InlineContent> headers;
    std::vector<std::vector<InlineContent>> rows;
    std::list<ListItem> items;
    std::list<DefinitionItem> definitions;
    std::list<Node> children;

    explicit Node(Kind type, std::string content = {})
        : kind(type), value(std::move(content)) {
        if (kind == Kind::Section && !value.empty()) heading_content.text(value);
    }

    Node& label(std::string text) { id = std::move(text); return *this; }
    Node& role(std::string text) { roles.push_back(std::move(text)); return *this; }
    InlineContent& heading() {
        if (kind != Kind::Section) throw std::logic_error("heading requires a section");
        return heading_content;
    }
    Node& caption(std::string text) { caption_content = InlineContent(std::move(text)); return *this; }
    InlineContent& caption() { return caption_content; }
    Node& width(double fraction) {
        if (fraction <= 0.0 || fraction > 1.0)
            throw std::invalid_argument("figure width must be in (0, 1]");
        width_fraction = fraction;
        return *this;
    }
    Node& language(std::string text) { language_name = std::move(text); return *this; }
    Node& text(std::string value) { inlines.text(std::move(value)); return *this; }
    Node& emphasis(std::string value) { inlines.emphasis(std::move(value)); return *this; }
    Node& strong(std::string value) { inlines.strong(std::move(value)); return *this; }
    Node& code(std::string value) { inlines.code(std::move(value)); return *this; }
    Node& math(std::string value) { inlines.math(std::move(value)); return *this; }
    Node& link(std::string text, std::string url) {
        inlines.link(std::move(text), std::move(url));
        return *this;
    }
    Node& reference(std::string target) {
        inlines.reference(std::move(target));
        return *this;
    }
    Node& citation(std::string key) { inlines.citation(std::move(key)); return *this; }
    Node& columns(std::vector<InlineContent> names) {
        if (kind != Kind::Table) throw std::logic_error("columns requires a table");
        if (!rows.empty()) throw std::logic_error("set columns before rows");
        table_column_count = names.size();
        headers = std::move(names);
        return *this;
    }
    Node& column_count(std::size_t count) {
        if (kind != Kind::Table) throw std::logic_error("column_count requires a table");
        if (!headers.empty() || !rows.empty()) throw std::logic_error("set column_count before cells or rows");
        if (count == 0) throw std::invalid_argument("table column count must be positive");
        table_column_count = count;
        return *this;
    }
    Node& columns(std::vector<std::string> names) {
        std::vector<InlineContent> rich;
        for (auto& name : names) rich.emplace_back(std::move(name));
        return columns(std::move(rich));
    }
    template<class... Args> Node& columns(Args&&... names) {
        return columns(std::vector<std::string>{std::forward<Args>(names)...});
    }
    Node& row(std::vector<InlineContent> cells) {
        if (kind != Kind::Table) throw std::logic_error("row requires a table");
        if (cells.empty()) throw std::invalid_argument("table rows must have cells");
        if (table_column_count == 0) table_column_count = cells.size();
        if (cells.size() != table_column_count ||
            (!rows.empty() && rows.back().size() != table_column_count))
            throw std::invalid_argument("table row width must match columns");
        rows.push_back(std::move(cells));
        return *this;
    }
    Node& row(std::vector<std::string> cells) {
        std::vector<InlineContent> rich;
        for (auto& cell : cells) rich.emplace_back(std::move(cell));
        return row(std::move(rich));
    }
    template<class... Args> Node& row(Args&&... cells) {
        return row(std::vector<std::string>{std::forward<Args>(cells)...});
    }
    Node& item(std::string content) {
        if (kind != Kind::List) throw std::logic_error("item requires a list");
        items.emplace_back(std::move(content));
        return *this;
    }
    ListItem& item() {
        if (kind != Kind::List) throw std::logic_error("item requires a list");
        items.emplace_back();
        return items.back();
    }
    Node& item(std::string term, std::string description) {
        if (kind != Kind::DefinitionList) throw std::logic_error("two-argument item requires a definition list");
        definitions.push_back({InlineContent(std::move(term)), InlineContent(std::move(description))});
        return *this;
    }
    DefinitionItem& definition_item() {
        if (kind != Kind::DefinitionList) throw std::logic_error("definition_item requires a definition list");
        definitions.emplace_back();
        return definitions.back();
    }
    InlineContent& cell() {
        if (kind != Kind::Table) throw std::logic_error("cell requires a table");
        if (table_column_count == 0) throw std::logic_error("set columns or column_count before cells");
        if (rows.empty() || rows.back().size() == table_column_count) rows.emplace_back();
        rows.back().emplace_back();
        return rows.back().back();
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
    Node& definition_list() { return child(Kind::DefinitionList); }
    Node& horizontal_rule() { return child(Kind::HorizontalRule); }
    Node& page_break() { return child(Kind::PageBreak); }
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
    Node& child(Kind type, std::string content = {}) {
        if (kind != Kind::Section) throw std::logic_error("only sections can contain blocks");
        children.emplace_back(type, std::move(content));
        return children.back();
    }
};

inline Node& ListItem::list(bool numbered) {
    children.emplace_back(Kind::List);
    children.back().ordered = numbered;
    return children.back();
}

} // namespace briefpp
