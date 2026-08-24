"""Live anomaly detector: subscribes to batch telemetry, computes vibration
SNR at the fan's rotational frequency (1X) per batch, and publishes an alert
when it crosses a threshold calibrated from a healthy baseline."""

from __future__ import annotations

import argparse
import csv
import json
import os

import paho.mqtt.client as mqtt
from dotenv import load_dotenv

from analyze_fft import find_peak_near, noise_floor, spectrum
from collect_baseline import BATCH_HEADER_SIZE, decode_batch
from detect_anomaly import AXIS, compute_threshold, snr_at_1x

AXIS_INDEX = {"ax": 0, "ay": 1, "az": 2}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--baseline", default="data/fft_summary.csv", help="Healthy fft_summary CSV used to calibrate the threshold.")
    parser.add_argument("--device-id", default="vigilo-01", help="Device ID to subscribe to and alert for.")
    parser.add_argument("--k", type=float, default=2.0, help="Standard deviations above the healthy mean (default 2.0).")
    return parser.parse_args()


def batch_snr(samples: list[tuple[int, int, int, int, int]], fs_hz: float, rpm: float) -> float:
    values = [s[AXIS_INDEX[AXIS]] for s in samples]
    freqs, magnitude = spectrum(values, fs_hz)
    _, amp = find_peak_near(freqs, magnitude, rpm / 60)
    return amp / noise_floor(magnitude)


def main() -> None:
    load_dotenv(os.path.join(os.path.dirname(__file__), ".env"))
    args = parse_args()

    with open(args.baseline) as f:
        baseline_rows = list(csv.DictReader(f))
    threshold = compute_threshold(snr_at_1x(baseline_rows), args.k)
    print(f"Calibrated threshold ({AXIS} SNR at 1X): {threshold:.2f}")

    batch_topic = f"vigilo/{args.device_id}/telemetry/batch"
    alert_topic = f"vigilo/{args.device_id}/alert"

    def on_connect(client: mqtt.Client, userdata, flags, reason_code, properties=None) -> None:
        if reason_code != 0:
            print(f"MQTT connection failed: {reason_code}")
            return
        print(f"Connected. Subscribed to {batch_topic}, alerting on {alert_topic}")
        client.subscribe(batch_topic)

    def on_message(client: mqtt.Client, userdata, msg: mqtt.MQTTMessage) -> None:
        if len(msg.payload) < BATCH_HEADER_SIZE:
            return
        sample_interval_us, rpm, samples = decode_batch(msg.payload)
        if not samples:
            return

        fs_hz = 1_000_000 / sample_interval_us
        snr = batch_snr(samples, fs_hz, rpm)
        verdict = "ANOMALY" if snr > threshold else "ok"
        print(f"rpm={rpm:.1f}  snr={snr:.2f}  {verdict}")

        if snr > threshold:
            alert = json.dumps({
                "device_id": args.device_id,
                "axis": AXIS,
                "snr": round(snr, 2),
                "threshold": round(threshold, 2),
                "rpm": round(rpm, 1),
            })
            client.publish(alert_topic, alert)

    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(os.environ["MQTT_BROKER"], int(os.environ.get("MQTT_PORT", "1883")))

    try:
        client.loop_forever()
    except KeyboardInterrupt:
        pass


if __name__ == "__main__":
    main()