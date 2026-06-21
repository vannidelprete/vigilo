# Vigilo

> Edge predictive maintenance, zero cloud.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Status](https://img.shields.io/badge/status-in%20development-orange)]()
[![Platform](https://img.shields.io/badge/platform-ESP32%20%2B%20RPi%204-blue)]()

Vigilo is an open source **edge AI predictive maintenance platform** for rotating machines - motors, pumps, fans, compressors, actuators. The entire pipeline runs on the local network: from data acquisition to anomaly detection, from historical dashboards to alert generation. No cloud, no subscription, no data leaving the site.

## Architecture

<p align="center">
  <img src="docs/architecture.svg" alt="Vigilo Architecture" width="100%">
</p>

| Component | Device | Role |
|---|---|---|
| **Sensor Node** | ESP32 + MPU6050 | Vibration acquisition (I2C) and RPM (interrupt), MQTT publishing |
| **Gateway** | Raspberry Pi 4 · Yocto Linux | MQTT broker, anomaly detection, bridge → InfluxDB, alerts |
| **Dashboard** | Grafana + InfluxDB (Docker) | Real-time and historical visualization, alert panel |

## Tech stack

| Layer | Technology |
|---|---|
| Firmware | C/C++, PlatformIO |
| Protocol | MQTT (PubSubClient / Mosquitto) |
| Gateway OS | Yocto Linux (Poky + meta-raspberrypi + meta-vigilo) |
| Data bridge | Python, paho-mqtt |
| Storage | InfluxDB |
| Dashboard | Grafana |
| Containerization | Docker Compose |
| ML (phase 5) | TensorFlow Lite Micro |

## Roadmap

| Phase | Description | Status |
|---|---|---|
| 0 | Scope, naming, repository structure | ✅ |
| 1 | Domain study — vibration and mechanical fault theory | ✅ |
| 2 | End-to-end data pipeline (sensor → MQTT → dashboard) | 🔄 |
| 3 | Baseline anomaly detection (FFT, statistical thresholds) | ⬜ |
| 4 | Custom Linux gateway (Yocto/Buildroot, Pi4 image) | ⬜ |
| 5 | Edge ML model (TensorFlow Lite Micro) | ⬜ |
| 6 | Packaging, documentation, demo | ⬜ |

## Hardware

| Component | Specs |
|---|---|
| Microcontroller | ESP32 DevKit v1 |
| Accelerometer | MPU6050 (I2C) |
| Test bench | 3-pin PC fan (tachometer on pin 3) |
| Tachometer pull-up | 10 kΩ resistor to 3.3V |
| Gateway | Raspberry Pi 4 (2 GB+), 5.1V/3A USB-C PSU |
| Gateway storage | MicroSD 16-32 GB, class A1/A2 |

> The test bench is a PC fan with a simulated mechanical imbalance. Vigilo is designed for any rotating machine with a sensor mounting point.

## Repository structure

```
vigilo/
  firmware/      # ESP32 code (PlatformIO, C/C++)
  dashboard/     # Docker Compose + Python MQTT→InfluxDB bridge
  ml/            # training scripts, exported models
  docs/          # architecture, theory, devlog
```

The custom Yocto layer for the gateway lives in a separate repository: [meta-vigilo](https://github.com/vannidelprete/meta-vigilo).

## License

[MIT](LICENSE)
