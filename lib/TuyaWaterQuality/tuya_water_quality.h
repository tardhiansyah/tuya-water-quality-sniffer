#ifndef TUYA_WATER_QUALITY_H
#define TUYA_WATER_QUALITY_H

#include <Arduino.h>
#include <Stream.h>
#include <tuya.h>

// Enums for Tuya Water Quality Data Points
enum TuyaWaterQualityDataPoint {
  DP_TEMPERATURE = 0x08,
  DP_HIGH_TEMPERATURE_THRESHOLD = 0x66,
  DP_LOW_TEMPERATURE_THRESHOLD = 0x67,
  DP_PH = 0x6A,
  DP_HIGH_PH_THRESHOLD = 0x6B,
  DP_LOW_PH_THRESHOLD = 0x6C,
  DP_TDS = 0x6F,
  DP_HIGH_TDS_THRESHOLD = 0x70,
  DP_LOW_TDS_THRESHOLD = 0x71,
};

// Structs for Sensor Data
struct SensorData {
  double value;
  double MaxThreshold;
  double MinThreshold;
};

struct TuyaWaterQualitySensor {
  SensorData temperature;
  SensorData ph;
  SensorData tds;
};

struct TuyaWaterQualityInformation {
  String productId;
  String version;
  uint16_t operationMode;
};

// TuyaWaterQuality class definition
class TuyaWaterQuality : public Tuya {
public:
  TuyaWaterQuality();

  double getTemperature();
  double getPH();
  int32_t getTDS();

  double getMaxTemperature();
  bool setMaxTemperature(int32_t value);

  double getMinTemperature();
  bool setMinTemperature(int32_t value);

  double getMaxPH();
  bool setMaxPH(int32_t value);

  double getMinPH();
  bool setMinPH(int32_t value);

  int32_t getMaxTDS();
  bool setMaxTDS(int32_t value);

  int32_t getMinTDS();
  bool setMinTDS(int32_t value);

  void onReceiveSensor(void (*callback)(TuyaWaterQualitySensor& sensor));

private:
  TuyaWaterQualitySensor _sensorData;
  void (*_onReceiveSensor)(TuyaWaterQualitySensor& sensor);

  bool decodeReportStatusAsync(TuyaFrame& frame) override;
  uint32_t decodeSensorRawValue(uint8_t* data);
  bool setThreshold(TuyaWaterQualityDataPoint datapoint, int32_t value);
  bool generateSensorData(uint8_t(&buffer)[8], TuyaWaterQualityDataPoint dataPoint, int32_t value);
};

#endif // TUYA_WATER_QUALITY_H