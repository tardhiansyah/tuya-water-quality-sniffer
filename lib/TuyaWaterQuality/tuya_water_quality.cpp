#include "tuya_water_quality.h"

TuyaWaterQuality::TuyaWaterQuality(HardwareSerial& serial)
  : _serial(serial), _callbackFrame(nullptr), _callbackSensor(nullptr), _delay(250) {
  _sensorData = {
    {0, 0, 0},
    {0, 0, 0},
    {0, 0, 0},
  };
}

// Public methods
void TuyaWaterQuality::begin(uint8_t rxPin, uint8_t txPin) {
  _serial.begin(9600, SERIAL_8N1, rxPin, txPin);
}

void TuyaWaterQuality::loop() {
  TuyaFrame frame;

  if (listeningMessage(frame) == ERROR_NONE) {
    setLatestTuyaFrame(frame);
    if (decodeSensorData(frame)) {
      invokeCallbackSensor(_sensorData);
    }

    invokeCallbackFrame(frame);
  }

  delay(_delay);
}

void TuyaWaterQuality::setDelay(uint32_t delay) {
  _delay = delay;
}

TuyaFrame TuyaWaterQuality::getLatestTuyaFrame() {
  return _newFrame;
}

float TuyaWaterQuality::getTemperature() {
  return _sensorData.temperature.value;
}

float TuyaWaterQuality::getPh() {
  return _sensorData.ph.value;
}

int32_t TuyaWaterQuality::getTds() {
  return _sensorData.tds.value;
}

float TuyaWaterQuality::getTemperatureHighThreshold() {
  return _sensorData.temperature.highThreshold;
}

bool TuyaWaterQuality::setTemperatureHighThreshold(int32_t value) {
  return setSensorData(DP_HIGH_TEMPERATURE_THRESHOLD, value);
}

float TuyaWaterQuality::getTemperatureLowThreshold() {
  return _sensorData.temperature.lowThreshold;
}

bool TuyaWaterQuality::setTemperatureLowThreshold(int32_t value) {
  return setSensorData(DP_LOW_TEMPERATURE_THRESHOLD, value);
}

float TuyaWaterQuality::getPhHighThreshold() {
  return _sensorData.ph.highThreshold;
}

bool TuyaWaterQuality::setPhHighThreshold(int32_t value) {
  return setSensorData(DP_HIGH_PH_THRESHOLD, value);
}

float TuyaWaterQuality::getPhLowThreshold() {
  return _sensorData.ph.lowThreshold;
}

bool TuyaWaterQuality::setPhLowThreshold(int32_t value) {
  return setSensorData(DP_LOW_PH_THRESHOLD, value);
}

int32_t TuyaWaterQuality::getTdsHighThreshold() {
  return _sensorData.tds.highThreshold;
}

bool TuyaWaterQuality::setTdsHighThreshold(int32_t value) {
  return setSensorData(DP_HIGH_TDS_THRESHOLD, value);
}

int32_t TuyaWaterQuality::getTdsLowThreshold() {
  return _sensorData.tds.lowThreshold;
}

bool TuyaWaterQuality::setTdsLowThreshold(int32_t value) {
  return setSensorData(DP_LOW_TDS_THRESHOLD, value);
}

void TuyaWaterQuality::setCallbackFrame(void (*callback)(TuyaFrame& frame)) {
  _callbackFrame = callback;
}

void TuyaWaterQuality::setCallbackSensor(void (*callback)(TuyaSensorData& sensor)) {
  _callbackSensor = callback;
}

// Private methods
TuyaErrorTransmission TuyaWaterQuality::listeningMessage(TuyaFrame& frame) {
  while (_serial.available()) {
    frame.header[0] = _serial.read();
    if (frame.header[0] == 0x55) {
      frame.header[1] = _serial.read();
      if (frame.header[1] == 0xAA) {
        frame.version = _serial.read();
        frame.command = _serial.read();

        frame.length[0] = _serial.read();
        frame.length[1] = _serial.read();
        uint16_t dataLength = (frame.length[0] << 8) | frame.length[1];
        if (dataLength > sizeof(frame.data)) {
          return ERROR_OVERFLOW;
        }
        if (dataLength > 0) {
          _serial.readBytes(frame.data, dataLength);
        }

        frame.checksum = _serial.read();
        if (!validateChecksum(frame)) {
          return ERROR_CHECKSUM;
        }

        return ERROR_NONE;
      }
    }
  }

  return ERROR_NO_DATA;
}

bool TuyaWaterQuality::decodeSensorData(TuyaFrame& frame) {
  if (frame.command != 0x07) {
    return false;
  }

  if (frame.data[1] != DT_VALUE) {
    return false;
  }

  uint8_t dpId = frame.data[0];
  switch (dpId) {
  case DP_TEMPERATURE:
    _sensorData.temperature.value = getSensorRawValue(frame.data) / 10.0;
    return true;
  case DP_HIGH_TEMPERATURE_THRESHOLD:
    _sensorData.temperature.highThreshold = getSensorRawValue(frame.data) / 10.0;
    return true;
  case DP_LOW_TEMPERATURE_THRESHOLD:
    _sensorData.temperature.lowThreshold = getSensorRawValue(frame.data) / 10.0;
    return true;

  case DP_PH:
    _sensorData.ph.value = getSensorRawValue(frame.data) / 100.0;
    return true;
  case DP_HIGH_PH_THRESHOLD:
    _sensorData.ph.highThreshold = getSensorRawValue(frame.data) / 100.0;
    return true;
  case DP_LOW_PH_THRESHOLD:
    _sensorData.ph.lowThreshold = getSensorRawValue(frame.data) / 100.0;
    return true;

  case DP_TDS:
    _sensorData.tds.value = getSensorRawValue(frame.data);
    return true;
  case DP_HIGH_TDS_THRESHOLD:
    _sensorData.tds.highThreshold = getSensorRawValue(frame.data);
    return true;
  case DP_LOW_TDS_THRESHOLD:
    _sensorData.tds.lowThreshold = getSensorRawValue(frame.data);
    return true;

  default:
    return false;
  }
}

float TuyaWaterQuality::getSensorRawValue(uint8_t* data) {
  return (data[4] << 24) | (data[5] << 16) | (data[6] << 8) | data[7];
}

bool TuyaWaterQuality::validateChecksum(TuyaFrame& frame) {
  uint16_t sum = 0;
  sum += frame.header[0];
  sum += frame.header[1];
  sum += frame.version;
  sum += frame.command;
  sum += frame.length[0];
  sum += frame.length[1];
  for (uint16_t i = 0; i < ((frame.length[0] << 8) | frame.length[1]); i++) {
    sum += frame.data[i];
  }
  return (sum % 256) == frame.checksum;
}

uint8_t TuyaWaterQuality::generateChecksum(TuyaFrame& frame) {
  uint16_t sum = 0;
  sum += frame.header[0];
  sum += frame.header[1];
  sum += frame.version;
  sum += frame.command;
  sum += frame.length[0];
  sum += frame.length[1];
  for (uint16_t i = 0; i < ((frame.length[0] << 8) | frame.length[1]); i++) {
    sum += frame.data[i];
  }
  return (sum % 256);
}

void TuyaWaterQuality::setLatestTuyaFrame(TuyaFrame& frame) {
  _newFrame = frame;
}

void TuyaWaterQuality::invokeCallbackFrame(TuyaFrame& frame) {
  if (_callbackFrame != nullptr) {
    _callbackFrame(frame);
  }
}

void TuyaWaterQuality::invokeCallbackSensor(TuyaSensorData& sensor) {
  if (_callbackSensor != nullptr) {
    _callbackSensor(sensor);
  }
}

bool TuyaWaterQuality::setSensorData(TuyaDataPoint datapoint, int32_t value) {
  uint8_t data[8];
  TuyaFrame frame;

  if (!generateSensorData(data, datapoint, value)) {
    return false;
  }

  if (!generateFrame(frame, 0x00, 0x06, data, 8)) {
    return false;
  }

  if (!sendFrame(frame)) {
    return false;
  }

  return true;
}

bool TuyaWaterQuality::generateSensorData(uint8_t(&buffer)[8], TuyaDataPoint dataPoint, int32_t value) {
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

bool TuyaWaterQuality::generateFrame(TuyaFrame& frame, uint8_t version, uint8_t command, uint8_t* data, uint16_t dataLength) {
  if (dataLength > sizeof(frame.data)) {
    return false;
  }

  frame.header[0] = 0x55;
  frame.header[1] = 0xAA;
  frame.version = version;
  frame.command = command;
  frame.length[0] = (dataLength >> 8) & 0xFF;
  frame.length[1] = dataLength & 0xFF;
  if (dataLength > 0) {
    memcpy(frame.data, data, dataLength);
  }
  frame.checksum = generateChecksum(frame);

  return true;
}

bool TuyaWaterQuality::generateFrame(TuyaFrame& frame, uint8_t version, uint8_t command) {
  frame.header[0] = 0x55;
  frame.header[1] = 0xAA;
  frame.version = version;
  frame.command = command;
  frame.length[0] = 0x00;
  frame.length[1] = 0x00;
  frame.checksum = generateChecksum(frame);

  return true;
}

bool TuyaWaterQuality::sendFrame(TuyaFrame& frame) {
  _serial.write(frame.header[0]);
  _serial.write(frame.header[1]);
  _serial.write(frame.version);
  _serial.write(frame.command);
  _serial.write(frame.length[0]);
  _serial.write(frame.length[1]);
  _serial.write(frame.data, (frame.length[0] << 8) | frame.length[1]);
  _serial.write(frame.checksum);

  delay(_delay);

  return true;
}
