"""Tests for collect_baseline.py's binary batch decoding."""

from __future__ import annotations

import struct

import pytest

from collect_baseline import BATCH_HEADER_FORMAT, SAMPLE_FORMAT, decode_batch


def make_payload(sample_interval_us: int, rpm: float, samples: list[tuple[int, int, int, int, int, int]]) -> bytes:
    payload = struct.pack(BATCH_HEADER_FORMAT, sample_interval_us, len(samples), rpm)
    for s in samples:
        payload += struct.pack(SAMPLE_FORMAT, *s)
    return payload


def test_decode_batch_round_trips_header_and_samples():
    samples = [(100, -200, 16384, 5, -5, 10), (101, -201, 16385, 6, -6, 11)]
    payload = make_payload(sample_interval_us=2000, rpm=1811.2, samples=samples)

    sample_interval_us, rpm, decoded = decode_batch(payload)

    assert sample_interval_us == 2000
    assert rpm == pytest.approx(1811.2, abs=0.01)
    assert decoded == samples


def test_decode_batch_handles_empty_sample_list():
    payload = make_payload(sample_interval_us=2000, rpm=0.0, samples=[])

    _, _, decoded = decode_batch(payload)

    assert decoded == []