"""Classify batches as healthy or anomalous based on vibration amplitude at the
fan's rotational frequency (1X), calibrated against a healthy baseline."""

from __future__ import annotations

import argparse
import csv

import numpy as np

AXIS = "az"
DEFAULT_K = 2.0 # mean + 2*stdev: 100% recall / 2.5% false-positive rate on the #38 validation data

def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", default="data/fft_summary.csv", help="Healty fft_summary CSV used to calibrate the threshold.")
    parser.add_argument("--input", required=True, help="fft_summary CSV to classify.")
    parser.add_argument("--k", type=float, default=DEFAULT_K, help=f"Standard deviations above the healty mean (default {DEFAULT_K}).")
    return parser.parse_args()

def snr_at_1x(rows: list[dict]) -> np.ndarray:
    amp = np.array([float(r[f"{AXIS}_1x_amp"]) for r in rows])
    noise = np.array([float(r["noise_floor"]) for r in rows])
    return amp / noise

def compute_threshold(baseline_snr: np.ndarray, k: float) -> float:
    return float(baseline_snr.mean() + k * baseline_snr.std())

def main() -> None:
    args = parse_args()
    baseline_rows = list(csv.DictReader(open(args.baseline)))
    input_rows = list(csv.DictReader(open(args.input)))

    baseline_snr = snr_at_1x(baseline_rows)
    threshold = compute_threshold(baseline_snr, args.k)
    print(f"Calibrated on {len(baseline_rows)} healthy batches: mean={baseline_snr.mean():.2f} stdev={baseline_snr.std():.2f}")
    print(f"Threshold ({AXIS} SNR at 1X): {threshold:.2f}\n")

    input_snr = snr_at_1x(input_rows)
    anomaly_count = 0
    for row, snr in zip(input_rows, input_snr):
        verdict = "ANOMALY" if snr > threshold else "ok"
        anomaly_count += verdict == "ANOMALY"
        print(f"batch {row['batch_id']:>3}  rpm={float(row['rpm']):.1f}  snr={snr:.2f}  {verdict}")

    print(f"\n{anomaly_count}/{len(input_rows)} batches flagged anomalous ({anomaly_count/len(input_rows)*100:.1f}%)")


if __name__ == "__main__":
    main()