#!/usr/bin/env python3
"""Extract a YAML scalar schema from its generated Tree-sitter lexer."""

import argparse
from pathlib import Path
import re


def indent(content):
    return "\n".join("  " + line for line in content.split("\n"))


def dedent(content):
    return re.sub(r"^  ", "", content, flags=re.MULTILINE)


def block(contents):
    return "{\n" + indent("\n".join(contents)) + "\n}"


def extract_cases(source):
    signature = "static bool ts_lex(TSLexer *lexer, TSStateId state) {"
    switch = "switch (state) {\n"
    start = source.index(switch, source.index(signature)) + len(switch)
    end = source.index("}\n}", start)
    content = re.sub(r"^\s*if \(eof\).+\n", "", source[start:end], flags=re.MULTILINE)
    cases = {}
    for case in dedent(dedent(content.rstrip())).split("END_STATE();"):
        key, body = case.split(":\n", 1)
        key = key.strip().removeprefix("case ")
        cases[key] = dedent(body).strip()
    return cases


def convert_schema(source):
    cases = extract_cases(source)
    enums = ["RS_STR"]
    content = "\n  END_STATE();\n".join(
        ("default:" if key == "default" else f"case {key}:") + "\n" + indent(body)
        for key, body in cases.items()
    )

    def expand_map(match):
        return re.sub(
            r"'(.)', (\d+),",
            r"if (lookahead == '\1') ADVANCE(\2);",
            match[1],
        )

    content = re.sub(r"\s+ADVANCE_MAP\(([\s\S]+?)\);\n", expand_map, content)

    def advance(match):
        state = match[1]
        accept = re.match(r"ACCEPT_TOKEN\(\w+\);", cases[state])
        result = accept[0] if accept else "*rlt_sch = RS_STR;"
        return "{" + result + f" return {state};" + "}"

    content = re.sub(r"ADVANCE\((\d+)\);", advance, content)
    content = content.replace("ACCEPT_TOKEN(ts_builtin_sym_end);", "abort();", 1)

    def accept_token(match):
        name = "RS_" + match[1].removeprefix("sym_").upper()
        if name not in enums:
            enums.append(name)
        return f"*rlt_sch = {name};"

    content = re.sub(r"ACCEPT_TOKEN\((\w+)\);", accept_token, content)
    content = content.replace("END_STATE();", "break;")
    content = content.replace("return false;", "*rlt_sch = RS_STR;\n  return SCH_STT_FRZ;", 1)
    content = content.replace("lookahead", "cur_chr")
    switch = "switch (sch_stt) " + block(["case SCH_STT_FRZ:\n  break;", content])
    function = block([
        switch,
        "if (cur_chr != '\\r' && cur_chr != '\\n' && cur_chr != ' ' && cur_chr != 0) *rlt_sch = RS_STR;",
        "return SCH_STT_FRZ;",
    ])
    return "\n\n".join([
        "#include <stdint.h>",
        "#include <stdlib.h>",
        "#define SCH_STT_FRZ -1",
        f"#define HAS_TIMESTAMP {int('RS_TIMESTAMP' in enums)}",
        "typedef enum " + block([f"{name}," for name in enums]) + " ResultSchema;",
        "static int8_t adv_sch_stt(int8_t sch_stt, int32_t cur_chr, ResultSchema *rlt_sch) " + function,
    ]) + "\n"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("schema", choices=("core", "json", "legacy"), nargs="?", default="core")
    schema = parser.parse_args().schema
    directory = Path(__file__).resolve().parent
    source = (directory / schema / "src" / "parser.c").read_text(encoding="utf-8")
    (directory.parent / "src" / f"schema.{schema}.c").write_text(
        convert_schema(source), encoding="utf-8"
    )


if __name__ == "__main__":
    main()
