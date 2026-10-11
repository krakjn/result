#!/usr/bin/env python3
"""Run every example and splice its source and output into the docs.

Rewrites only the regions between capture markers. Pass --check to exit
non-zero when a rewrite would change a committed file.
"""

import argparse
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
README = ROOT / "README.md"
EXAMPLES_PAGE = ROOT / "docs" / "pages" / "examples.md"
BUILD = ROOT / "build"

REGION = re.compile(
    r"<!-- capture:(?P<name>[a-z0-9_-]+) -->\n.*?<!-- /capture:(?P=name) -->",
    re.DOTALL,
)


class Run:
    def __init__(self, source, stdout, stderr):
        self.source = source
        self.stdout = stdout
        self.stderr = stderr


def fence(lang, body):
    return "```%s\n%s\n```" % (lang, body.strip("\n"))


def render(name, runs):
    if name.endswith("-source"):
        example = name[: -len("-source")]
        return fence("cpp", runs[example].source)
    if name.endswith("-output"):
        example = name[: -len("-output")]
        return fence("text", runs[example].stdout)
    run = runs[name]
    parts = ['<div class="demo-file">%s.cpp</div>' % name, "", fence("cpp", run.source)]
    if run.stdout.strip("\n"):
        parts += ["", fence("stdout", run.stdout)]
    if run.stderr.strip("\n"):
        parts += ["", fence("stderr", run.stderr)]
    return "\n".join(parts)


def splice(text, runs):
    def replace(match):
        name = match.group("name")
        try:
            body = render(name, runs)
        except KeyError:
            raise SystemExit("no example for capture marker %s" % name)
        return "<!-- capture:%s -->\n%s\n<!-- /capture:%s -->" % (name, body, name)

    return REGION.sub(replace, text)


def executable(name):
    for candidate in (BUILD / name, BUILD / (name + ".exe")):
        if candidate.is_file():
            return candidate
    raise SystemExit("built example not found: %s" % name)


def capture(names):
    subprocess.run(["cmake", "-S", str(ROOT), "-B", str(BUILD)], check=True)
    subprocess.run(["cmake", "--build", str(BUILD), "-j"], check=True)
    runs = {}
    for name in names:
        source = (ROOT / "examples" / (name + ".cpp")).read_text()
        proc = subprocess.run(
            [str(executable(name))],
            cwd=str(ROOT),
            capture_output=True,
            text=True,
        )
        if proc.returncode != 0:
            raise SystemExit("%s exited %s" % (name, proc.returncode))
        runs[name] = Run(source, proc.stdout, proc.stderr)
    return runs


def write_or_check(path, text, check):
    current = path.read_text()
    if current == text:
        return False
    if check:
        print("%s is out of date; run: python3 docs/capture.py" % path, file=sys.stderr)
        return True
    path.write_text(text)
    return False


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--check",
        action="store_true",
        help="fail if the committed files would change",
    )
    args = parser.parse_args()

    names = sorted(path.stem for path in (ROOT / "examples").glob("*.cpp"))
    page = EXAMPLES_PAGE.read_text()
    missing = [name for name in names if "<!-- capture:%s -->" % name not in page]
    if missing:
        raise SystemExit("examples.md is missing capture markers for: %s" % ", ".join(missing))

    runs = capture(names)
    stale = write_or_check(README, splice(README.read_text(), runs), args.check)
    stale = write_or_check(EXAMPLES_PAGE, splice(page, runs), args.check) or stale
    if stale:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
