"""Collect a baseline dataset from high-rate IMU batches published over MQTT."""

from __future__ import annotations

import argparse
import csv
import os
import struct
from datetime import datetime, timedelta, timezone

import paho.mqtt.client as mqtt
from dotenv import load_dotenv

BATCH_HEADER_FORMAT = "<IHf"
BATCH_HEADER_SIZE = struct.calcsize(BATCH_HEADER_FORMAT)
SAMPLE_FORMAT = "<6h"
SAMPLE_SIZE = struct.calcsize(SAMPLE_FORMAT)
EXPECTED_SAMPLE_COUNT = 256  # must match ImuBatchSampler::BATCH_SIZE
EXPECTED_BATCH_INTERVAL_S = 30  # must match config::BATCH_INTERVAL_MS
GAP_WARNING_RATIO = 1.5  # warn if the gap since the last batch exceeds this multiple of the expected interval

FIELDNAMES = ("batch_id", "time", "ax", "ay", "az", "gx", "gy", "gz", "rpm", "sample_interval_us")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--batches", type=int, default=20,
        help="Number of batches to collect (default: 20, ~10 minutes at the default 30s batch interval)."
    )
    parser.add_argument("--output", default="data/baseline.csv", help="Output CSV path.")
    parser.add_argument("--device-id", default="vigilo-01", help="Device ID to subscribe to.")
    return parser.parse_args()


def decode_batch(payload: bytes) -> tuple[int, float, list[tuple[int, int, int, int, int, int]]]:
    sample_interval_us, sample_count, rpm = struct.unpack(BATCH_HEADER_FORMAT, payload[:BATCH_HEADER_SIZE])
    offset = BATCH_HEADER_SIZE
    samples = []
    for _ in range(sample_count):
        samples.append(struct.unpack(SAMPLE_FORMAT, payload[offset:offset + SAMPLE_SIZE]))
        offset += SAMPLE_SIZE
    return sample_interval_us, rpm, samples


def main() -> None:
    load_dotenv(os.path.join(os.path.dirname(__file__), ".env"))
    args = parse_args()

    topic = f"vigilo/{args.device_id}/telemetry/batch"
    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    csv_file = open(args.output, "w", newline="")
    writer = csv.writer(csv_file)
    writer.writerow(FIELDNAMES)

    state = {
        "batches_received": 0,
        "rows_written": 0,
        "partial_batches": 0,
        "likely_dropped": 0,
        "last_batch_at": None,
    }

    def on_connect(client: mqtt.Client, userdata, flags, reason_code, properties=None) -> None:
        if reason_code != 0:
            print(f"MQTT connection failed: {reason_code}")
            return
        print(f"Connected. Subscribed to {topic}")
        client.subscribe(topic)

    def on_message(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage) -> None:
        if len(msg.payload) < BATCH_HEADER_SIZE:
            print(f"Batch payload too short: {len(msg.payload)} bytes, skipping")
            return

        received_at = datetime.now(timezone.utc)
        if state["last_batch_at"] is not None:
            gap_s = (received_at - state["last_batch_at"]).total_seconds()
            if gap_s > EXPECTED_BATCH_INTERVAL_S * GAP_WARNING_RATIO:
                print(f"WARNING: {gap_s:.1f}s since last batch (expected ~{EXPECTED_BATCH_INTERVAL_S}s) - a batch was likely dropped (QoS 0, no delivery guarantee)")
                state["likely_dropped"] += 1
        state["last_batch_at"] = received_at

        sample_interval_us, rpm, samples = decode_batch(msg.payload)
        if len(samples) < EXPECTED_SAMPLE_COUNT:
            print(f"WARNING: batch has only {len(samples)}/{EXPECTED_SAMPLE_COUNT} samples - IMU read likely failed partway through")
            state["partial_batches"] += 1

        batch_id = state["batches_received"]
        for i, (ax, ay, az, gx, gy, gz) in enumerate(samples):
            offset_us = (len(samples) - 1 - i) * sample_interval_us
            sample_time = received_at - timedelta(microseconds=offset_us)
            writer.writerow([batch_id, sample_time.isoformat(), ax, ay, az, gx, gy, gz, rpm, sample_interval_us])
            state["rows_written"] += 1
        csv_file.flush()

        state["batches_received"] += 1
        print(f"Batch {batch_id}: {len(samples)} samples, rpm={rpm:.1f} ({state['batches_received']}/{args.batches})")

        if state["batches_received"] >= args.batches:
            client.disconnect()

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(os.environ["MQTT_BROKER"], int(os.environ.get("MQTT_PORT", "1883")))

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        pass
    finally:
        csv_file.close()

    print(f"Collected {state['batches_received']} batches, {state['rows_written']} rows, written to {args.output}")
    print(f"Partial batches: {state['partial_batches']}, likely dropped: {state['likely_dropped']}")
    if state["partial_batches"] or state["likely_dropped"]:
        print("Some batches were partial or likely dropped - review before trusting this as a clean baseline.")


if __name__ == "__main__":
    main()