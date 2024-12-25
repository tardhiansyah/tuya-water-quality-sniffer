#include "tuya.h"

Tuya::Tuya()
  : _delay(250), _debug(false), _pSerial(nullptr), _pDebugSerial(nullptr), _lastHeartbeats(0) {
  _state = {
    .information = {
      .productId = "",
      .version = "",
      .operationMode = 0,
    },
    .networkStatus = WIFI_NOT_CONNECTED,
    .heartbeats = false,
    .productInfo = false,
    .workingMode = false,
    .initialized = false
  };
}

void Tuya::begin(Stream* pSerial) {
  _pSerial = pSerial;
}

void Tuya::loop() {
  if (_pSerial == nullptr) {
    return;
  }

  intervalHeartbeats = _state.heartbeats ? 15000 : 1000;
  if (millis() - _lastHeartbeats > intervalHeartbeats) {
    sendHeartbeats();
    _lastHeartbeats = millis();
  }

  if (_state.heartbeats && !_state.productInfo) {
    queryProductInfo();
  }

  if (_state.heartbeats && !_state.workingMode) {
    queryWorkingMode();
  }

  TuyaFrame frame;
  if (listeningMessage(frame) == ERROR_NONE) {
    decodeFrame(frame);
  }

  _state.initialized = _state.heartbeats && _state.productInfo && _state.workingMode;

  delay(_delay);
}

void Tuya::debug(Stream& stream, bool enable) {
  _debug = enable;
  _pDebugSerial = _debug ? &stream : nullptr;
}

void Tuya::setDelay(uint32_t delay) {
  _delay = delay;
}

bool Tuya::isInitialized() const {
  return _state.initialized;
}

TuyaNetworkStatus Tuya::getNetworkStatus() const {
  return _state.networkStatus;
}

TuyaProductInformation Tuya::getProductInformation() const {
  return _state.information;
}

void Tuya::onResetWiFiPairMode(void (*callback)()) {
  _onResetWiFiPairMode = callback;
}

TuyaErrorTransmission Tuya::listeningMessage(TuyaFrame& frame) {
  while (_pSerial->available()) {
    frame.header[0] = _pSerial->read();
    if (frame.header[0] == 0x55) {
      frame.header[1] = _pSerial->read();
      if (frame.header[1] == 0xAA) {
        frame.version = _pSerial->read();
        frame.command = _pSerial->read();
        frame.length[0] = _pSerial->read();
        frame.length[1] = _pSerial->read();
        uint16_t dataLength = (frame.length[0] << 8) | frame.length[1];
        if (dataLength > sizeof(frame.data)) {
          _pSerial->flush();
          return ERROR_OVERFLOW;
        }
        if (dataLength > 0) {
          _pSerial->readBytes(frame.data, dataLength);
        }
        frame.checksum = _pSerial->read();
        if (!validateChecksum(frame)) {
          _pSerial->flush();
          return ERROR_CHECKSUM;
        }
        _pSerial->flush();
        return ERROR_NONE;
      }
    }
  }
  return ERROR_NO_DATA;
}

bool Tuya::validateChecksum(const TuyaFrame& frame) const {
  uint16_t sum = frame.header[0] + frame.header[1] + frame.version + frame.command + frame.length[0] + frame.length[1];
  for (uint16_t i = 0; i < ((frame.length[0] << 8) | frame.length[1]); i++) {
    sum += frame.data[i];
  }
  return (sum % 256) == frame.checksum;
}

uint8_t Tuya::generateChecksum(const TuyaFrame& frame) const {
  uint16_t sum = frame.header[0] + frame.header[1] + frame.version + frame.command + frame.length[0] + frame.length[1];
  for (uint16_t i = 0; i < ((frame.length[0] << 8) | frame.length[1]); i++) {
    sum += frame.data[i];
  }
  return (sum % 256);
}

TuyaFrame Tuya::generateFrame(TuyaDeviceType version, TuyaCommandType command, uint8_t* data, uint16_t dataLength) const {
  TuyaFrame frame;
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
  return frame;
}

TuyaFrame Tuya::generateFrame(TuyaDeviceType version, TuyaCommandType command) const {
  TuyaFrame frame;
  frame.header[0] = 0x55;
  frame.header[1] = 0xAA;
  frame.version = version;
  frame.command = command;
  frame.length[0] = 0x00;
  frame.length[1] = 0x00;
  frame.checksum = generateChecksum(frame);
  return frame;
}

bool Tuya::sendFrame(const TuyaFrame& frame) const {
  _pSerial->write(frame.header[0]);
  _pSerial->write(frame.header[1]);
  _pSerial->write(frame.version);
  _pSerial->write(frame.command);
  _pSerial->write(frame.length[0]);
  _pSerial->write(frame.length[1]);
  _pSerial->write(frame.data, (frame.length[0] << 8) | frame.length[1]);
  _pSerial->write(frame.checksum);
  _pSerial->flush();
  return true;
}

void Tuya::decodeFrame(TuyaFrame& frame) {
  if (_debug) {
    printFrame(frame);
  }

  switch (frame.command) {
  case HEARTBEATS:
    handleHeartbeats(frame);
    break;
  case QUERY_PRODUCT_INFO:
    handleQueryProductInfo(frame);
    break;
  case QUERY_WORKING_MODE:
    handleQueryWorkingMode(frame);
    break;
  case REPORT_NETWORK_STATUS:
    handleReportNetworkStatus(frame);
    break;
  case REPORT_STATUS_ASYNC:
    handleReportStatusAsync(frame);
    break;
  case GET_CURRENT_NETWORK_STATUS:
    handleGetCurrentNetworkStatus(frame);
    break;
  case RESET_WIFI_PAIR_MODE:
    handleResetWiFiPairMode(frame);
    break;
  default:
    handleUnknownCommand(frame);
    break;
  }
}

void Tuya::printFrame(const TuyaFrame& frame) const {
  _pDebugSerial->print("Received frame: ");
  _pDebugSerial->print("header[0]: ");
  _pDebugSerial->print(frame.header[0], HEX);
  _pDebugSerial->print(", header[1]: ");
  _pDebugSerial->print(frame.header[1], HEX);
  _pDebugSerial->print(", version: ");
  _pDebugSerial->print(frame.version, HEX);
  _pDebugSerial->print(", command: ");
  _pDebugSerial->print(frame.command, HEX);
  _pDebugSerial->print(", length[0]: ");
  _pDebugSerial->print(frame.length[0], HEX);
  _pDebugSerial->print(", length[1]: ");
  _pDebugSerial->print(frame.length[1], HEX);
  _pDebugSerial->print(", data: ");
  for (uint16_t i = 0; i < ((frame.length[0] << 8) | frame.length[1]); i++) {
    _pDebugSerial->print(frame.data[i], HEX);
    _pDebugSerial->print(" ");
  }
  _pDebugSerial->print(", checksum: ");
  _pDebugSerial->println(frame.checksum, HEX);
}

void Tuya::setNetworkStatus(TuyaNetworkStatus status) {
  _state.networkStatus = status;
  reportNetworkStatus();
}

void Tuya::reportNetworkStatus() const {
  _pDebugSerial->println("Report network status");
  uint8_t data[1] = { _state.networkStatus };
  TuyaFrame frame = generateFrame(MODULE, REPORT_NETWORK_STATUS, data, sizeof(data));
  sendFrame(frame);
}

void Tuya::sendNetworkStatus() const {
  uint8_t data[1] = { _state.networkStatus };
  TuyaFrame frame = generateFrame(MODULE, GET_CURRENT_NETWORK_STATUS, data, sizeof(data));
  sendFrame(frame);
}

void Tuya::sendHeartbeats() const {
  TuyaFrame frame = generateFrame(MODULE, HEARTBEATS);
  sendFrame(frame);
}

void Tuya::queryProductInfo() const {
  TuyaFrame frame = generateFrame(MODULE, QUERY_PRODUCT_INFO);
  sendFrame(frame);
  delay(_delay);
}

void Tuya::queryWorkingMode() const {
  TuyaFrame frame = generateFrame(MODULE, QUERY_WORKING_MODE);
  sendFrame(frame);
  delay(_delay);
}

bool Tuya::decodeHeartbeats(TuyaFrame& frame) {
  return true;
}

bool Tuya::decodeProductInfo(TuyaFrame& frame) {
  uint16_t length = (frame.length[0] << 8) | frame.length[1];
  String productInfo = hexToString(frame.data, length);

  JsonDocument json;
  if (DeserializationError error = deserializeJson(json, productInfo)) {
    return false;
  }

  _state.information.productId = json["product_id"].as<String>();
  _state.information.version = json["version"].as<String>();
  _state.information.operationMode = json["operation_mode"].as<uint16_t>();

  return true;
}

bool Tuya::decodeQueryWorkingMode(TuyaFrame& frame) {
  return true;
}

bool Tuya::decodeReportStatusAsync(TuyaFrame& frame) {
  return true;
}

String Tuya::hexToString(uint8_t* data, uint16_t length) const {
  String result = "";
  for (uint16_t i = 0; i < length; i++) {
    result += char(data[i]);
  }
  return result;
}

void Tuya::handleHeartbeats(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received heartbeats");
  }
  _state.heartbeats = decodeHeartbeats(frame);
}

void Tuya::handleQueryProductInfo(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received query product info");
  }
  _state.productInfo = decodeProductInfo(frame);
}

void Tuya::handleQueryWorkingMode(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received query working mode");
  }
  _state.workingMode = decodeQueryWorkingMode(frame);
}

void Tuya::handleReportNetworkStatus(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received report network status");
  }
  // Do nothing for now
}

void Tuya::handleReportStatusAsync(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received report status");
  }
  decodeReportStatusAsync(frame);
}

void Tuya::handleGetCurrentNetworkStatus(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received get current network status");
  }
  sendNetworkStatus();
}

void Tuya::handleResetWiFiPairMode(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received reset WiFi pair mode");
  }

  if (_onResetWiFiPairMode != nullptr) {
    _onResetWiFiPairMode();
  }
}

void Tuya::handleUnknownCommand(TuyaFrame& frame) {
  if (_debug) {
    _pDebugSerial->println("Received unknown command");
  }
}