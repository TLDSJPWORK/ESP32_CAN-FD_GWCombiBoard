// SPDX-License-Identifier: CC-BY-4.0
// JP WORKS / TLDSJPWORK - ESP32 CAN-FD GateWay CombiBoard
// CH1 <-> CH2 pass-through. See ../README.md before connecting a bus.
#include <Arduino.h>
#include <SPI.h>
#include <ACAN2517FD.h>

#if !defined(CONFIG_IDF_TARGET_ESP32)
#error "Select a classic ESP32 board (ESP32 Dev Module), not ESP32-S3/C3."
#endif

// Match these to the oscillator fitted on EACH CAN module.
constexpr auto CH1_OSC = ACAN2517FDSettings::OSC_20MHz;
constexpr auto CH2_OSC = ACAN2517FDSettings::OSC_20MHz;
constexpr uint32_t CH1_BIT_RATE = 500000;
constexpr uint32_t CH2_BIT_RATE = 500000;
// x4: 500 kbit/s arbitration, 2 Mbit/s data phase for BRS frames.
constexpr auto CH1_DATA_FACTOR = DataBitRateFactor::x4;
constexpr auto CH2_DATA_FACTOR = DataBitRateFactor::x4;

SPIClass spiCh1(VSPI);
SPIClass spiCh2(HSPI);
ACAN2517FD canCh1(25, spiCh1, 32); // CS, SPI, INT (J9)
ACAN2517FD canCh2(15, spiCh2, 27); // CS, SPI, INT (J10)

// One retained frame per direction, in addition to driver/controller FIFOs.
// A full destination never blocks servicing the reverse direction.
struct Direction {
  CANFDMessage frame;
  bool pending = false;
  uint32_t received = 0;
  uint32_t queued = 0;
};
Direction ch1ToCh2;
Direction ch2ToCh1;

void forwardOne(ACAN2517FD &source, ACAN2517FD &destination,
                Direction &state) {
  if (!state.pending && source.receive(state.frame)) {
    ++state.received;
    state.frame.idx = 0; // RX filter index -> normal transmit FIFO.
    state.pending = true;
  }
  if (state.pending && destination.tryToSend(state.frame)) {
    state.pending = false;
    ++state.queued; // Accepted by TX queue; not a bus delivery confirmation.
  }
}

void setup() {
  Serial.begin(115200);
  // Both CS lines inactive before starting either SPI bus.
  pinMode(25, OUTPUT);
  pinMode(15, OUTPUT);
  digitalWrite(25, HIGH);
  digitalWrite(15, HIGH);
  spiCh1.begin(26, 18, 19, 25); // SCK, MISO, MOSI, CS (J9)
  spiCh2.begin(14, 12, 13, 15); // SCK, MISO, MOSI, CS (J10)

  ACAN2517FDSettings settings1(CH1_OSC, CH1_BIT_RATE, CH1_DATA_FACTOR);
  ACAN2517FDSettings settings2(CH2_OSC, CH2_BIT_RATE, CH2_DATA_FACTOR);
  settings1.mRequestedMode = ACAN2517FDSettings::NormalFD;
  settings2.mRequestedMode = ACAN2517FDSettings::NormalFD;
  // Two-argument begin installs the library's pass-all receive filter.
  const uint32_t error1 = canCh1.begin(settings1, [] { canCh1.isr(); });
  const uint32_t error2 = canCh2.begin(settings2, [] { canCh2.isr(); });
  Serial.printf("CH1 begin=0x%08lX, CH2 begin=0x%08lX\n",
                (unsigned long)error1, (unsigned long)error2);
  if (error1 != 0 || error2 != 0) {
    // Disable any successfully started channel as well.
    if (error1 == 0) canCh1.end();
    if (error2 == 0) canCh2.end();
    Serial.println("Gateway stopped: check oscillator, wiring and bit rates.");
    while (true) delay(1000);
  }
  Serial.println("Gateway ready: CH1 <-> CH2, all IDs, CAN / CAN FD.");
}

void loop() {
  forwardOne(canCh1, canCh2, ch1ToCh2);
  forwardOne(canCh2, canCh1, ch2ToCh1);

  static uint32_t lastReport = 0;
  const uint32_t now = millis();
  if (uint32_t(now - lastReport) >= 1000) {
    lastReport = now;
    Serial.printf("1->2 rx=%lu queued=%lu pending=%u | "
                  "2->1 rx=%lu queued=%lu pending=%u\n",
                  (unsigned long)ch1ToCh2.received,
                  (unsigned long)ch1ToCh2.queued, unsigned(ch1ToCh2.pending),
                  (unsigned long)ch2ToCh1.received,
                  (unsigned long)ch2ToCh1.queued, unsigned(ch2ToCh1.pending));
    Serial.printf("CAN error counters: CH1=0x%08lX CH2=0x%08lX\n",
                  (unsigned long)canCh1.errorCounters(),
                  (unsigned long)canCh2.errorCounters());
  }
  delay(1); // Yield to ESP32 driver tasks / watchdog.
}
