"""Shared test helpers."""
import numpy as np


def make_sine(freq_hz: float, amplitude: float, fs_hz: float, n_samples: int, dc_offset: float = 0.0) -> list[int]:
    t = np.arange(n_samples) / fs_hz
    signal = dc_offset + amplitude * np.sin(2 * np.pi * freq_hz * t)
    return signal.astype(int).tolist()