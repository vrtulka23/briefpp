"""Python bindings and bundled C++ headers for Brief++."""

from pathlib import Path

from ._core import (
    Backend,
    DefinitionItem,
    Document,
    DocumentFragment,
    HtmlRenderer,
    InlineContent,
    JsonRenderer,
    Kind,
    LatexRenderer,
    ListItem,
    MarkdownRenderer,
    Node,
    PlainTextRenderer,
    RstRenderer,
    TypstRenderer,
)

__all__ = [
    "Backend", "DefinitionItem", "Document", "DocumentFragment", "HtmlRenderer", "InlineContent",
    "JsonRenderer", "Kind", "LatexRenderer", "ListItem", "MarkdownRenderer",
    "Node", "PlainTextRenderer", "RstRenderer", "TypstRenderer", "get_include",
]


def get_include() -> str:
    """Return the directory to pass to a C++ compiler with ``-I``."""
    package = Path(__file__).resolve().parent
    bundled = package / "include"
    if bundled.is_dir():
        return str(bundled)
    return str(package.parents[1] / "include")
