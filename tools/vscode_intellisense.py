"""Generate .vscode/c_cpp_properties.json for the sketches in firmware/ and experiments/.

VS Code's C/C++ extension can't see the ESP32 core/library headers that
arduino-cli keeps under ~/Library/Arduino15 and ~/Documents/Arduino. This asks
arduino-cli for each sketch's exact compile command, expands its @response files
and -iprefix/-iwithprefixbefore includes, and writes the union as one config.

Re-run after installing a library, adding a sketch, or upgrading the core:
    python3 tools/vscode_intellisense.py
"""

import json
import pathlib
import shlex
import subprocess
import tempfile

ROOT = pathlib.Path(__file__).resolve().parent.parent
FQBN = "esp32:esp32:esp32"


def expand(args: list[str]) -> list[str]:
    """Inline @file response files (recursively)."""
    out = []
    for arg in args:
        if arg.startswith("@"):
            text = pathlib.Path(arg[1:]).read_text()
            out.extend(expand(shlex.split(text)))
        else:
            out.append(arg)
    return out


def parse(args: list[str]) -> tuple[list[str], list[str], str | None]:
    includes, defines, std = [], [], None
    prefix = ""
    it = iter(expand(args))
    for arg in it:
        if arg == "-iprefix":
            prefix = next(it)
        elif arg == "-iwithprefixbefore":
            includes.append(prefix + next(it))
        elif arg.startswith("-iwithprefixbefore"):
            includes.append(prefix + arg[len("-iwithprefixbefore"):])
        elif arg in ("-I", "-isystem"):
            includes.append(next(it))
        elif arg.startswith("-I"):
            includes.append(arg[2:])
        elif arg == "-D":
            defines.append(next(it))
        elif arg.startswith("-D"):
            defines.append(arg[2:])
        elif arg.startswith("-std="):
            std = arg[5:]
    return includes, defines, std


def sketch_command(sketch: pathlib.Path, build: pathlib.Path) -> list[str]:
    subprocess.run(
        ["arduino-cli", "compile", "-b", FQBN, "--only-compilation-database",
         "--build-path", str(build), str(sketch)],
        check=True, capture_output=True,
    )
    entries = json.loads((build / "compile_commands.json").read_text())
    return next(e["arguments"] for e in entries if e["file"].endswith(".ino.cpp"))


def main() -> None:
    inos = [*ROOT.glob("firmware/*/*.ino"), *ROOT.glob("experiments/*/*.ino")]
    sketches = sorted(p.parent for p in inos)
    includes: dict[str, None] = {}
    defines: dict[str, None] = {}
    compiler, std = None, None
    with tempfile.TemporaryDirectory() as tmp:
        for sketch in sketches:
            args = sketch_command(sketch, pathlib.Path(tmp) / sketch.name)
            compiler = args[0]
            inc, dfn, std = parse(args[1:])
            includes.update(dict.fromkeys(inc))
            defines.update(dict.fromkeys(dfn))
            print(f"{sketch.relative_to(ROOT)}: {len(inc)} include dirs")

    core = next(i for i in includes if i.endswith("/cores/esp32"))
    config = {
        "configurations": [{
            "name": "ESP32 (arduino-cli)",
            "compilerPath": compiler,
            "intelliSenseMode": "gcc-x86",
            "cStandard": "gnu17",
            "cppStandard": std or "gnu++17",
            "includePath": list(includes),
            "defines": list(defines),
            "forcedInclude": [f"{core}/Arduino.h"],
        }],
        "version": 4,
    }
    out = ROOT / ".vscode" / "c_cpp_properties.json"
    out.parent.mkdir(exist_ok=True)
    out.write_text(json.dumps(config, indent=2) + "\n")
    print(f"wrote {out.relative_to(ROOT)}: {len(includes)} include dirs, "
          f"{len(defines)} defines")


if __name__ == "__main__":
    main()
