"""Embed web UI files as C++ string constants.

Arduino can't serve files from the sketch folder, so each firmware/*/src/web/*.html
is turned into a sibling header, e.g. index.html -> index_html.h with
`const char kIndexHtml[] PROGMEM`. The headers are committed so a plain
`arduino-cli compile` works; `make build` re-runs this first.

    python3 tools/embed_web.py
"""

import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent
DELIM = "WEB"  # raw-string delimiter: the file must not contain )WEB"


def embed(src: pathlib.Path) -> pathlib.Path:
    stem = src.stem + "_" + src.suffix[1:]          # index_html
    name = "k" + "".join(p.title() for p in stem.split("_"))  # kIndexHtml
    text = src.read_text()
    assert f'){DELIM}"' not in text, f"{src} contains the raw-string delimiter"
    out = src.with_name(stem + ".h")
    out.write_text(
        f"// GENERATED from {src.name} by tools/embed_web.py. Do not edit.\n"
        "#pragma once\n\n#include <pgmspace.h>\n\n"
        f'const char {name}[] PROGMEM = R"{DELIM}({text}){DELIM}";\n'
    )
    return out


def main() -> None:
    for src in sorted(ROOT.glob("firmware/*/src/web/*.html")):
        print(f"{src.relative_to(ROOT)} -> {embed(src).name}")


if __name__ == "__main__":
    main()
