// EEG Filter - BioAmp EXG Pill (Serial Plotter version)
// https://github.com/upsidedownlabs/BioAmp-EXG-Pill
// Copyright (c) 2021 Upside Down Labs - MIT License (see LICENSE notice in README)

#define SAMPLE_RATE 256
#define BAUD_RATE 115200
#define INPUT_PIN A0

// ---- Plotter settings ----
#define PLOT_EVERY 4      // send 1 of every 4 samples -> 64 points/sec, so the plot scrolls slower
#define Y_LIMIT 100       // fixed top/bottom lines so the plotter doesn't keep rescaling
#define WARMUP_MS 2000    // skip the filter's start-up spike

void setup() {
  Serial.begin(BAUD_RATE);
}

void loop() {
  // Calculate elapsed time
  static unsigned long past = 0;
  unsigned long present = micros();
  unsigned long interval = present - past;
  past = present;

  // Run timer
  static long timer = 0;
  timer -= interval;

  if (timer < 0) {
    timer += 1000000 / SAMPLE_RATE;

    // Sample and filter EVERY time (the filter needs the full 256 Hz rate)
    float sensor_value = analogRead(INPUT_PIN);
    float signal = EEGFilter(sensor_value);

    // Envelope: a smoothed measure of how strong the signal is
    static float envelope = 0;
    envelope += (fabs(signal) - envelope) * 0.05;

    // Only print some samples so the plot is readable
    static byte count = 0;
    if (++count >= PLOT_EVERY) {
      count = 0;
      if (millis() > WARMUP_MS) {
        Serial.print("Max:");      Serial.print(Y_LIMIT);
        Serial.print(",Min:");     Serial.print(-Y_LIMIT);
        Serial.print(",EEG:");     Serial.print(signal);
        Serial.print(",Envelope:");Serial.println(envelope);
      }
    }
  }
}

// Band-Pass Butterworth IIR digital filter, generated using filter_gen.py.
// Sampling rate: 256.0 Hz, frequency: [0.5, 29.5] Hz.
// Filter is order 4, implemented as second-order sections (biquads).
float EEGFilter(float input) {
  float output = input;
  {
    static float z1, z2;
    float x = output - -0.95391350*z1 - 0.25311356*z2;
    output = 0.00735282*x + 0.01470564*z1 + 0.00735282*z2;
    z2 = z1; z1 = x;
  }
  {
    static float z1, z2;
    float x = output - -1.20596630*z1 - 0.60558332*z2;
    output = 1.00000000*x + 2.00000000*z1 + 1.00000000*z2;
    z2 = z1; z1 = x;
  }
  {
    static float z1, z2;
    float x = output - -1.97690645*z1 - 0.97706395*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1; z1 = x;
  }
  {
    static float z1, z2;
    float x = output - -1.99071687*z1 - 0.99086813*z2;
    output = 1.00000000*x + -2.00000000*z1 + 1.00000000*z2;
    z2 = z1; z1 = x;
  }
  return output;
}
