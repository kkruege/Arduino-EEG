// EEG Band Amplitudes (Backyard Brains SpikerBox)
// For the Backyard Brains Heart & Brain SpikerBox (Arduino Leonardo inside).
// Outputs Delta, Theta, Alpha and Beta amplitudes for the Serial Plotter and the 3D brain page (index.html).
//
// IMPORTANT: in the Arduino IDE choose Tools > Board > Arduino Leonardo.
// Uploading this replaces the SpikerBox's factory firmware, so the Spike Recorder
// app won't work until you upload Backyard Brains' HeartAndBrainLeonardo-V1-0.ino again:
// https://github.com/BackyardBrains/SpikerShield/tree/master/Heart-And-Brain/Arduino%20Code
//
// Based on:
//  - Backyard Brains' HeartAndBrainLeonardo-V1-0.ino (timer setup, fast ADC, and the
//    5 kHz carrier on D13 that the SpikerBox hardware expects)
//  - Upside Down Labs' 02-eeg-filter.ino (sampling loop and biquad filter structure):
//    https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware/blob/main/02-eeg-filter/02-eeg-filter.ino
//
// Changes from those originals:
//  - Sends labeled band amplitudes as text instead of Spike Recorder's binary format
//  - Keeps the 10 kHz timer and 5 kHz carrier, but averages every 40 readings into
//    one 250 Hz sample
//  - Four band-pass filters (Delta 0.5-4, Theta 4-8, Alpha 8-13, Beta 13-30 Hz),
//    designed for 250 Hz with SciPy
//  - Outputs the smoothed RMS amplitude of each band (or each band's % share)
//
// AI assistance: this sketch was written with the assistance of Claude, an AI model
// by Anthropic (https://www.anthropic.com/claude), and reviewed by the project author.
//
// ---- Backyard Brains license (MIT) ----
//
// Copyright (c) 2013 Backyard Brains
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// ---- Upside Down Labs license (MIT) ----
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

#if !defined(__AVR_ATmega32U4__)
#error "This sketch is for the Heart & Brain SpikerBox. Select Tools > Board > Arduino Leonardo."
#endif

#define BAUD_RATE 115200
#define INPUT_PIN A0          // the SpikerBox's amplified signal (same pin as the factory firmware)
#define CARRIER_PIN 13        // the SpikerBox needs a 5 kHz carrier here (D13 = port C bit 7)

// Timer: 16 MHz / 8 / (199 + 1) = 10,000 interrupts per second.
// Each interrupt flips the carrier (giving 5 kHz) and takes one ADC reading.
// Every 40 readings are averaged into one sample: 10,000 / 40 = 250 samples per second.
// Averaging whole carrier cycles also removes any 5 kHz ripple from the signal.
#define TIMER_COMPARE 199
#define DECIMATE 40
#define SAMPLE_RATE 250

// Hand-off from the interrupt to loop(): a small queue of 40-reading sums.
#define QUEUE_SIZE 16
volatile unsigned int sampleQueue[QUEUE_SIZE];
volatile byte queueHead = 0;  // written by the interrupt
volatile byte queueTail = 0;  // written by loop()
volatile unsigned int adcSum = 0;
volatile byte adcCount = 0;

void startSampling() {
  pinMode(CARRIER_PIN, OUTPUT);
  cli();
  // Faster ADC clock (prescaler 16), as in the factory firmware, so a reading fits in 100 us
  ADCSRA = (ADCSRA & ~0x07) | 0x04;
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = TIMER_COMPARE;
  TCCR1B |= (1 << WGM12);   // CTC mode
  TCCR1B |= (1 << CS11);    // prescaler 8
  TIMSK1 |= (1 << OCIE1A);  // enable compare interrupt
  sei();
}

ISR(TIMER1_COMPA_vect) {
  PORTC ^= B10000000;       // toggle D13: 5 kHz carrier
  adcSum += analogRead(INPUT_PIN);
  if (++adcCount >= DECIMATE) {
    byte next = (queueHead + 1) % QUEUE_SIZE;
    if (next != queueTail) {          // if loop() fell behind, drop the sample
      sampleQueue[queueHead] = adcSum;
      queueHead = next;
    }
    adcSum = 0;
    adcCount = 0;
  }
}

// Returns true and the next averaged sample (0-1023 scale) when one is ready.
bool nextSample(float &sample) {
  if (queueTail == queueHead) return false;
  sample = sampleQueue[queueTail] / (float)DECIMATE;
  queueTail = (queueTail + 1) % QUEUE_SIZE;
  return true;
}

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

// ---- Settings ----
#define PLOT_EVERY 8        // print 1 of every 8 samples -> about 31 lines/sec
#define SMOOTHING 0.005     // lower = smoother but slower to react (~0.8 s here)
#define WARMUP_MS 3000      // skip the filters' start-up spike
#define SHOW_PERCENT 0      // 0 = amplitude in ADC counts, 1 = each band's share of the total (%)

// Butterworth band-pass filters (order 2, two biquads each), designed for 250 Hz
//            b0          b1          b2          a1           a2       z1 z2
Biquad bands[4][2] = {
  { // Delta 0.5-4 Hz
    {0.00182013, 0.00364026, 0.00182013, -1.88979231, 0.89766288, 0, 0},
    {1.0,       -2.0,        1.0,        -1.98350381, 0.98369456, 0, 0} },
  { // Theta 4-8 Hz
    {0.00235721, 0.00471442, 0.00235721, -1.88294420, 0.91509809, 0, 0},
    {1.0,       -2.0,        1.0,        -1.93614071, 0.94795535, 0, 0} },
  { // Alpha 8-13 Hz
    {0.00362168, 0.00724336, 0.00362168, -1.81326188, 0.90127690, 0, 0},
    {1.0,       -2.0,        1.0,        -1.88439409, 0.92888395, 0, 0} },
  { // Beta 13-30 Hz
    {0.03476160, 0.06952319, 0.03476160, -1.30268422, 0.67887887, 0, 0},
    {1.0,       -2.0,        1.0,        -1.68751190, 0.80541776, 0, 0} },
};

const char* names[4] = {"Delta", "Theta", "Alpha", "Beta"};
float power[4] = {0, 0, 0, 0};   // smoothed power of each band

void setup() {
  Serial.begin(BAUD_RATE);
  startSampling();
}

void loop() {
  float sample;
  while (nextSample(sample)) {
    for (byte i = 0; i < 4; i++) {
      float y = bands[i][1].process(bands[i][0].process(sample));
      power[i] += (y * y - power[i]) * SMOOTHING;
    }

    static byte count = 0;
    if (++count < PLOT_EVERY) continue;
    count = 0;
    if (millis() < WARMUP_MS) continue;

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
