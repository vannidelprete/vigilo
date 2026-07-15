# IMU batch sampling for FFT analysis

Design notes and calculations behind issue #66 (high-rate batched IMU sampling)
and its dependency chain to #35 and #36. Kept as a permanent reference since the
numbers here (sample rate, batch size, buffer sizes) are load-bearing: changing
the fan, the RPM range, or the harmonics of interest requires redoing this math.

## The problem

The original firmware published one accelerometer/gyroscope sample every
~100ms (~10Hz), each as its own MQTT message. This rate is far too low for
FFT-based vibration analysis.

Measured on the test bench (from `ml/data/baseline.csv`, collected in #35):

- Sample interval: ~100-110ms, confirming the ~10Hz rate implied by the
  firmware's `delay(100)` loop.
- RPM during that recording: 2550-2730 (likely inflated by PSU overvoltage
  under light load; box rating was 1600 RPM - see the RPM investigation
  earlier in the project).

By the Nyquist theorem, a signal sampled at 10Hz can only be reliably
reconstructed up to 5Hz. The fan's rotational frequency (1X RPM) is 42-45Hz at
the observed speed - about 9x higher than what 10Hz sampling can resolve. FFT
on the original baseline dataset would be meaningless: the frequency we need
to isolate isn't representable in the data at all (aliasing).

## Calculations

All calculations use the highest observed RPM (2730) for a conservative
margin, since the true operating point is uncertain (see the PSU overvoltage
discussion).

**1. Rotational frequency and harmonics**

```
f_1X = 2730 / 60 = 45.5 Hz
f_2X = 91 Hz
f_3X = 136.5 Hz
```

2X and 3X matter because Phase 5 (#27, feature engineering) explicitly needs
them, not just 1X.

**2. Sample rate candidates**

400Hz (first candidate):
```
Nyquist = 400 / 2 = 200 Hz
Margin over f_3X = 200 / 136.5 = 1.465x
```
46.5% headroom above the third harmonic. Workable but thin, especially since
the MPU6050 has no dedicated analog anti-aliasing filter - any higher-frequency
noise can fold back into the band we care about.

500Hz (chosen):
```
Nyquist = 500 / 2 = 250 Hz
Margin over f_3X = 250 / 136.5 = 1.832x
```
83% headroom. Comfortable.

**3. I2C read timing**

An MPU6050 burst read (14 bytes: 6 accel + 2 temperature + 6 gyro) over I2C
takes roughly:

```
cycles = 9 (addr+W) + 9 (register pointer) + 9 (addr+R) + 14 x 9 (14 data bytes)
       = 153 clock cycles
```

At the default I2C standard mode (100kHz, unspecified in the original
`Wire.begin()` call):
```
time = 153 x 10us/cycle = 1530us ~= 1.53ms
```

At 500Hz (2ms period), that leaves only 0.47ms (23.5%) of margin for buffer
writes and loop overhead - too tight given WiFi/MQTT activity running
concurrently on the same core.

At I2C Fast Mode (400kHz, supported by the MPU6050 per its datasheet):
```
time = 153 x 2.5us/cycle = 382.5us ~= 0.38ms
```

At 500Hz, that leaves 1.62ms (81%) of margin. Comfortable. This is why the
firmware change bumps the I2C clock, not just the sample rate.

**4. Batch size and frequency resolution**

256 samples at 500Hz:
```
batch duration = 256 / 500 = 0.512 s
FFT bin width  = 500 / 256 = 1.953 Hz/bin
```

Distance between 1X and 2X (45.5Hz apart) is about 23 bins - harmonics are
cleanly separated, nowhere near the resolution limit.

**5. MQTT payload size**

```
256 samples x 6 fields (ax, ay, az, gx, gy, gz) x 2 bytes (int16_t) = 3072 bytes
+ header (uint32_t sampleIntervalUs + uint16_t count + float rpm)  = 10 bytes
total = 3082 bytes
```

PubSubClient's default MQTT buffer is 256 bytes - about 12x too small. The
firmware change increases it via `setBufferSize()` to 4096 bytes, leaving
margin for MQTT protocol overhead and the topic string.

## Design decisions

- **Sample rate: 500Hz**, paced in firmware by measuring actual I2C read time
  each iteration and sleeping only the remainder of the target interval
  (`ImuBatchSampler`), rather than a fixed delay that would drift as read time
  varies.
- **I2C clock: 400kHz (Fast Mode)**, up from the previous unspecified default
  (100kHz). Needed to keep enough timing margin at 500Hz sampling.
- **Batch size: 256 samples**, published as one MQTT message every 30 seconds
  (`BATCH_INTERVAL_MS`). The original plan interleaved this with continuous
  ~10Hz single-sample JSON telemetry for a live dashboard view, but that JSON
  stream was dropped: the raw accelerometer/gyroscope values it carried were
  themselves subject to the same aliasing problem motivating this whole
  redesign, so they were not meaningful data at that rate. Device connectivity
  is now tracked via MQTT Last Will and Testament instead of a periodic
  heartbeat - the broker marks the device offline automatically on an
  unexpected disconnect, published as a retained message on
  `vigilo/<deviceId>/status`.
- **Binary payload, not JSON**, on a separate topic
  (`vigilo/<deviceId>/telemetry/batch`) from the regular telemetry
  (`vigilo/<deviceId>/telemetry`). JSON's per-sample overhead would meaningfully
  inflate a 256-sample payload; a packed binary struct (`#pragma pack(1)`)
  keeps it at the calculated 3082 bytes.
- **MQTT buffer: 4096 bytes**, sized with margin above the 3082-byte payload.

## Consequences for other tasks

- The baseline dataset from #35 was collected at the old ~10Hz rate and needs
  to be re-collected once #66 ships, using the batch topic instead of
  `collect_baseline.py`'s InfluxDB export.
- #36 (FFT analysis) depends on #66 and will need a new Python-side consumer
  that parses this binary batch format (a fixed 10-byte header via
  `struct.unpack("<IHf", ...)`, then `count` samples of
  `struct.unpack("<6h", ...)` each) rather than reading CSV.
- The mDNS auto-discovery work (#60) was separately deferred to Phase 4 for
  unrelated reasons (Docker Desktop on Windows cannot advertise mDNS onto the
  physical LAN) - the two deferrals are independent of each other.
