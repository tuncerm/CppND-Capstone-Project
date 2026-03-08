#!/usr/bin/env python3
import argparse
import collections
import pathlib
import sys
import xml.etree.ElementTree as ET


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Summarize cppcheck XML report for CI step summary."
    )
    parser.add_argument("report", type=pathlib.Path, help="Path to cppcheck XML report")
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    if not args.report.exists():
        print(f"cppcheck report not found: {args.report}", file=sys.stderr)
        return 2

    tree = ET.parse(args.report)
    root = tree.getroot()
    errors_node = root.find("errors")
    if errors_node is None:
        print("No <errors> node found in cppcheck XML.", file=sys.stderr)
        return 3

    severity_counts: collections.Counter[str] = collections.Counter()
    id_counts: collections.Counter[str] = collections.Counter()
    total = 0

    for err in errors_node.findall("error"):
        total += 1
        severity = err.get("severity", "unknown")
        issue_id = err.get("id", "unknown")
        severity_counts[severity] += 1
        id_counts[issue_id] += 1

    lines = []
    lines.append("## Cppcheck Strict Summary")
    lines.append("")
    lines.append(f"- Total findings: `{total}`")
    lines.append("- Severity counts:")
    for sev, count in sorted(severity_counts.items()):
        lines.append(f"  - `{sev}`: `{count}`")

    lines.append("")
    lines.append("Top issue IDs:")
    top_n = 10
    for issue_id, count in id_counts.most_common(top_n):
        lines.append(f"- `{issue_id}`: `{count}`")

    print("\n".join(lines))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
