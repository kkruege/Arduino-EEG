// EEG Band Amplitudes - BioAmp + Arduino UNO
// Shows the amplitude of Delta, Theta, Alpha and Beta activity in the Serial Plotter,
// and feeds the 3D brain visualization (index.html).
//
// Modified from Upside Down Labs' 02-eeg-filter.ino:
// https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware/blob/main/02-eeg-filter/02-eeg-filter.ino
//
// Changes from the original:
//  - Keeps the original 256 Hz sampling timer and biquad (second-order section) filter structure
//  - Replaces the single 0.5-29.5 Hz band-pass with four band-pass filters
//    (Delta 0.5-4, Theta 4-8, Alpha 8-13, Beta 13-30 Hz), designed with SciPy for 256 Hz
//  - Outputs the smoothed RMS amplitude of each band (or each band's % share of the total)
//
// AI assistance: this sketch was written with the assistance of Claude, an AI model
// by Anthropic (https://www.anthropic.com/claude), and reviewed by the project author.
//
// Original code under the MIT License:
//
// Copyright (c) 2021 Upside Down Labs - contact@upsidedownlabs.tech
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#define SAMPLE_RATE 256
#define BAUD_RATE 115200
#define INPUT_PIN A0

// ---- Settings ----
#define PLOT_EVERY 8        // print 1 of every 8 samples -> 32 lines/sec
#define SMOOTHING 0.005     // lower = smoother but slower to react (~0.8 s here)
#define WARMUP_MS 3000      // skip the filters' start-up spike
#define SHOW_PERCENT 0      // 0 = amplitude in ADC counts, 1 = each band's share of the total (%)

// One second-order filter section (biquad)
struct Biquad {
  float b0, b1, b2, a1, a2;
  float z1, z2;
  float process(float in) {
    float x = in - a1 * z1 - a2 * z2;
    float y = b0 * x + b1 * z1 + b2 * z2;
    z2 = z1;
    z1 = x;
    return y;
  }
};

// Butterworth band-pass filters (order 2, two biquads each), designed for 256 Hz
//            b0          b1          b2          a1           a2       z1 z2
Biquad bands[4][2] = {
  { // Delta 0.5-4 Hz
    {0.00173824, 0.00347648, 0.00173824, -1.89242199, 0.89993704, 0, 0},
    {1.0,       -2.0,        1.0,        -1.98389180, 0.98407375, 0, 0} },
  { // Theta 4-8 Hz
    {0.00225158, 0.00450317, 0.00225158, -1.88630094, 0.91699885, 0, 0},
    {1.0,       -2.0,        1.0,        -1.93787319, 0.94914784, 0, 0} },
  { // Alpha 8-13 Hz
    {0.00346068, 0.00692137, 0.00346068, -1.81940129, 0.90346240, 0, 0},
    {1.0,       -2.0,        1.0,        -1.88803326, 0.93050452, 0, 0} },
  { // Beta 13-30 Hz
    {0.03334123, 0.06668246, 0.03334123, -1.32434253, 0.68484989, 0, 0},
    {1.0,       -2.0,        1.0,        -1.69697557, 0.80973432, 0, 0} },
};

const char* names[4] = {"Delta", "Theta", "Alpha", "Beta"};
float power[4] = {0, 0, 0, 0};   // smoothed power of each band

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
    float sample = analogRead(INPUT_PIN);

    // Run the sample through each band filter, then track its smoothed power
    for (byte i = 0; i < 4; i++) {
      float y = bands[i][1].process(bands[i][0].process(sample));
      power[i] += (y * y - power[i]) * SMOOTHING;
    }

    static byte count = 0;
    if (++count >= PLOT_EVERY) {
      count = 0;
      if (millis() < WARMUP_MS) return;

      float amp[4], total = 0;
      for (byte i = 0; i < 4; i++) {
        amp[i] = sqrt(power[i]);   // RMS amplitude
        total += amp[i];
      }

      for (byte i = 0; i < 4; i++) {
        if (i > 0) Serial.print(',');
        Serial.print(names[i]);
        Serial.print(':');
#if SHOW_PERCENT
        Serial.print(total > 0 ? 100.0 * amp[i] / total : 0);
#else
        Serial.print(amp[i]);
#endif
      }
      Serial.println();
    }
  }
}
