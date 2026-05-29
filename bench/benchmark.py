import subprocess
import re
import statistics
from collections import defaultdict
import os
from time import time


def run_benchmark(cmd, runs=10):
    all_metrics = defaultdict(list)
    print(f"Benchmarking: {' '.join(cmd)}")
    for i in range(runs):
        print(f"=> Running {cmd} {{run={i}}} ", end="", flush=True)
        start = time()
        process = subprocess.Popen(
            cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        stdout, stderr = process.communicate()
        print(f"(~{time()-start:.9} secs)")

        metrics_found = {
            "Lexer Time": re.search(r"Lexer Time: ([\d.]+) Seconds", stdout),
            "Compile Time": re.search(r"Compile Time: ([\d.]+) Seconds", stdout),
            "Run Time": re.search(r"Run Time: ([\d.]+) Seconds", stdout),
            "Total Program Time": re.search(r"Total Program Time: ([\d.]+) Seconds", stdout)
        }

        for name, match in metrics_found.items():
            if match:
                all_metrics[name].append(float(match.group(1)))
            else:
                print(f"Warning: Could not find {name} in output of run {i+1}")

    averages = {}
    for name, values in all_metrics.items():
        if values:
            averages[name] = statistics.mean(values)
    return averages


def generate_svg(data, filename="results.svg", runs=10):
    # Stacked bar chart + Table in SVG
    width = 1100
    height = 800
    margin_x = 80
    margin_y = 60
    chart_height = 400
    bar_width = 160
    gap = 60

    font_stack = "system-ui, -apple-system, 'Segoe UI', Roboto, Helvetica, Arial, sans-serif"

    metrics_to_stack = ["Lexer Time", "Compile Time", "Run Time"]
    colors = {
        "Lexer Time": "#ff9999",
        "Compile Time": "#66b3ff",
        "Run Time": "#99ff99"
    }

    max_val = max(d.get("Total Program Time", 0)
                  for d in data.values()) if data else 1
    max_val *= 1.1

    with open(filename, "w") as f:
        f.write(
            f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">\n')
        f.write(f'  <style>text {{ font-family: {font_stack}; }}</style>\n')
        f.write('  <rect width="100%" height="100%" fill="white" />\n')

        # --- Chart Section ---
        f.write(
            f'  <line x1="{margin_x}" y1="{chart_height}" x2="{width-margin_x}" y2="{chart_height}" stroke="black" />\n')
        f.write(
            f'  <line x1="{margin_x}" y1="{margin_y}" x2="{margin_x}" y2="{chart_height}" stroke="black" />\n')
        f.write(
            f'  <text x="{width/2}" y="{margin_y/2}" text-anchor="middle" font-size="22" font-weight="bold">Brainfuck Performance Breakdown</text>\n')
        f.write(
            f'  <text x="{width/2}" y="{margin_y/2 + 25}" text-anchor="middle" font-size="14" fill="#666">(Average of {runs} runs)</text>\n')

        # Legend
        lx = width - 180
        ly = margin_y
        for i, m_name in enumerate(metrics_to_stack):
            color = colors[m_name]
            f.write(
                f'  <rect x="{lx}" y="{ly + i*25}" width="15" height="15" fill="{color}" />\n')
            f.write(
                f'  <text x="{lx + 20}" y="{ly + i*25 + 12}" font-size="12">{m_name}</text>\n')

        f.write(
            f'  <text x="{margin_x-10}" y="{chart_height+5}" text-anchor="end" font-size="12">0s</text>\n')
        f.write(
            f'  <text x="{margin_x-10}" y="{margin_y+5}" text-anchor="end" font-size="12">{max_val:.2f}s</text>\n')

        x = margin_x + gap
        impl_names = list(data.keys())
        for impl_name in impl_names:
            metrics = data[impl_name]
            current_y = chart_height
            for m_name in metrics_to_stack:
                val = metrics.get(m_name, 0)
                bar_h = (val / max_val) * (chart_height - margin_y)
                if bar_h > 0:
                    f.write(
                        f'  <rect x="{x}" y="{current_y - bar_h}" width="{bar_width}" height="{bar_h}" fill="{colors[m_name]}" stroke="#333" stroke-width="0.5" />\n')
                    current_y -= bar_h
            f.write(
                f'  <text x="{x + bar_width/2}" y="{chart_height+20}" text-anchor="middle" font-size="13" font-weight="bold">{impl_name}</text>\n')
            total = metrics.get("Total Program Time", 0)
            f.write(
                f'  <text x="{x + bar_width/2}" y="{current_y - 10}" text-anchor="middle" font-size="12" font-weight="600">{total:.4f}s</text>\n')
            x += bar_width + gap

        # --- Table Section ---
        table_top = chart_height + 80
        col_widths = [220, 150, 150, 150, 150]
        table_x = (width - sum(col_widths)) / 2

        # Table Header
        headers = ["Implementation",
                   "Lexer (s)", "Compile (s)", "Run (s)", "Total (s)"]
        for i, header in enumerate(headers):
            tx = table_x + sum(col_widths[:i])
            f.write(
                f'  <rect x="{tx}" y="{table_top}" width="{col_widths[i]}" height="35" fill="#f8f9fa" stroke="#dee2e6" />\n')
            f.write(
                f'  <text x="{tx + col_widths[i]/2}" y="{table_top + 22}" text-anchor="middle" font-size="13" font-weight="bold" fill="#212529">{header}</text>\n')

        # Table Rows
        for row_idx, impl_name in enumerate(impl_names):
            y = table_top + 35 + (row_idx * 35)
            metrics = data[impl_name]
            row_vals = [
                impl_name,
                f"{metrics.get('Lexer Time', 0):.6f}",
                f"{metrics.get('Compile Time', 0):.6f}",
                f"{metrics.get('Run Time', 0):.6f}",
                f"{metrics.get('Total Program Time', 0):.6f}"
            ]
            for i, val in enumerate(row_vals):
                tx = table_x + sum(col_widths[:i])
                bg_color = "white" if row_idx % 2 == 0 else "#f8f9fa"
                f.write(
                    f'  <rect x="{tx}" y="{y}" width="{col_widths[i]}" height="35" fill="{bg_color}" stroke="#dee2e6" />\n')
                f.write(
                    f'  <text x="{tx + col_widths[i]/2}" y="{y + 22}" text-anchor="middle" font-size="12" fill="#495057">{val}</text>\n')

        f.write("</svg>\n")


if __name__ == "__main__":
    runs = 10
    implementations = {
        "Interpreter": ["../bin/bfc", "-metrics", "-interpret", "../examples/mandelbrot.bf"],
        "Compiler (C Backend)": ["../bin/bfc", "-metrics", "-c", "-o", "mandelbrot_c", "-run", "../examples/mandelbrot.bf"],
        "Compiler (NASM)": ["../bin/bfc", "-metrics", "-nasm", "-o", "mandelbrot_nasm", "-run", "../examples/mandelbrot.bf"],
        "JIT": ["../bin/bfc", "-metrics", "-jit", "../examples/mandelbrot.bf"]
    }
    results = {}
    for name, cmd in implementations.items():
        results[name] = run_benchmark(cmd, runs=runs)

    generate_svg(results, "results.svg", runs=runs)
    print(f"\nResults (avg of {runs} runs) saved to results.svg")

    os.remove("mandelbrot_c")
    os.remove("mandelbrot_nasm")
