"""MQTT to InfluxDB bridge for Vigilo telemetry."""

from __future__ import annotations

import json
import logging
import os

import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import urllib3

urllib3.disable_warnings(urllib3.exceptions.InsecureRequestWarning)

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
logger = logging.getLogger("vigilo-bridge")

REQUIRED_FIELDS = ("ax", "ay", "az", "gx", "gy", "gz", "rpm")

class Bridge:
    def __init__(
            self,
            mqtt_broker: str,
            mqtt_port: int,
            mqtt_topic: str,
            influxdb_url: str,
            influxdb_token: str,
            influxdb_org: str,
            influxdb_bucket: str
    ) -> None:
        self._mqtt_broker = mqtt_broker
        self._mqtt_port = mqtt_port
        self._mqtt_topic = mqtt_topic
        self._bucket = influxdb_bucket

        self._influx_client = InfluxDBClient(
            url=influxdb_url, token=influxdb_token, org=influxdb_org, verify_ssl=False
        )
        self._write_api = self._influx_client.write_api(write_options=SYNCHRONOUS)

        self._mqtt_client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)
        self._mqtt_client.on_connect = self._on_connect
        self._mqtt_client.on_disconnect = self._on_disconnect
        self._mqtt_client.on_message = self._on_message

    def run(self) -> None:
        self._mqtt_client.connect(self._mqtt_broker, self._mqtt_port)
        try:
            self._mqtt_client.loop_forever()
        except KeyboardInterrupt:
            logger.info("Shutting down")
        finally:
            self._influx_client.close()

    def _on_connect(self, client, userdata, flags, reason_code, properties=None) -> None:
        if reason_code != 0:
            logger.error("MQTT connection failed: %s", reason_code)
            return
        logger.info("Connected to MQTT broker at %s:%s", self._mqtt_broker, self._mqtt_port)
        client.subscribe(self._mqtt_topic)
        logger.info("Subscribed to %s", self._mqtt_topic)

    def _on_disconnect(self, client, userdata, flags, reason_code, properties=None) -> None:
        logger.warning("Disconnected from MQTT broker: %s", reason_code)

    def _on_message(self, client, userdata, msg: mqtt.MQTTMessage) -> None:
        parts = msg.topic.split("/")
        if len(parts) != 3:
            logger.warning("Unexpected topic format: %s", msg.topic)
            return
        
        device_id = parts[1]

        try:
            payload = json.loads(msg.payload)
        except json.JSONDecodeError:
            logger.warning("Invalid JSON on %s: %r", msg.topic, msg.payload)
            return
        
        if not all(field in payload for field in REQUIRED_FIELDS):
            logger.warning("Missing fields on %s: %r", msg.topic, payload)
            return
        
        point = Point("telemetry").tag("device_id", device_id)
        for field in REQUIRED_FIELDS:
            point = point.field(field, payload[field])

        self._write_api.write(bucket=self._bucket, record=point)

def main() -> None:
    bridge = Bridge(
        mqtt_broker=os.environ["MQTT_BROKER"],
        mqtt_port=int(os.environ.get("MQTT_PORT", "1883")),
        mqtt_topic=os.environ.get("MQTT_TOPIC", "vigilo/+/telemetry"),
        influxdb_url=os.environ["INFLUXDB_URL"],
        influxdb_token=os.environ["INFLUXDB_TOKEN"],
        influxdb_org=os.environ["INFLUXDB_ORG"],
        influxdb_bucket=os.environ["INFLUXDB_BUCKET"],
    )
    bridge.run()


if __name__ == "__main__":
    main()
