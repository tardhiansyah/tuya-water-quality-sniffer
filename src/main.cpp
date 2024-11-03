#include <Arduino.h>
#include <HardwareSerial.h>
#include <tuya_water_quality.h>

#define RX_MCU_PIN 16
#define TX_MCU_PIN 17

HardwareSerial Monitor(0);
HardwareSerial TuyaSniffer(2);

TuyaWaterQuality waterQuality(TuyaSniffer);

void logFrame(TuyaFrame& frame);
void logSensor(TuyaSensorData& sensor);

void setup() {
  Monitor.begin(115200);
  waterQuality.begin(RX_MCU_PIN, TX_MCU_PIN);

  waterQuality.setCallbackFrame(logFrame);
  waterQuality.setCallbackSensor(logSensor);

  Monitor.println("Serial communication started");
}

void loop() {
  static uint32_t lastMillis = 0;
  if (millis() - lastMillis < 5000) {
    if (!waterQuality.setTemperatureHighThreshold(810)) {
      Monitor.println("Failed to set temperature high threshold");
    }

    if (!waterQuality.setTemperatureLowThreshold(214)) {
      Monitor.println("Failed to set temperature low threshold");
    }

    if (!waterQuality.setPhHighThreshold(745)) {
      Monitor.println("Failed to set PH high threshold");
    }

    if (!waterQuality.setPhLowThreshold(512)) {
      Monitor.println("Failed to set PH low threshold");
    }

    if (!waterQuality.setTdsHighThreshold(1000)) {
      Monitor.println("Failed to set TDS high threshold");
    }

    if (!waterQuality.setTdsLowThreshold(500)) {
      Monitor.println("Failed to set TDS low threshold");
    }
    lastMillis = millis();
  }
  waterQuality.loop();
}

void logFrame(TuyaFrame& frame) {
  Monitor.print("Header: ");
  Monitor.print(frame.header[0], HEX);
  Monitor.print(" ");
  Monitor.print(frame.header[1], HEX);
  Monitor.print(" Version: ");
  Monitor.print(frame.version, HEX);
  Monitor.print(" Command: ");
  Monitor.print(frame.command, HEX);
  Monitor.print(" Length: ");
  Monitor.print(frame.length[0], HEX);
  Monitor.print(" ");
  Monitor.print(frame.length[1], HEX);
  Monitor.print(" Data: ");
  for (uint16_t i = 0; i < (frame.length[0] << 8 | frame.length[1]); i++) {
    Monitor.print(frame.data[i], HEX);
    Monitor.print(" ");
  }
  Monitor.print(" Checksum: ");
  Monitor.println(frame.checksum, HEX);
}

void logSensor(TuyaSensorData& sensor) {
  Monitor.print("Temperature: ");
  Monitor.print(sensor.temperature.value);
  Monitor.print(" High: ");
  Monitor.print(sensor.temperature.highThreshold);
  Monitor.print(" Low: ");
  Monitor.println(sensor.temperature.lowThreshold);
  Monitor.print("PH: ");
  Monitor.print(sensor.ph.value);
  Monitor.print(" High: ");
  Monitor.print(sensor.ph.highThreshold);
  Monitor.print(" Low: ");
  Monitor.println(sensor.ph.lowThreshold);
  Monitor.print("TDS: ");
  Monitor.print(sensor.tds.value);
  Monitor.print(" High: ");
  Monitor.print(sensor.tds.highThreshold);
  Monitor.print(" Low: ");
  Monitor.println(sensor.tds.lowThreshold);
}
