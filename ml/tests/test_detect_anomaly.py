"""Tests for detect_anomaly.py's SNR and threshold logic."""
from __future__ import annotations

import numpy as np
import pytest

from detect_anomaly import AXIS, compute_threshold, snr_at_1x


def make_row(amp: float, noise: float) -> dict:
    return {f"{AXIS}_1x_amp": str(amp), "noise_floor": str(noise)}


def test_snr_at_1x_computes_ratio():
    rows = [make_row(amp=20.0, noise=10.0), make_row(amp=5.0, noise=5.0)]

    result = snr_at_1x(rows)

    assert result.tolist() == pytest.approx([2.0, 1.0])


def test_compute_threshold_is_mean_plus_k_stdev():
    baseline_snr = np.array([1.0, 2.0, 3.0])

    threshold = compute_threshold(baseline_snr, k=2.0)

    assert threshold == pytest.approx(baseline_snr.mean() + 2 * baseline_snr.std())


def test_threshold_separates_the_38_validation_data():
    healthy = np.array([1.5, 2.0, 1.8, 2.2, 1.9])
    fault = np.array([6.0, 6.5, 5.8])

    threshold = compute_threshold(healthy, k=2.0)

    assert np.all(fault > threshold)
    assert not np.any(healthy > threshold)