#!/usr/bin/env python3
"""
Splits a monolithic main.cpp into modular headers (include/) and
parallel compilation units (src/), backing up main.cpp first.
"""

import re
import shutil
from pathlib import Path

MAIN_FILE = Path("main.cpp")
BACKUP_FILE = Path("main.cpp.bak")
INC_DIR = Path("include")
SRC_DIR = Path("src")


def find_matching_brace(text: str, open_pos: int) -> int:
    depth = 0
    i = open_pos
    n = len(text)
    while i < n:
        c = text[i]
        if c == "/" and i + 1 < n and text[i + 1] == "/":
            nl = text.find("\n", i + 2)
            i = n if nl == -1 else nl + 1
            continue
        if c == "/" and i + 1 < n and text[i + 1] == "*":
            end = text.find("*/", i + 2)
            i = n if end == -1 else end + 2
            continue
        if c == '"':
            i += 1
            while i < n:
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == '"':
                    i += 1
                    break
                i += 1
            continue
        if c == "'":
            i += 1
            while i < n:
                if text[i] == "\\":
                    i += 2
                    continue
                if text[i] == "'":
                    i += 1
                    break
                i += 1
            continue

        if c == "{":
            depth += 1
        elif c == "}":
            depth -= 1
            if depth == 0:
                return i
        i += 1
    raise ValueError(f"Unmatched '{{' starting at character {open_pos}")


def locate_top_level_blocks(text: str):
    pattern = re.compile(
        r"^(?:namespace|class|struct)\s+([A-Za-z_][A-Za-z0-9_]*)\b[^;{]*\{",
        re.MULTILINE,
    )
    blocks = []
    pos = 0
    while True:
        m = pattern.search(text, pos)
        if not m:
            break
        name = m.group(1)
        brace_open = m.end() - 1
        brace_close = find_matching_brace(text, brace_open)
        end_pos = brace_close + 1
        while end_pos < len(text) and text[end_pos] in " \t":
            end_pos += 1
        if end_pos < len(text) and text[end_pos] == ";":
            end_pos += 1
        if end_pos < len(text) and text[end_pos] == "\n":
            end_pos += 1

        blocks.append({
            "name": name,
            "start": m.start(),
            "end": end_pos,
            "code": text[m.start() : end_pos],
        })
        pos = end_pos
    return blocks


def make_free_functions_inline(code: str) -> str:
    """Only adds 'inline' to true top-level free functions outside any class/struct/namespace."""
    blocks = locate_top_level_blocks(code)
    out = []
    cursor = 0

    # Single-line or multi-line free function pattern (never matches public:/private: or classes)
    fn_pattern = re.compile(
        r"^([ \t]*)(?!inline\b|static\b|constexpr\b|class\b|struct\b|namespace\b|enum\b|template\b|typedef\b|using\b|#)"
        r"([A-Za-z_][A-Za-z0-9_:<>,\t \*&]*?\s+[A-Za-z_][A-Za-z0-9_]*\s*\([^;{}]*\)\s*\{)",
        re.MULTILINE,
    )

    for b in blocks:
        outside_chunk = code[cursor : b["start"]]
        outside_chunk = fn_pattern.sub(r"\1inline \2", outside_chunk)
        out.append(outside_chunk)
        out.append(b["code"])  # Leave class/struct/namespace bodies 100% untouched
        cursor = b["end"]

    tail_chunk = code[cursor:]
    tail_chunk = fn_pattern.sub(r"\1inline \2", tail_chunk)
    out.append(tail_chunk)
    return "".join(out)


def main():
    # Always restore from main.cpp.bak if it exists so we split a clean copy
    if BACKUP_FILE.exists():
        shutil.copy2(BACKUP_FILE, MAIN_FILE)
        print(f"✔ Restored clean {MAIN_FILE} from {BACKUP_FILE}")
    elif MAIN_FILE.exists():
        shutil.copy2(MAIN_FILE, BACKUP_FILE)
        print(f"✔ Backed up {MAIN_FILE} -> {BACKUP_FILE}")
    else:
        print("Error: main.cpp not found.")
        return

    raw = MAIN_FILE.read_text(encoding="utf-8")
    INC_DIR.mkdir(exist_ok=True)
    SRC_DIR.mkdir(exist_ok=True)

    include_lines = re.findall(r"^\s*#include\s+[<\"].*[>\"]\s*$", raw, re.MULTILINE)
    seen = set()
    unique_includes = []
    for inc in include_lines:
        s = inc.strip()
        if s not in seen and "main.moc" not in s:
            seen.add(s)
            unique_includes.append(s)

    (INC_DIR / "CommonIncludes.h").write_text(
        "#pragma once\n\n" + "\n".join(unique_includes) + "\n",
        encoding="utf-8",
    )

    main_match = re.search(r"^int\s+main\s*\(", raw, re.MULTILINE)
    if not main_match:
        raise RuntimeError("Could not find 'int main(' in main.cpp")
    main_start = main_match.start()

    body_before_main = raw[:main_start]
    main_fn_code = raw[main_start:]

    body_without_includes = re.sub(
        r"^\s*#include\s+[<\"].*[>\"]\s*\n?", "", body_before_main, flags=re.MULTILINE
    )

    blocks = locate_top_level_blocks(body_without_includes)
    split_targets = [
        b for b in blocks
        if b["name"].endswith("Tab")
        or b["name"].endswith("Window")
        or b["name"].endswith("Canvas")
        or b["name"].endswith("Dialog")
    ]

    first_split_pos = split_targets[0]["start"]

    core_code = make_free_functions_inline(body_without_includes[:first_split_pos])
    (INC_DIR / "CoreAndWidgets.h").write_text(
        '#pragma once\n#include "CommonIncludes.h"\n\n' + core_code + "\n",
        encoding="utf-8",
    )

    module_headers = ["CoreAndWidgets.h"]
    cursor = first_split_pos
    for b in split_targets:
        chunk = body_without_includes[cursor : b["end"]]
        chunk = make_free_functions_inline(chunk)
        cursor = b["end"]

        hdr_name = f"{b['name']}.h"
        prev_hdr = module_headers[-1]
        hdr_content = f'#pragma once\n#include "{prev_hdr}"\n\n{chunk}\n'
        (INC_DIR / hdr_name).write_text(hdr_content, encoding="utf-8")
        module_headers.append(hdr_name)

        cpp_path = SRC_DIR / f"{b['name']}.cpp"
        cpp_path.write_text(f'#include "{hdr_name}"\n', encoding="utf-8")

    trailing = body_without_includes[cursor:].strip()
    last_hdr = module_headers[-1]
    if trailing:
        trailing = make_free_functions_inline(trailing)
        with open(INC_DIR / last_hdr, "a", encoding="utf-8") as f:
            f.write("\n" + trailing + "\n")

    new_main = f'#include "{last_hdr}"\n\n{main_fn_code.lstrip()}'
    new_main = re.sub(r'^\s*#include\s+"main\.moc"\s*$', "", new_main, flags=re.MULTILINE)
    MAIN_FILE.write_text(new_main, encoding="utf-8")
    print("✔ Cleanly regenerated include/*.h, src/*.cpp, and main.cpp!")


if __name__ == "__main__":
    main()