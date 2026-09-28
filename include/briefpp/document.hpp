#pragma once

#include "node.hpp"
#include <fstream>

namespace briefpp {

struct BibliographyEntry {
    std::string key, author, title, year, url;
};

class Document {
public:
    struct Metadata {
        std::string title, subtitle, author, date, institution, abstract;
        std::vector<std::string> keywords;
    } metadata;
    std::list<Node> children;
    std::vector<BibliographyEntry> bibliography;

    Document& title(std::string value) { metadata.title = std::move(value); return *this; }
    Document& subtitle(std::string value) { metadata.subtitle = std::move(value); return *this; }
    Document& author(std::string value) { metadata.author = std::move(value); return *this; }
    Document& date(std::string value) { metadata.date = std::move(value); return *this; }
    Document& institution(std::string value) { metadata.institution = std::move(value); return *this; }
    Document& abstract(std::string value) { metadata.abstract = std::move(value); return *this; }
    Document& keywords(std::vector<std::string> values) { metadata.keywords = std::move(values); return *this; }
    Document& bibliography_entry(std::string key, std::string author, std::string title,
                                 std::string year, std::string url = {}) {
        if (key.empty() || !ascii_alnum(key.front()))
            throw std::invalid_argument("bibliography key must start with an ASCII letter or digit");
        for (char c : key)
            if (!ascii_alnum(c) && c != '-' && c != '_' && c != '.')
                throw std::invalid_argument("bibliography key contains an invalid character: " + key);
        if (author.empty() || title.empty())
            throw std::invalid_argument("bibliography author and title must be nonempty");
        for (const auto& entry : bibliography)
            if (entry.key == key) throw std::invalid_argument("duplicate bibliography key: " + key);
        bibliography.push_back({std::move(key), std::move(author), std::move(title),
                                std::move(year), std::move(url)});
        return *this;
    }

    Node& section(std::string heading) { return add(Kind::Section, std::move(heading)); }
    Node& paragraph(std::string content = {}) {
        Node& node = add(Kind::Paragraph);
        if (!content.empty()) node.text(std::move(content));
        return node;
    }
    Node& equation(std::string math, std::string label = {}) {
        return add(Kind::Equation, std::move(math)).label(std::move(label));
    }
    Node& figure(std::string path) { return add(Kind::Figure, std::move(path)); }
    Node& table() { return add(Kind::Table); }
    Node& code_block(std::string content, std::string language = {}) {
        return add(Kind::CodeBlock, std::move(content)).language(std::move(language));
    }
    Node& list(bool numbered = false) {
        Node& node = add(Kind::List);
        node.ordered = numbered;
        return node;
    }
    Node& definition_list() { return add(Kind::DefinitionList); }
    Node& horizontal_rule() { return add(Kind::HorizontalRule); }
    Node& page_break() { return add(Kind::PageBreak); }
    Node& quote(std::string content) { return add(Kind::Quote, std::move(content)); }
    Node& admonition(std::string type, std::string content) {
        Node& node = add(Kind::Admonition, std::move(content));
        node.admonition_kind = std::move(type);
        return node;
    }
    Node& warning(std::string content) { return admonition("warning", std::move(content)); }
    Node& note(std::string content) { return admonition("note", std::move(content)); }
    Node& raw(Backend backend, std::string content) {
        Node& node = add(Kind::Raw, std::move(content));
        node.raw_backend = backend;
        return node;
    }
    Document& append(const Document& fragment) {
        for (const auto& entry : fragment.bibliography) {
            bool found = false;
            for (const auto& existing : bibliography) {
                if (existing.key != entry.key) continue;
                if (existing.author != entry.author || existing.title != entry.title ||
                    existing.year != entry.year || existing.url != entry.url)
                    throw std::invalid_argument("conflicting bibliography key: " + entry.key);
                found = true;
                break;
            }
            if (!found) bibliography.push_back(entry);
        }
        children.insert(children.end(), fragment.children.begin(), fragment.children.end());
        return *this;
    }
    Document& operator<<(const Document& fragment) { return append(fragment); }

    // Defined in report.hpp, where all renderers are available.
    void write(const std::string& path) const;

private:
    static bool ascii_alnum(char c) {
        return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9');
    }
    Node& add(Kind type, std::string content = {}) {
        children.emplace_back(type, std::move(content));
        return children.back();
    }
};

using DocumentFragment = Document;

} // namespace briefpp
