"""Tests for bridge.py's pure MQTT-payload-to-InfluxDB-point conversion."""
from __future__ import annotations

import json
import struct

import pytest

from bridge import (
    BATCH_HEADER_FORMAT,
    alert_point,
    batch_point,
    status_point,
    vibration_point,
)


def test_batch_point_encodes_header_fields():
    payload = struct.pack(BATCH_HEADER_FORMAT, 2000, 256, 1800.0)

    point = batch_point("vigilo-01", payload)

    assert point.to_line_protocol() == "telemetry,device_id=vigilo-01 rpm=1800,sample_count=256i,sample_interval_us=2000i"


def test_status_point_online():
    point = status_point("vigilo-01", b"online")
    assert point.to_line_protocol() == "status,device_id=vigilo-01 online_flag=1i"


def test_status_point_offline():
    point = status_point("vigilo-01", b"offline")
    assert point.to_line_protocol() == "status,device_id=vigilo-01 online_flag=0i"


def test_alert_point_encodes_fields():
    payload = json.dumps({"device_id": "vigilo-01", "axis": "az", "snr": 4.31, "threshold": 3.24, "rpm": 1820.1}).encode()

    point = alert_point("vigilo-01", payload)

    assert point.to_line_protocol() == "alert,axis=az,device_id=vigilo-01 rpm=1820.1,snr=4.31,threshold=3.24"


def test_alert_point_raises_on_malformed_json():
    with pytest.raises(json.JSONDecodeError):
        alert_point("vigilo-01", b"not json")


def test_alert_point_raises_on_missing_field():
    payload = json.dumps({"device_id": "vigilo-01", "axis": "az"}).encode()
    with pytest.raises(KeyError):
        alert_point("vigilo-01", payload)

def test_vibration_point_encodes_fields():
    payload = json.dumps({"device_id": "vigilo-01", "axis": "az", "snr": 2.15, "rpm": 1800.0}).encode()

    point = vibration_point("vigilo-01", payload)

    assert point.to_line_protocol() == "vibration,axis=az,device_id=vigilo-01 rpm=1800,snr=2.15"


def test_vibration_point_raises_on_missing_field():
    payload = json.dumps({"device_id": "vigilo-01", "axis": "az"}).encode()
    with pytest.raises(KeyError):
        vibration_point("vigilo-01", payload)