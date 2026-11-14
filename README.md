# Arduino-EEG

EEG experiments with Upside Down Labs' [BioAmp](https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware) hardware and an Arduino UNO, building on their [EEG filter example](https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware/blob/main/02-eeg-filter/02-eeg-filter.ino), including a 3D brain visualization that glows with live EEG band activity.

**Live visualization:** [kkruege.github.io/Arduino-EEG](https://kkruege.github.io/Arduino-EEG/). Open it in Chrome or Edge, then click **Connect Arduino**, or **Demo** to try it without hardware.

## Contents

| File | What it does |
|---|---|
| `eeg_filter/eeg_filter.ino` | Filters the EEG signal (0.5–29.5 Hz) and plots it in the Arduino Serial Plotter, with an envelope line. |
| `eeg_bands/eeg_bands.ino` | Splits the signal into Delta, Theta, Alpha and Beta bands and outputs each band's amplitude. |
| `index.html` | 3D brain visualization that reads `eeg_bands.ino` output over USB and pulses in a different color for each band. |
| `spikerbox/spikerbox_eeg_filter/` | Same as `eeg_filter`, for the Backyard Brains Heart & Brain SpikerBox. See [Using a Backyard Brains SpikerBox](#using-a-backyard-brains-heart--brain-spikerbox). |
| `spikerbox/spikerbox_eeg_bands/` | Same as `eeg_bands`, for the SpikerBox. Works with the 3D visualization. |

## Materials

| Photo | Item | Where to buy | Notes |
|---|---|---|---|
| <img src="https://img.tindie.com/images/resize/QVfCPGSn4BIBKMxJnm4FhiLwT8o=/p/1200x630/smart/i/582641/products/2024-01-27T07:47:46.377Z-2.jpg?1706312881" width="200" alt="BioAmp EXG Pill"><br><sub>Image: Upside Down Labs</sub> | **BioAmp EXG Pill** | [Upside Down Labs on Tindie](https://www.tindie.com/products/upsidedownlabs/bioamp-exg-pill-sensor-for-ecg-emg-eog-or-eeg/) | The biopotential amplifier. Needs header pins soldered if yours came unassembled. |
| <img src="https://store.arduino.cc/cdn/shop/files/A000066_03.front_1200x900.jpg?v=1727098250" width="200" alt="Arduino UNO Rev3"><br><sub>Image: Arduino</sub> | **Arduino UNO** | [Adafruit](https://www.adafruit.com/product/4806) · [Arduino Store](https://store.arduino.cc/products/arduino-uno-rev3) | Reads the amplified signal on pin A0. If the UNO is out of stock at Adafruit, their [Metro 328](https://www.adafruit.com/product/2488) is a compatible substitute (it uses a Micro USB cable instead). |
| <img src="https://img.tindie.com/images/resize/T0xWyUJ0kfYDV5oCrurPueIMmH4=/p/1200x630/smart/i/582641/products/2023-01-06T06:26:50.848Z-2.jpg?1672958189" width="200" alt="BioAmp Cable v3"><br><sub>Image: Upside Down Labs</sub> | **BioAmp Cable** | [Upside Down Labs on Tindie](https://www.tindie.com/products/upsidedownlabs/bioamp-cable-v3-record-eeg-emg-ecg-eog/) | Connects the electrodes to the Pill's IN+, IN− and REF. Usually included in Upside Down Labs kits. |
| <img src="https://img.tindie.com/images/resize/AM0NwwsV6_HHCKc3LuXiCQkcuzc=/p/1200x630/smart/i/582641/products/2022-11-23T12:36:15.400Z-20.jpg?1669179000" width="200" alt="Brain BioAmp Band"><br><sub>Image: Upside Down Labs</sub> | **Brain BioAmp Band** (electrode headband) | [Upside Down Labs on Tindie](https://tindie.com/products/upsidedownlabs/brain-bioamp-band-multi-channel-eeg-band) | Holds the two forehead electrodes (IN+ and IN−). |
| <img src="images/photo-gel-electrodes.svg" width="200" alt="Gel electrodes"> | **Gel electrodes** | [Upside Down Labs on Tindie](https://www.tindie.com/stores/upsidedownlabs) | Used for REF behind the ear. Can also be used instead of a headband for all three electrodes. |
| <img src="https://cdn.shopify.com/s/files/1/0613/9684/4785/files/exp-hu-eeg.png?v=1737395870" width="200" alt="Backyard Brains Brain (EEG) Headband"><br><sub>Image: Backyard Brains</sub> | **Backyard Brains EEG Headband** (alternative) | [Backyard Brains](https://backyardbrains.com/products/eegheadband) | Another headband option for the forehead electrodes. |
| <img src="https://cdn-shop.adafruit.com/480x360/62-00.jpg" width="200" alt="USB A-to-B cable"><br><sub>Image: Adafruit</sub> | **3 jumper wires** + **USB cable** for the UNO | USB A-to-B cable: [Adafruit](https://www.adafruit.com/product/62) · [DigiKey](https://www.digikey.com/en/products/result?keywords=102-1030-BE-F0100) | The UNO uses a **USB A-to-B** cable (the square "printer" plug). Female-to-male jumpers work with the Pill's header pins. |

**Electrode options:** use either the **Brain BioAmp Band**, **three gel electrodes**, or the **Backyard Brains headband** for the two forehead electrodes. In all cases, REF goes on a gel electrode behind the ear.

<sub>Product images belong to their vendors and are shown from the vendors' own sites. Photos of the assembled build are in [Photos of the build](#photos-of-the-build) and [Full setup](#full-setup).</sub>

## Wiring

![Wiring diagram: electrodes to BioAmp EXG Pill to Arduino UNO](images/wiring-diagram.svg)

**BioAmp EXG Pill → Arduino UNO**

| BioAmp EXG Pill | Arduino UNO | Wire color in diagram |
|---|---|---|
| VCC | 5V | Red |
| GND | GND | Black |
| OUT | A0 | Yellow |

> ⚠️ **Double-check VCC and GND before plugging in.** Swapping them will destroy the BioAmp EXG Pill. Always follow the labels printed on the board, not the position of the pins or the wire colors.

**5V or 3.3V?** The BioAmp EXG Pill's operating voltage is **3.3 V or 5 V**, according to the [Upside Down Labs documentation](https://docs.upsidedownlabs.tech/hardware/bioamp/bioamp-exg-pill/index.html), so VCC can go to either the UNO's **5V** or **3.3V** pin.
- **5V is recommended with the UNO.** It's what Upside Down Labs uses in their Arduino UNO examples, and it uses the full range of the UNO's 0–5 V analog input.
- **3.3V also works.** The signal then uses a smaller part of the UNO's input range, so the numbers in the plotter will be smaller.
- If you use a board whose analog pins only accept 3.3 V, check the Upside Down Labs docs first: they say the Pill is set up for 5 V by default and can be made 3.3 V-compatible with a voltage divider.

If you connect OUT to a pin other than A0, change `INPUT_PIN` at the top of the sketch to match.

**Electrodes → BioAmp EXG Pill** (through the BioAmp Cable)

| Electrode | Where it goes on your head | Pill input |
|---|---|---|
| Forehead, left | On the forehead, under the headband | IN+ (red wire in the photos) |
| Forehead, right | On the forehead, a few cm from IN+ | IN− (black wire in the photos) |
| Reference | Bony area behind the ear (mastoid), on a gel electrode | REF (yellow wire in the photos) |

Cable colors differ between kits, so go by the **IN+ / IN− / REF** labels on the board and cable.

### Photos of the build

| BioAmp EXG Pill close-up | Pill connected to the UNO | UNO connections |
|---|---|---|
| <img src="images/build-bioamp-pill-closeup.jpg" width="260" alt="BioAmp EXG Pill close-up showing VCC, GND, OUT labels at the top and IN−, IN+, REF at the electrode connector"> | <img src="images/build-pill-to-uno.jpg" width="260" alt="BioAmp EXG Pill wired to an Arduino UNO with red, black and yellow jumper wires"> | <img src="images/build-uno-connections.jpg" width="260" alt="Close-up of jumper wires plugged into the Arduino UNO power and analog headers"> |
| Top: **VCC, GND, OUT** (to the UNO). Bottom connector: **IN−, IN+, REF** (to the electrodes). | Red = VCC, black = GND, yellow = OUT. | Connect yellow to **A0**, black to **GND**, red to **5V**. |

## Setup steps

1. Wire the BioAmp EXG Pill to the Arduino UNO as shown above. Check VCC and GND twice.
2. Plug the UNO into a **laptop running on battery** (unplugged from the wall). This is safer while electrodes are on your body, and reduces 50/60 Hz mains noise.
3. Upload a sketch from this repository with the Arduino IDE (**Tools → Board → Arduino UNO**).
4. Clean the skin where the electrodes go (forehead and behind the ear) to remove oil and makeup.
5. Put on the headband with the electrodes on your forehead, and stick the gel electrode for REF behind your ear.
6. Connect the electrode cable to the Pill's IN+, IN− and REF.
7. Open the Serial Plotter (for `eeg_filter`) or the 3D visualization (for `eeg_bands`). Blink a few times: you should see clear spikes, which confirms everything is connected.
8. Sit still with your face relaxed. Unplug the electrodes from your body before disconnecting or rewiring anything.

> This is an educational and hobby project, not a medical device.

## Full setup

<img src="images/build-full-setup.jpg" width="640" alt="Full setup: electrode headband with three snap electrodes, BioAmp cable, BioAmp EXG Pill, and Arduino UNO with USB cable">

The headband holds the two forehead electrodes (red and black wires). The third electrode on its own tab (yellow wire) is the reference that goes behind the ear.

## Running the 3D visualization

1. Upload `eeg_bands/eeg_bands.ino` to the Arduino.
2. Close the Arduino IDE's Serial Monitor and Serial Plotter, so the port is free.
3. Open the visualization in **Chrome or Edge**: either the [live link](https://kkruege.github.io/Arduino-EEG/), or `index.html` downloaded to your computer.
4. Click **Connect Arduino** and choose the board's port. Or click **Demo** to see it with simulated data.

| Band | Range | Color |
|---|---|---|
| Delta | 0.5–4 Hz | Purple |
| Theta | 4–8 Hz | Blue |
| Alpha | 8–13 Hz | Green |
| Beta | 13–30 Hz | Orange |

Each band's brightness is relative to its own average over the last ~20 seconds. The pulse speeds are slowed down so they're visible, and the spread across the brain is artistic: two forehead electrodes can't tell where in the brain activity comes from.

## Using a Backyard Brains Heart & Brain SpikerBox

The [Heart & Brain SpikerBox](https://backyardbrains.com/products/heartandbrainspikerbox) can be used instead of the BioAmp EXG Pill and Arduino UNO. It has its own amplifier and an **Arduino Leonardo** built in, so there's no wiring: connect the electrodes (or the [Backyard Brains EEG headband](https://backyardbrains.com/products/eegheadband)) to the SpikerBox and plug it into your computer by USB.

1. Open `spikerbox/spikerbox_eeg_bands/spikerbox_eeg_bands.ino` (or `spikerbox_eeg_filter.ino` for the Serial Plotter) in the Arduino IDE.
2. Choose **Tools → Board → Arduino Leonardo** and the SpikerBox's port. The sketch refuses to compile for other boards, as a safety check.
3. Upload. If the upload can't find the board, press the SpikerBox's reset button (if it has one) just as the IDE says "Uploading…", which often helps with Leonardo boards.
4. Use the Serial Plotter or the [3D visualization](https://kkruege.github.io/Arduino-EEG/) as usual.

> **This replaces the SpikerBox's factory firmware.** The Backyard Brains Spike Recorder app won't work with it until you put the original back: upload [`HeartAndBrainLeonardo-V1-0.ino`](https://github.com/BackyardBrains/SpikerShield/tree/master/Heart-And-Brain/Arduino%20Code/HeartAndBrainLeonardo-V1-0) from Backyard Brains' [SpikerShield repository](https://github.com/BackyardBrains/SpikerShield), with the board set to Arduino Leonardo.

**How the SpikerBox versions differ from the UNO sketches**

- They keep the **5 kHz carrier signal on pin D13** that Backyard Brains' firmware generates, since the SpikerBox hardware expects it, and read the signal on **A0** like the factory firmware.
- They take 10,000 readings per second and average every 40 into one sample, giving **250 samples per second**. Averaging whole carrier cycles also removes carrier ripple from the signal. The filters are redesigned for 250 Hz.
- The SpikerBox amplifies differently from the BioAmp, so the numbers are on a different scale. If the EEG trace in the Serial Plotter is cut off or looks flat, change `Y_LIMIT`. The 3D visualization adjusts automatically.

**How they were tested:** both sketches compile for the Arduino Leonardo. Their code was also run on a computer with simulated SpikerBox signals: the carrier came out at 5 kHz; test tones at 2, 6, 10 and 20 Hz each showed up in the right band; carrier ripple, 60 Hz mains hum and slow drift were filtered out; and the output streamed correctly into the 3D visualization. They have **not yet been tested on a physical SpikerBox**.

## Notes

- Values are raw ADC counts after amplification, not calibrated microvolts.
- At the forehead, eye blinks (Delta) and facial and jaw muscle activity (Beta) are much larger than brain activity.

## Credits and license

The Arduino sketches are modified from Upside Down Labs' **02-eeg-filter.ino** in [Brain-BioAmp-Arduino-Firmware](https://github.com/upsidedownlabs/Brain-BioAmp-Arduino-Firmware/blob/main/02-eeg-filter/02-eeg-filter.ino), © 2021 Upside Down Labs, used under the MIT License. The full license notice is included at the top of each sketch.

What was changed:

- **`eeg_filter.ino`:** same 0.5–29.5 Hz filter as the original, with output reformatted for the Serial Plotter (labeled lines, fixed Max/Min bounds, slower scrolling), a start-up spike skip, and an added amplitude envelope.
- **`eeg_bands.ino`:** keeps the original 256 Hz sampling timer and biquad filter structure, but replaces the single band-pass filter with four band filters (Delta, Theta, Alpha, Beta) and outputs each band's amplitude.
- **`spikerbox/` sketches:** combine the filtering above with the timer, fast ADC setup and 5 kHz carrier from Backyard Brains' [`HeartAndBrainLeonardo-V1-0.ino`](https://github.com/BackyardBrains/SpikerShield/tree/master/Heart-And-Brain/Arduino%20Code/HeartAndBrainLeonardo-V1-0) (© 2013 Backyard Brains, MIT License; full notice in each sketch). They output text for the Serial Plotter and visualization instead of Spike Recorder's binary format.
- **`index.html`:** new. A 3D visualization built with [Three.js](https://threejs.org/) (MIT License) that reads the `eeg_bands.ino` output.

### AI assistance

The code (`eeg_filter.ino`, `eeg_bands.ino`, the `spikerbox/` sketches, `index.html`), the wiring diagram and this README were created with the assistance of [Claude](https://www.anthropic.com/claude), an AI model by Anthropic. The project author directed the work and reviewed the results.

If you use Upside Down Labs' hardware and code, please consider supporting them by buying from [Upside Down Labs](https://upsidedownlabs.tech/).
