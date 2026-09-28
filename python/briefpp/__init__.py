"""Locate the C++ headers shipped with the Brief++ distribution."""

from pathlib import Path


def get_include() -> str:
    """Return the directory to pass to a C++ compiler with ``-I``."""
    return str(Path(__file__).resolve().parent / "include")
