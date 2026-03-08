#!/usr/bin/env python3
import argparse
import collections
import pathlib
import sys
import xml.etree.ElementTree as ET


def parse_max_severity(value: str) -> tuple[str, int]:
    parts = value.split("=", maxsplit=1)
    if len(parts) != 2:
        raise argparse.ArgumentTypeError(
            f"Invalid --max-severity value '{value}'. Expected format severity=count."
        )
    severity = parts[0].strip()
    if not severity:
        raise argparse.ArgumentTypeError("Severity name cannot be empty.")
    try:
        count = int(parts[1])
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            f"Invalid count in --max-severity value '{value}'."
        ) from exc
    if count < 0:
        raise argparse.ArgumentTypeError("Severity count threshold cannot be negative.")
    return severity, count


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Summarize cppcheck XML report for CI step summary."
    )
    parser.add_argument("report", type=pathlib.Path, help="Path to cppcheck XML report")
    parser.add_argument(
        "--max-severity",
        action="append",
        default=[],
        type=parse_max_severity,
        help="Maximum allowed findings for a severity (format: severity=count).",
    )
    parser.add_argument(
        "--strict-exit-code",
        type=int,
        default=None,
        help="Optional strict cppcheck process exit code to include in the summary.",
    )
    parser.add_argument(
        "--enforce",
        action="store_true",
        help="Return non-zero when configured severity thresholds are exceeded.",
    )
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

    severity_limits = dict(args.max_severity)
    violations: list[tuple[str, int, int]] = []
    for sev, limit in severity_limits.items():
        actual = severity_counts.get(sev, 0)
        if actual > limit:
            violations.append((sev, actual, limit))

    lines = []
    lines.append("## Cppcheck Strict Summary")
    lines.append("")
    if args.strict_exit_code is not None:
        lines.append(f"- Strict command exit code: `{args.strict_exit_code}`")
    lines.append(f"- Total findings: `{total}`")
    lines.append("- Severity counts:")
    for sev, count in sorted(severity_counts.items()):
        lines.append(f"  - `{sev}`: `{count}`")

    if severity_limits:
        lines.append("- Gate thresholds:")
        for sev, limit in sorted(severity_limits.items()):
            lines.append(f"  - `{sev}` <= `{limit}`")

        if violations:
            lines.append("- Gate result: `FAIL`")
            for sev, actual, limit in sorted(violations):
                lines.append(
                    f"  - Severity `{sev}` exceeded limit: `{actual}` > `{limit}`"
                )
        else:
            lines.append("- Gate result: `PASS`")

    lines.append("")
    lines.append("Top issue IDs:")
    top_n = 10
    for issue_id, count in id_counts.most_common(top_n):
        lines.append(f"- `{issue_id}`: `{count}`")

    print("\n".join(lines))
    if args.enforce and violations:
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
