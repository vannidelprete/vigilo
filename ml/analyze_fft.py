"""Compute FFT-based vibration features from baseline batch data.

For each batch in the baseline CSV, computes the frequency spectrum of each
accelerometer axis, reports the actual dominant peak in the spectrum, and
extracts the amplitude near 1X/2X/3X the fan's rotational frequency (derived
from that batch's own measured RPM - see the RPM baseline variability
project memory).

Reports both because they do not always agree: on the first real dataset
analyzed, the true dominant peak sat several bins away from RPM/60, which a
narrow RPM-targeted search alone would have silently missed. Compare
{axis}_peak_hz against {axis}_1x_hz per batch before trusting the
RPM-targeted amplitude as "the" 1X feature.

Validates #66's whole premise: at the old ~10Hz sampling rate this signal
was aliased and unrecoverable; at 500Hz it should now show up as a clear
peak above the noise floor - which axis and at what frequency is exactly
what this script is for finding out.
"""

from __future__ import annotations

import argparse
import csv
from collections import defaultdict

import matplotlib.pyplot as plt
import numpy as np

AXES = ("ax", "ay", "az")
HARMONICS = (1, 2, 3)
MIN_PEAK_HZ = 5.0  # ignore near-DC bins when searching for the dominant peak


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input", default="data/baseline.csv", help="Baseline CSV produced by collect_baseline.py.")
    parser.add_argument("--output", default="data/fft_summary.csv", help="Output CSV with per-batch spectral features.")
    parser.add_argument("--plot-batch", type=int, default=0, help="Batch id to render a spectrum plot for (default: 0). Pass -1 to skip plotting.")
    parser.add_argument("--plot-output", default="data/fft_spectrum.png", help="Path for the spectrum plot.")
    return parser.parse_args()


def load_batches(path: str) -> dict[int, dict]:
    batches: dict[int, dict] = defaultdict(lambda: {"rpm": None, "sample_interval_us": None, "ax": [], "ay": [], "az": []})
    with open(path, newline="") as f:
        for row in csv.DictReader(f):
            b = batches[int(row["batch_id"])]
            b["rpm"] = float(row["rpm"])
            b["sample_interval_us"] = int(row["sample_interval_us"])
            for axis in AXES:
                b[axis].append(int(row[axis]))
    return batches


def spectrum(values: list[int], fs_hz: float) -> tuple[np.ndarray, np.ndarray]:
    signal = np.asarray(values, dtype=np.float64)
    signal -= signal.mean()  # remove DC (gravity offset on az, any constant tilt on ax/ay)

    window = np.hanning(len(signal))
    windowed = signal * window

    fft = np.fft.rfft(windowed)
    freqs = np.fft.rfftfreq(len(signal), d=1 / fs_hz)

    # Amplitude-correct, single-sided spectrum: divide by the window's
    # coherent gain (its sum) to undo the energy the window tapered away,
    # then double every bin except DC and Nyquist since rfft only returns
    # the positive half of a real signal's spectrum.
    magnitude = np.abs(fft) / window.sum()
    magnitude[1:-1] *= 2
    return freqs, magnitude


def find_peak_near(freqs: np.ndarray, magnitude: np.ndarray, target_hz: float, search_bins: int = 1) -> tuple[float, float]:
    center = int(np.argmin(np.abs(freqs - target_hz)))
    lo, hi = max(0, center - search_bins), min(len(freqs), center + search_bins + 1)
    peak = lo + int(np.argmax(magnitude[lo:hi]))
    return freqs[peak], magnitude[peak]


def find_global_peak(freqs: np.ndarray, magnitude: np.ndarray, min_hz: float = MIN_PEAK_HZ) -> tuple[float, float]:
    mask = freqs >= min_hz
    idx = int(np.argmax(magnitude[mask]))
    return freqs[mask][idx], magnitude[mask][idx]


def plot_spectrum(batch: dict, fs_hz: float, f_1x: float, output_path: str) -> None:
    fig, axs = plt.subplots(len(AXES), 1, figsize=(10, 8), sharex=True)
    for ax_plot, axis in zip(axs, AXES):
        freqs, magnitude = spectrum(batch[axis], fs_hz)
        ax_plot.plot(freqs, magnitude)
        for h, color in zip(HARMONICS, ("r", "g", "orange")):
            ax_plot.axvline(f_1x * h, color=color, linestyle="--", alpha=0.6, label=f"{h}X ({f_1x*h:.1f} Hz)")
        peak_hz, peak_amp = find_global_peak(freqs, magnitude)
        ax_plot.axvline(peak_hz, color="purple", linestyle=":", alpha=0.8, label=f"actual peak ({peak_hz:.1f} Hz)")
        ax_plot.set_ylabel(f"{axis} amplitude")
        ax_plot.legend(loc="upper right", fontsize=8)
    axs[-1].set_xlabel("Frequency (Hz)")
    fig.suptitle(f"Batch spectrum - RPM={batch['rpm']:.1f}, 1X={f_1x:.2f} Hz")
    fig.tight_layout()
    fig.savefig(output_path, dpi=120)
    print(f"Saved spectrum plot to {output_path}")


def main() -> None:
    args = parse_args()
    batches = load_batches(args.input)

    fieldnames = ["batch_id", "rpm", "noise_floor"]
    for axis in AXES:
        fieldnames += [f"{axis}_peak_hz", f"{axis}_peak_amp"]
        for h in HARMONICS:
            fieldnames += [f"{axis}_{h}x_hz", f"{axis}_{h}x_amp"]

    with open(args.output, "w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()

        for batch_id in sorted(batches):
            b = batches[batch_id]
            fs_hz = 1_000_000 / b["sample_interval_us"]
            f_1x = b["rpm"] / 60

            row = {"batch_id": batch_id, "rpm": b["rpm"]}
            noise_floors = []
            for axis in AXES:
                freqs, magnitude = spectrum(b[axis], fs_hz)
                noise_floors.append(float(np.median(magnitude[1:])))  # exclude DC bin

                peak_hz, peak_amp = find_global_peak(freqs, magnitude)
                row[f"{axis}_peak_hz"] = round(peak_hz, 2)
                row[f"{axis}_peak_amp"] = round(peak_amp, 2)

                for h in HARMONICS:
                    hz, amp = find_peak_near(freqs, magnitude, f_1x * h)
                    row[f"{axis}_{h}x_hz"] = round(hz, 2)
                    row[f"{axis}_{h}x_amp"] = round(amp, 2)
            row["noise_floor"] = round(sum(noise_floors) / len(noise_floors), 2)
            writer.writerow(row)

            if batch_id == args.plot_batch:
                plot_spectrum(b, fs_hz, f_1x, args.plot_output)

    print(f"Wrote spectral features for {len(batches)} batches to {args.output}")


if __name__ == "__main__":
    main()