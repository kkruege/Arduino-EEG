# Arduino-EEG

EEG experiments with Upside Down Labs' [BioAmp](https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware) hardware and an Arduino UNO, building on their [EEG filter example](https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware/blob/main/02-eeg-filter/02-eeg-filter.ino), including a 3D brain visualization that glows with live EEG band activity.

**Live visualization:** https://kkruege.github.io/Arduino-EEG/ (after GitHub Pages is turned on, see below)

## Contents

| File | What it does |
|---|---|
| `eeg_filter/eeg_filter.ino` | Filters the EEG signal (0.5–29.5 Hz) and plots it in the Arduino Serial Plotter, with an envelope line. |
| `eeg_bands/eeg_bands.ino` | Splits the signal into Delta, Theta, Alpha and Beta bands and outputs each band's amplitude. |
| `index.html` | 3D brain visualization that reads `eeg_bands.ino` output over USB and pulses in a different color for each band. |

## Hardware setup

- BioAmp EXG Pill output → Arduino **A0**, plus 5V and GND.
- Electrodes: two on the forehead (IN+ and IN−), reference (REF) behind the ear on the bony mastoid.
- For safety and less noise, power the Arduino from a **laptop running on battery**, not plugged into the wall.

## Running the 3D visualization

1. Upload `eeg_bands/eeg_bands.ino` to the Arduino.
2. Close the Arduino IDE's Serial Monitor and Serial Plotter, so the port is free.
3. Open the visualization in **Chrome or Edge**: either the GitHub Pages link above, or `index.html` downloaded to your computer.
4. Click **Connect Arduino** and choose the board's port. Or click **Demo** to see it with simulated data.

| Band | Range | Color |
|---|---|---|
| Delta | 0.5–4 Hz | Purple |
| Theta | 4–8 Hz | Blue |
| Alpha | 8–13 Hz | Green |
| Beta | 13–30 Hz | Orange |

Each band's brightness is relative to its own average over the last ~20 seconds. The pulse speeds are slowed down so they're visible, and the spread across the brain is artistic: two forehead electrodes can't tell where in the brain activity comes from.

## Notes

- Values are raw ADC counts after amplification, not calibrated microvolts.
- At the forehead, eye blinks (Delta) and facial and jaw muscle activity (Beta) are much larger than brain activity.

## Turning on GitHub Pages

In this repository, go to **Settings → Pages**, set **Branch** to `main` and `/ (root)`, and click **Save**. The page appears at the link above after a minute or two.

## Credits and license

The Arduino sketches are modified from Upside Down Labs' **02-eeg-filter.ino** in [Brain-BioAmp-Arduino-Firmware](https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware/blob/main/02-eeg-filter/02-eeg-filter.ino), © 2021 Upside Down Labs, used under the MIT License. The full license notice is included at the top of each sketch.

What was changed:

- **`eeg_filter.ino`:** same 0.5–29.5 Hz filter as the original, with output reformatted for the Serial Plotter (labeled lines, fixed Max/Min bounds, slower scrolling), a start-up spike skip, and an added amplitude envelope.
- **`eeg_bands.ino`:** keeps the original 256 Hz sampling timer and biquad filter structure, but replaces the single band-pass filter with four band filters (Delta, Theta, Alpha, Beta) and outputs each band's amplitude.
- **`index.html`:** new. A 3D visualization built with [Three.js](https://threejs.org/) (MIT License) that reads the `eeg_bands.ino` output.

If you use Upside Down Labs' hardware and code, please consider supporting them by buying from [Upside Down Labs](https://upsidedownlabs.tech/).
