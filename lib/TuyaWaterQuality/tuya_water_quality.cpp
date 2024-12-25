#include "tuya_water_quality.h"

TuyaWaterQuality::TuyaWaterQuality() : Tuya() {
  _onReceiveSensor = nullptr;
  _sensorData = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
  };
}

double TuyaWaterQuality::getTemperature() {
  return _sensorData.temperature.value;
}

double TuyaWaterQuality::getPH() {
  return _sensorData.ph.value;
}

int32_t TuyaWaterQuality::getTDS() {
  return _sensorData.tds.value;
}

double TuyaWaterQuality::getMaxTemperature() {
  return _sensorData.temperature.MaxThreshold;
}

bool TuyaWaterQuality::setMaxTemperature(int32_t value) {
  return setThreshold(DP_HIGH_TEMPERATURE_THRESHOLD, value);
}

double TuyaWaterQuality::getMinTemperature() {
  return _sensorData.temperature.MinThreshold;
}

bool TuyaWaterQuality::setMinTemperature(int32_t value) {
  return setThreshold(DP_LOW_TEMPERATURE_THRESHOLD, value);
}

double TuyaWaterQuality::getMaxPH() {
  return _sensorData.ph.MaxThreshold;
}

bool TuyaWaterQuality::setMaxPH(int32_t value) {
  return setThreshold(DP_HIGH_PH_THRESHOLD, value);
}

double TuyaWaterQuality::getMinPH() {
  return _sensorData.ph.MinThreshold;
}

bool TuyaWaterQuality::setMinPH(int32_t value) {
  return setThreshold(DP_LOW_PH_THRESHOLD, value);
}

int32_t TuyaWaterQuality::getMaxTDS() {
  return _sensorData.tds.MaxThreshold;
}

bool TuyaWaterQuality::setMaxTDS(int32_t value) {
  return setThreshold(DP_HIGH_TDS_THRESHOLD, value);
}

int32_t TuyaWaterQuality::getMinTDS() {
  return _sensorData.tds.MinThreshold;
}

bool TuyaWaterQuality::setMinTDS(int32_t value) {
  return setThreshold(DP_LOW_TDS_THRESHOLD, value);
}

void TuyaWaterQuality::onReceiveSensor(void (*callback)(TuyaWaterQualitySensor& sensor)) {
  _onReceiveSensor = callback;
}

// Private methods
bool TuyaWaterQuality::decodeReportStatusAsync(TuyaFrame& frame) {
  TuyaDataType dataType = static_cast<TuyaDataType>(frame.data[1]);
  if (dataType != DT_VALUE) {
    return false;
  }

  TuyaWaterQualityDataPoint dpId = static_cast<TuyaWaterQualityDataPoint>(frame.data[0]);
  switch (dpId) {
  case DP_TEMPERATURE:
    _sensorData.temperature.value = decodeSensorRawValue(frame.data) / 10.0;
    break;
  case DP_HIGH_TEMPERATURE_THRESHOLD:
    _sensorData.temperature.MaxThreshold = decodeSensorRawValue(frame.data) / 10.0;
    break;
  case DP_LOW_TEMPERATURE_THRESHOLD:
    _sensorData.temperature.MinThreshold = decodeSensorRawValue(frame.data) / 10.0;
    break;
  case DP_PH:
    _sensorData.ph.value = decodeSensorRawValue(frame.data) / 100.0;
    break;
  case DP_HIGH_PH_THRESHOLD:
    _sensorData.ph.MaxThreshold = decodeSensorRawValue(frame.data) / 100.0;
    break;
  case DP_LOW_PH_THRESHOLD:
    _sensorData.ph.MinThreshold = decodeSensorRawValue(frame.data) / 100.0;
    break;
  case DP_TDS:
    _sensorData.tds.value = decodeSensorRawValue(frame.data);
    break;
  case DP_HIGH_TDS_THRESHOLD:
    _sensorData.tds.MaxThreshold = decodeSensorRawValue(frame.data);
    break;
  case DP_LOW_TDS_THRESHOLD:
    _sensorData.tds.MinThreshold = decodeSensorRawValue(frame.data);
    break;
  default:
    return false;
  }

  if (_onReceiveSensor != nullptr) {
    _onReceiveSensor(_sensorData);
  }

  return true;
}

uint32_t TuyaWaterQuality::decodeSensorRawValue(uint8_t* data) {
  return (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
}

bool TuyaWaterQuality::setThreshold(TuyaWaterQualityDataPoint datapoint, int32_t value) {
  uint8_t data[8];
  if (!generateSensorData(data, datapoint, value)) {
    return false;
  }

  TuyaFrame frame = generateFrame(MODULE, SEND_COMMAND, data, 8);
  return sendFrame(frame);
}

bool TuyaWaterQuality::generateSensorData(uint8_t(&buffer)[8], TuyaWaterQualityDataPoint dataPoint, int32_t value) {
  const int VALUE_LENGTH = 4;

  buffer[0] = dataPoint;
  buffer[1] = DT_VALUE;
  buffer[2] = (VALUE_LENGTH >> 8) & 0xFF;
  buffer[3] = VALUE_LENGTH & 0xFF;
  buffer[4] = (value >> 24) & 0xFF;
  buffer[5] = (value >> 16) & 0xFF;
  buffer[6] = (value >> 8) & 0xFF;
  buffer[7] = value & 0xFF;

  return true;
}