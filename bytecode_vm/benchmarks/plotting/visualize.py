#!/usr/bin/env python3
# /// script
# dependencies = [
#   "matplotlib",
# ]
# ///
import csv
import matplotlib.pyplot as plt
import os


def generate_plots():
    results_path = "../results.csv"
    if not os.path.exists(results_path):
        print(f"Error: {results_path} not found.")
        return

    data = []
    with open(results_path, "r") as f:
        reader = csv.DictReader(f)
        for row in reader:
            row["n"] = int(row["n"])
            row["time_ms"] = float(row["time_ms"])
            data.append(row)

    # 1. Log graph for operations comparison
    plt.figure(figsize=(5, 3))  # Increased width from 4 to 5
    ops_names = set(row["name"] for row in data if row["type"] == "op")
    for name in sorted(ops_names):
        subset = [row for row in data if row["name"] == name]
        subset.sort(key=lambda x: x["n"])
        ns = [row["n"] for row in subset]
        ts = [row["time_ms"] for row in subset]
        plt.plot(ns, ts, marker="o", label=name)

    plt.xscale("log")
    plt.yscale("log")
    plt.xlabel("Number of Operations (Log Scale)")
    plt.ylabel("Time (ms, Log Scale)")
    plt.title("Performance Comparison of VM Operations")
    plt.legend()
    plt.grid(True, which="both", ls="-", alpha=0.5)

    # Add padding using tight layout with padding
    plt.tight_layout(pad=1.5)
    plt.savefig("ops_comparison.png", dpi=300, bbox_inches="tight")
    print("Generated ops_comparison.png")

    # 2. Graph for Fibonacci
    plt.figure(figsize=(5, 3))  # Increased width from 4 to 5
    fib_subset = [row for row in data if row["type"] == "fib"]
    fib_subset.sort(key=lambda x: x["n"])
    if fib_subset:
        plt.plot(
            [row["n"] for row in fib_subset],
            [row["time_ms"] for row in fib_subset],
            marker="o",
            color="green",
            label="Recursive Fibonacci",
        )
        plt.xlabel("n (Fibonacci index)")
        plt.ylabel("Time (ms)")
        plt.title("Recursive Fibonacci Performance")
        plt.yscale("log")
        plt.legend()
        plt.grid(True)

        # Add padding
        plt.tight_layout(pad=1.5)
        plt.savefig("fib_performance.png", dpi=300, bbox_inches="tight")
        print("Generated fib_performance.png")

    # 3. Graph for Factorial
    plt.figure(figsize=(5, 3))  # Increased width from 4 to 5
    fact_subset = [row for row in data if row["type"] == "fact"]
    fact_subset.sort(key=lambda x: x["n"])
    if fact_subset:
        plt.plot(
            [row["n"] for row in fact_subset],
            [row["time_ms"] for row in fact_subset],
            marker="o",
            color="red",
            label="Recursive Factorial",
        )
        plt.xlabel("n (Factorial input)")
        plt.ylabel("Time (ms)")
        plt.title("Recursive Factorial Performance")
        plt.legend()
        plt.grid(True)

        # Add padding
        plt.tight_layout(pad=1.5)
        plt.savefig("fact_performance.png", dpi=300, bbox_inches="tight")
        print("Generated fact_performance.png")


if __name__ == "__main__":
    generate_plots()
