"""Tests for analyze_fft.py's pure signal-processing functions."""

from __future__ import annotations

import numpy as np
import pytest

from analyze_fft import find_global_peak, find_peak_near, spectrum

def make_sine(freq_hz: float, amplitude: float, fs_hz: float, n_samples: int, dc_offset: float = 0.0) -> list[int]:
    t = np.arange(n_samples) / fs_hz
    signal= dc_offset + amplitude * np.sin(2 * np.pi * freq_hz * t)
    return signal.astype(int).tolist()

def test_spectrum_recovers_known_frequency_and_amplitude():
    fs_hz = 500.0
    n_samples = 256
    bin_hz = fs_hz / n_samples
    true_freq = 20 * bin_hz # exact bin center, avoids spectral leakage in the test
    true_amp = 1000.0

    values = make_sine(true_freq, true_amp, fs_hz, n_samples, dc_offset=16384)
    freqs, magnitude = spectrum(values, fs_hz)

    peak_idx = int(np.argmax(magnitude))
    assert freqs[peak_idx] == pytest.approx(true_freq, abs=bin_hz / 2)
    assert magnitude[peak_idx] == pytest.approx(true_amp, rel=0.05)

def test_spectrum_removes_dc_offset():
    values = make_sine(50.0, 100.0, 500.0, 256, dc_offset=20000)
    freqs, magnitude = spectrum(values, 500.0)
    assert magnitude[0] < 5.0  # negligible vs. the 20000 offset and the true amplitude of 100


def test_find_peak_near_finds_target_within_window():
    freqs = np.array([0.0, 10.0, 20.0, 30.0, 40.0, 50.0])
    magnitude = np.array([1.0, 1.0, 1.0, 50.0, 1.0, 1.0])

    hz, amp = find_peak_near(freqs, magnitude, target_hz=32.0, search_bins=1)

    assert hz == 30.0
    assert amp == 50.0


def test_find_global_peak_ignores_near_dc_content():
    freqs = np.array([0.0, 1.0, 2.0, 30.0, 40.0])
    magnitude = np.array([500.0, 400.0, 300.0, 50.0, 10.0])

    hz, amp = find_global_peak(freqs, magnitude, min_hz=5.0)

    assert hz == 30.0
    assert amp == 50.0