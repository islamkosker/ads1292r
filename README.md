# ADS1292R Analog Front-End (AFE) IC

## 📊 Overview

The ADS1292R is a low-power, 2-channel, 24-bit delta-sigma analog front-end (AFE) for biopotential measurements such as ECG (electrocardiogram) and respiration. It integrates all necessary features to reduce board size, component count, and power consumption, making it ideal for portable medical devices.

---

## ⚖️ Key Features

* 2 simultaneous-sampling delta-sigma ADCs (24-bit resolution)
* Low power consumption: \~335 µW/channel (at 3V supply)
* Programmable Gain Amplifiers (PGA): Gains of 1, 2, 3, 4, 6, 8, or 12
* Integrated:

  * Internal oscillator (512 kHz)
  * Voltage reference (2.42 V or 4.033 V)
  * Right Leg Drive (RLD) amplifier
  * Lead-off detection
  * Respiration measurement engine (ADS1292R only)
* SPI-compatible serial interface
* Small 32-pin TQFP or VQFN package

---

## ⚡ Power Supply and Startup

* **Analog Supply (AVDD)**: 2.7 V to 5.25 V
* **Digital Supply (DVDD)**: 1.7 V to 3.6 V
* **Startup sequence:**

  1. Apply power with all digital inputs LOW.
  2. Wait \~32 ms for power-on-reset.
  3. Pulse `PWDN/RESET` low to reset the device.
  4. Configure registers via SPI.

---

## 🔄 SPI Communication

* **SPI Mode**: CPOL = 0, CPHA = 1 (Mode 1)
* **Interface pins**:

  * `CS`: Chip Select
  * `SCLK`: Serial Clock
  * `DIN`: Data In
  * `DOUT`: Data Out
  * `DRDY`: Data Ready (active LOW)

### SPI Commands

| Command | Byte Code |           |       |         |
| ------- | --------- | --------- | ----- | ------- |
| WAKEUP  | 0x02      |           |       |         |
| STANDBY | 0x04      |           |       |         |
| RESET   | 0x06      |           |       |         |
| START   | 0x08      |           |       |         |
| STOP    | 0x0A      |           |       |         |
| RDATAC  | 0x10      |           |       |         |
| SDATAC  | 0x11      |           |       |         |
| RDATA   | 0x12      |           |       |         |
| RREG    | 0x20      | reg\_addr | num-1 |         |
| WREG    | 0x40      | reg\_addr | num-1 | data... |

---

## 📃 Register Map Summary

Registers must be accessed after issuing `SDATAC`.

| Address | Name    | Description                         |
| ------- | ------- | ----------------------------------- |
| 0x01    | CONFIG1 | Data rate selection                 |
| 0x02    | CONFIG2 | Reference buffer, test signal       |
| 0x03    | LOFF    | Lead-off control                    |
| 0x04    | CH1SET  | Channel 1 configuration             |
| 0x05    | CH2SET  | Channel 2 configuration             |
| 0x09    | RESP1   | Respiration control (ADS1292R only) |
| 0x0A    | RESP2   | Respiration phase and frequency     |

---

## 🪠 Functional Blocks

* **Delta-Sigma ADC**: Oversampled at internal CLK (e.g., 512 kHz)
* **PGA**: Provides analog gain (max 12x)
* **RLD Amplifier**: Improves CMRR by driving common-mode voltage
* **Lead-Off Detection**: Detects electrode disconnection using current source/sink
* **Respiration Engine**: Injects and demodulates excitation signal across electrodes

---

## 🤫 Typical Initialization Sequence (Pseudocode)

```c
spi_write(CMD::SDATAC);                    // Stop continuous read
spi_write_reg(CONFIG1, 0x02);              // 500 SPS
spi_write_reg(CONFIG2, 0xA0);              // Internal ref on, no test signals
spi_write_reg(CH1SET, 0x00);               // Gain=6, normal electrode input
spi_write_reg(CH2SET, 0x00);
spi_write(CMD::START);                     // Start conversions
spi_write(CMD::RDATAC);                    // Continuous read mode
```

---

## 👨‍💻 Design Recommendations

* **Power filtering**: 0.1 µF + 10 µF on AVDD/DVDD pins
* **Reference cap**: 10 µF low-ESR on `VREFP`
* **Layout**:

  * Separate analog and digital ground planes
  * Shield analog inputs
  * Place RLDOUT routing away from analog inputs

---

## 📈 Performance Summary

| Parameter             | Value             |
| --------------------- | ----------------- |
| Noise (PGA=6)         | 8 µVpp (typ.)     |
| CMRR                  | 120 dB (typ.)     |
| SNR                   | 107 dB @ Gain=6   |
| Data Rates            | 125 SPS to 8 kSPS |
| Input Bias Current    | 200 pA            |
| Respiration Frequency | 32 / 64 kHz       |

---

## 📖 Resources

* [ADS1292R Datasheet - Texas Instruments](https://www.ti.com/product/ADS1292R)
* Application Notes:

  * SLAU384 (ECG front-end design)
  * SBAA204 (Respiration measurement theory)

---

> Prepared for embedded system engineers developing ADS1292R firmware in C.
