import json
import os
import re
import sys

import matplotlib.pyplot as plt
import numpy as np


def save_figure(figure, path):
    try:
        figure.savefig(path, dpi=300, bbox_inches="tight")
        return path
    except PermissionError:
        root, ext = os.path.splitext(path)
        fallback_path = f"{root}_updated{ext}"
        figure.savefig(fallback_path, dpi=300, bbox_inches="tight")
        return fallback_path


def parse_bench_name(name):
    match = re.match(r"([^/]+)/(\d+)", name)
    if not match:
        return None, None
    return match.group(1), int(match.group(2))


def classify_complexity(operation_name):
    if any(token in operation_name for token in ("Find", "Insert", "Delete")):
        return "O(log n)", lambda n: np.log2(n)
    if "Build" in operation_name:
        return "O(n log n)", lambda n: n * np.log2(n)
    return "Unknown", lambda n: n


def fit_scale(measured, basis):
    denominator = np.dot(basis, basis)
    if denominator == 0:
        return 0.0
    return float(np.dot(measured, basis) / denominator)


def make_comparison_basis(operation_name, n_vals):
    if any(token in operation_name for token in ("Find", "Insert", "Delete")):
        return {
            "O(1)": np.ones_like(n_vals, dtype=float),
            "O(log n)": np.log2(n_vals),
            "O(n)": n_vals.astype(float),
        }

    return {
        "O(n)": n_vals.astype(float),
        "O(n log n)": n_vals * np.log2(n_vals),
        "O(n^2)": n_vals.astype(float) ** 2,
    }


def choose_xscale(operation_name):
    if "Build" in operation_name:
        return "log"
    return "log"


if len(sys.argv) < 3:
    print("Usage: python3 graphics.py <results.json> <output_dir>")
    sys.exit(1)

json_path = sys.argv[1]
output_dir = sys.argv[2]
os.makedirs(output_dir, exist_ok=True)

with open(json_path, "r", encoding="utf-8") as file:
    data = json.load(file)

benchmarks = data.get("benchmarks", [])
operations = {}

for bench in benchmarks:
    name = bench.get("name")
    real_time = bench.get("real_time")
    time_unit = bench.get("time_unit", "ns")

    if not name or real_time is None:
        continue

    operation_name, n_value = parse_bench_name(name)
    if operation_name is None:
        continue

    operations.setdefault(operation_name, []).append(
        {
            "n": n_value,
            "time": float(real_time),
            "time_unit": time_unit,
        }
    )


for op_name, results in sorted(operations.items()):
    results.sort(key=lambda item: item["n"])

    n_vals = np.array([item["n"] for item in results], dtype=float)
    times = np.array([item["time"] for item in results], dtype=float)
    time_unit = results[0]["time_unit"]

    expected_label, expected_basis_builder = classify_complexity(op_name)
    expected_basis = expected_basis_builder(n_vals)
    expected_scale = fit_scale(times, expected_basis)
    expected_curve = expected_scale * expected_basis
    normalized_expected = times / expected_basis

    comparison_basis = make_comparison_basis(op_name, n_vals)

    plt.style.use("seaborn-v0_8-whitegrid")
    figure, axes = plt.subplots(
        2,
        1,
        figsize=(12, 9),
        gridspec_kw={"height_ratios": [2.2, 1.2]},
    )

    main_ax, normalized_ax = axes

    main_ax.plot(
        n_vals,
        times,
        "o-",
        color="#1f77b4",
        linewidth=2.5,
        markersize=7,
        label="Measured time",
    )

    for label, basis in comparison_basis.items():
        scale = fit_scale(times, basis)
        curve = scale * basis
        is_expected = label == expected_label
        main_ax.plot(
            n_vals,
            curve,
            "--" if is_expected else ":",
            linewidth=2.5 if is_expected else 1.8,
            alpha=0.95 if is_expected else 0.75,
            label=f"{label} fit",
        )

    main_ax.set_xscale(choose_xscale(op_name))
    main_ax.set_ylabel(f"Time ({time_unit})")
    main_ax.set_title(f"{op_name}: measured time vs fitted asymptotic curves")
    main_ax.legend()
    main_ax.grid(True, which="both", linestyle="--", alpha=0.45)

    normalized_ax.plot(
        n_vals,
        normalized_expected,
        "o-",
        color="#d62728",
        linewidth=2.3,
        markersize=6,
        label=f"T(n) / {expected_label}",
    )
    normalized_ax.axhline(
        float(np.mean(normalized_expected)),
        color="#2ca02c",
        linestyle="--",
        linewidth=1.8,
        label="Mean level",
    )

    normalized_ax.set_xscale(choose_xscale(op_name))
    normalized_ax.set_xlabel("Number of elements (n)")
    normalized_ax.set_ylabel(f"Normalized time ({time_unit})")
    normalized_ax.set_title(f"{op_name}: normalization by expected complexity")
    normalized_ax.legend()
    normalized_ax.grid(True, which="both", linestyle="--", alpha=0.45)

    figure.suptitle(f"Asymptotic analysis for {op_name}\nExpected complexity: {expected_label}", fontsize=14)
    figure.tight_layout()

    overview_path = os.path.join(output_dir, f"{op_name.lower()}_asymptotic_linear.png")
    saved_overview_path = save_figure(figure, overview_path)
    plt.close(figure)

    print(f"Saved plot for {op_name}: {saved_overview_path}")


print(f"Done: created {len(operations)} plots.")
