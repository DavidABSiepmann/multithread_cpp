#!/usr/bin/env python3
"""Generate separate plots for profile timeline and benchmark results.

Usage examples:
  # both default files -> saves assets/profile.png and assets/results.png
  python3 bench/grafico.py

  # specify files
  python3 bench/grafico.py --profile inputs/profile.csv --results inputs/results.csv --out-profile assets/profile.png --out-results assets/results.png

"""

import argparse
import os
import pandas as pd
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description='Gerar gráficos a partir dos CSVs de bench e profile')
parser.add_argument('--profile', dest='profile_csv', default='outputs/profile_events.csv', help='CSV de eventos de profile (thread,time,status)')
parser.add_argument('--results', dest='results_csv', default='outputs/results.csv', help='CSV de resultados (throughput)')
parser.add_argument('--out-profile', dest='out_profile', default='assets/profile.png', help='PNG de saída para timeline de profile')
parser.add_argument('--out-results', dest='out_results', default='assets/results.png', help='PNG de saída para resultados (throughput)')
parser.add_argument('--profile-only', dest='profile_only', action='store_true', help='Gerar somente o gráfico de profile')
parser.add_argument('--results-only', dest='results_only', action='store_true', help='Gerar somente o gráfico de resultados')
args = parser.parse_args()

prof_file = args.profile_csv
res_file = args.results_csv
out_profile = args.out_profile
out_results = args.out_results

# Validate input files
if not args.results_only:
    if not os.path.exists(prof_file):
        raise SystemExit(f"Profile file not found: {prof_file}")
if not args.profile_only:
    if not os.path.exists(res_file):
        raise SystemExit(f"Results file not found: {res_file}")

# Helper: ensure dir exists
def ensure_dir(path):
    d = os.path.dirname(path)
    if d:
        os.makedirs(d, exist_ok=True)

# --------- Profile timeline plot ---------
if not args.results_only:
    df = pd.read_csv(prof_file)
    # Ensure numeric time and status
    df['time'] = pd.to_numeric(df['time'], errors='coerce')
    df['status'] = pd.to_numeric(df['status'], errors='coerce')

    # Downsample if very large
    max_points = 500
    if len(df) > max_points:
        step = max(1, len(df) // max_points)
        df = df.iloc[::step].reset_index(drop=True)
        print(f"Profile events downsampled to {len(df)} points (step={step})")

    threads = sorted(df['thread'].unique())

    fig, ax = plt.subplots(figsize=(12, max(3, len(threads))))

    min_t = df['time'].min()
    max_t = df['time'].max()
    for i, th in enumerate(threads):
        sub = df[df['thread'] == th]
        if sub.empty:
            continue
        y = (i + 1)
        times = sub['time'].values
        status = sub['status'].values
        ax.step(times, status + (y - 1) * 2, where='post', label=th)

    ax.set_ylim(0, max(1 + (len(threads) - 1) * 2, 3))
    ax.set_xlim(min_t, max_t)
    ax.set_ylabel('event / thread')
    ax.set_title('Profile timeline (events)')
    ax.legend(bbox_to_anchor=(1.02, 1), loc='upper left')
    plt.tight_layout()

    ensure_dir(out_profile)
    plt.savefig(out_profile)
    print(f'Profile timeline salvo em: {out_profile}')
    plt.close(fig)

# --------- Results throughput plot ---------
if not args.profile_only:
    r = pd.read_csv(res_file)
    # Aggregate throughput: mean throughput per threads for each work_us
    pivot = r.groupby(['threads', 'work_us'])['throughput_items_s'].mean().reset_index()

    fig, ax = plt.subplots(figsize=(10,6))
    for work_us in sorted(pivot['work_us'].unique()):
        sub = pivot[pivot['work_us'] == work_us]
        ax.plot(sub['threads'], sub['throughput_items_s'], marker='o', label=f'work_us={work_us}')

    ax.set_xlabel('threads')
    ax.set_ylabel('throughput (items/s)')
    ax.set_title('Throughput médio por threads (por work_us)')
    ax.grid(True)
    ax.legend()
    plt.tight_layout()

    ensure_dir(out_results)
    plt.savefig(out_results)
    print(f'Results plot salvo em: {out_results}')
    plt.close(fig)

