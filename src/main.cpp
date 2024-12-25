#include <Arduino.h>
#include <HardwareSerial.h>
#include "tuya_water_quality.h"

#define DEBUG           true
#define UART_RX_PIN     16
#define UART_TX_PIN     17

#if DEBUG
#define D_begin(...)    Serial.begin(__VA_ARGS__)
#define D_print(...)    Serial.print(__VA_ARGS__)
#define D_write(...)    Serial.write(__VA_ARGS__)
#define D_println(...)  Serial.println(__VA_ARGS__)
#else
#define D_begin(...)
#define D_print(...)
#define D_write(...)
#define D_println(...)
#endif

// Hardware serial and water quality sensor
HardwareSerial TuyaSniffer(2);
TuyaWaterQuality waterQuality;

void setup() {
  D_begin(115200);
  TuyaSniffer.begin(9600, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);

  waterQuality.begin(&TuyaSniffer);
  waterQuality.debug(Serial, DEBUG);
  waterQuality.onResetWiFiPairMode(resetDevice);
  waterQuality.onReceiveSensor(logSensor);
}

void loop() {
  if (waterQuality.isInitialized()) {
    TuyaProductInformation product = waterQuality.getProductInformation();

    int32_t maxTDS = waterQuality.getMaxTDS();
    int32_t minTDS = waterQuality.getMinTDS();
    int32_t TDS = waterQuality.getTDS();

    float maxPH = waterQuality.getMaxPH();
    float minPH = waterQuality.getMinPH();
    float pH = waterQuality.getPH();

    float maxTemperature = waterQuality.getMaxTemperature();
    float minTemperature = waterQuality.getMinTemperature();
    float temperature = waterQuality.getTemperature();

    waterQuality.setMaxTDS(1000);
    waterQuality.setMinTDS(500);

    waterQuality.setMaxPH(7.0 * 100);
    waterQuality.setMinPH(5.5 * 100);

    waterQuality.setMaxTemperature(31.2 * 10);
    waterQuality.setMinTemperature(20.5 * 10);

    waterQuality.getNetworkStatus();
    waterQuality.setNetworkStatus(WIFI_NOT_CONNECTED);
  }


  waterQuality.loop();
}

void logSensor(TuyaWaterQualitySensor& sensor) {
  D_print("TDS: ");
  D_print(sensor.tds.value);
  D_print(" High: ");
  D_print(sensor.tds.MaxThreshold);
  D_print(" Low: ");
  D_println(sensor.tds.MinThreshold);

  D_print("pH: ");
  D_print(sensor.ph.value);
  D_print(" High: ");
  D_print(sensor.ph.MaxThreshold);
  D_print(" Low: ");
  D_println(sensor.ph.MinThreshold);

  D_print("Temperature: ");
  D_print(sensor.temperature.value);
  D_print(" High: ");
  D_print(sensor.temperature.MaxThreshold);
  D_print(" Low: ");
  D_println(sensor.temperature.MinThreshold);

  D_println();
}

void resetDevice() {
  ESP.restart();
}