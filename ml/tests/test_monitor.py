"""Tests for monitor.py's per-batch SNR computation."""
from __future__ import annotations

import numpy as np
from helpers import make_sine

from monitor import batch_snr


def test_batch_snr_high_for_strong_signal_at_predicted_1x():
    fs_hz = 500.0
    n_samples = 256
    bin_hz = fs_hz / n_samples
    rpm = 60 * 20 * bin_hz  # predicted 1X lands exactly on a bin center
    az_values = make_sine(20 * bin_hz, 1000.0, fs_hz, n_samples, dc_offset=16384)
    samples = [(0, 0, v, 0, 0, 0) for v in az_values]

    snr = batch_snr(samples, fs_hz, rpm)

    assert snr > 10


def test_batch_snr_low_for_flat_signal():
    rng = np.random.default_rng(42)
    samples = [(0, 0, int(16384 + rng.normal(0, 5)), 0, 0, 0) for _ in range(256)]

    snr = batch_snr(samples, 500.0, rpm=1800.0)

    assert snr < 3