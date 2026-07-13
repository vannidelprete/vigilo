"""Export a time range of telemetry data from InfluxDB to a CSV baseline dataset."""

from __future__ import annotations

import argparse
import csv
import os
from datetime import timedelta

from dotenv import load_dotenv
from influxdb_client import InfluxDBClient

FIELDS = ("ax", "ay", "az", "gx", "gy", "gz", "rpm")

QUERY = '''
from(bucket: _bucket)
  |> range(start: _start)
  |> filter(fn: (r) => r._measurement == "telemetry")
  |> filter(fn: (r) => r.device_id == _deviceId)
  |> filter(fn: (r) => contains(value: r._field, set: ["ax", "ay", "az", "gx", "gy", "gz", "rpm"]))
  |> pivot(rowKey: ["_time"], columnKey: ["_field"], valueColumn: "_value")
  |> sort(columns: ["_time"])
'''


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--minutes", type=int, default=15, help="How many minutes of history to export (default: 15).")
    parser.add_argument("--output", default="data/baseline.csv", help="Output CSV path.")
    parser.add_argument("--device-id", default="vigilo-01", help="Device ID to filter on.")
    return parser.parse_args()


def main() -> None:
    load_dotenv(os.path.join(os.path.dirname(__file__), ".env"))
    args = parse_args()

    client = InfluxDBClient(
        url=os.environ["INFLUXDB_URL"],
        token=os.environ["INFLUXDB_TOKEN"],
        org=os.environ["INFLUXDB_ORG"],
        verify_ssl=False,
    )
    query_api = client.query_api()

    params = {
        "_bucket": os.environ["INFLUXDB_BUCKET"],
        "_start": timedelta(minutes=-args.minutes),
        "_deviceId": args.device_id,
    }
    tables = query_api.query(QUERY, params=params)

    os.makedirs(os.path.dirname(args.output), exist_ok=True)
    rows = 0
    with open(args.output, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["time", *FIELDS])
        for table in tables:
            for record in table.records:
                writer.writerow([record.values.get("_time"), *[record.values.get(field) for field in FIELDS]])
                rows += 1

    print(f"Exported {rows} rows to {args.output}")
    client.close()


if __name__ == "__main__":
    main()