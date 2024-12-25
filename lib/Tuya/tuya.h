#ifndef TUYA_H
#define TUYA_H

#include <Arduino.h>
#include <Stream.h>
#include <ArduinoJson.h>

// Enums for various Tuya types
enum TuyaErrorTransmission {
  ERROR_NONE = 0,
  ERROR_NO_DATA,
  ERROR_CHECKSUM,
  ERROR_OVERFLOW,
};

enum TuyaDataType {
  DT_RAW = 0x00,
  DT_BOOLEAN = 0x01,
  DT_VALUE = 0x02,
  DT_STRING = 0x03,
  DT_ENUM = 0x04,
  DT_BITMAP = 0x05,
};

enum TuyaDeviceType {
  MODULE = 0x00,
  MCU = 0x03,
};

enum TuyaCommandType {
  HEARTBEATS = 0x00,
  QUERY_PRODUCT_INFO = 0x01,
  QUERY_WORKING_MODE = 0x02,
  REPORT_NETWORK_STATUS = 0x03,
  RESET_WIFI = 0x04,
  RESET_WIFI_PAIR_MODE = 0x05,
  SEND_COMMAND = 0x06,
  REPORT_STATUS_ASYNC = 0x07,
  QUERY_DP_STATUS = 0x08,
  START_OTA = 0x0A,
  TRANSMIT_OTA_DATA = 0x0B,
  GET_GMT_TIME = 0x0C,
  TEST_WIFI_SCANNING = 0x0E,
  GET_MODULE_MEMORY = 0x0F,
  GET_LOCAL_TIME = 0x1C,
  ENABLE_WEATHER_SERVICES = 0x20,
  SEND_WEATHER_DATA = 0x21,
  REPORT_STATUS_SYNC = 0x22,
  RESPONSE_STATUS_SYNC = 0x23,
  GET_WIFI_SIGNAL_STRENGTH = 0x24,
  DISABLE_HEARTBEATS = 0x25,
  PAIR_VIA_SERIAL_PORT = 0x2A,
  GET_CURRENT_NETWORK_STATUS = 0x2B,
  TEST_WIFI_CONNECTION = 0x2C,
  GET_MODULE_MAC_ADDRESS = 0x2D,
  EXTENDED_SERVICES = 0x34,
  BLUETOOTH_PAIRING = 0x35,
  REPORT_SEND_EXTENDED_DP = 0x36,
  NEW_FEATURE_SETTING = 0x37,
};

enum TuyaNetworkStatus {
  PAIRING_EZ_MODE = 0x00,
  PAIRING_AP_MODE = 0x01,
  WIFI_NOT_CONNECTED = 0x02,
  WIFI_CONNECTED = 0x03,
  CLOUD_CONNECTED = 0x04,
  LOW_POWER_MODE = 0x05,
  EZ_AP_CONFIG_MODE = 0x06,
};

// Structs for Tuya data
struct TuyaFrame {
  uint8_t header[2];
  uint8_t version;
  uint8_t command;
  uint8_t length[2];
  uint8_t data[1024];
  uint8_t checksum;
};

struct TuyaProductInformation {
  String productId;
  String version;
  uint16_t operationMode;
};

struct TuyaModuleInformation {
  TuyaProductInformation information;
  TuyaNetworkStatus networkStatus;
  bool heartbeats;
  bool productInfo;
  bool workingMode;
  bool initialized;
};

// Tuya class definition
class Tuya {
public:
  Tuya();

  void begin(Stream* pSerial);
  void loop();

  void debug(Stream& stream, bool enable);
  void setDelay(uint32_t delay);
  void setNetworkStatus(TuyaNetworkStatus status);

  bool isInitialized() const;
  TuyaNetworkStatus getNetworkStatus() const;
  TuyaProductInformation getProductInformation() const;

  void onResetWiFiPairMode(void (*callback)());

protected:
  virtual bool decodeHeartbeats(TuyaFrame& frame);
  virtual bool decodeProductInfo(TuyaFrame& frame);
  virtual bool decodeQueryWorkingMode(TuyaFrame& frame);
  virtual bool decodeReportStatusAsync(TuyaFrame& frame);

  TuyaFrame generateFrame(TuyaDeviceType version, TuyaCommandType command, uint8_t* data, uint16_t dataLength) const;
  TuyaFrame generateFrame(TuyaDeviceType version, TuyaCommandType command) const;

  bool sendFrame(const TuyaFrame& frame) const;

private:
  uint32_t _delay;
  Stream* _pSerial;
  Stream* _pDebugSerial;
  TuyaModuleInformation _state;
  uint32_t intervalHeartbeats = 1000;
  uint32_t _lastHeartbeats = 0;
  bool _debug;
  void (*_onResetWiFiPairMode)();

  TuyaErrorTransmission listeningMessage(TuyaFrame& frame);
  bool validateChecksum(const TuyaFrame& frame) const;
  uint8_t generateChecksum(const TuyaFrame& frame) const;

  void decodeFrame(TuyaFrame& frame);
  void printFrame(const TuyaFrame& frame) const;

  void handleHeartbeats(TuyaFrame& frame);
  void handleQueryProductInfo(TuyaFrame& frame);
  void handleQueryWorkingMode(TuyaFrame& frame);
  void handleReportNetworkStatus(TuyaFrame& frame);
  void handleReportStatusAsync(TuyaFrame& frame);
  void handleGetCurrentNetworkStatus(TuyaFrame& frame);
  void handleResetWiFiPairMode(TuyaFrame& frame);
  void handleUnknownCommand(TuyaFrame& frame);

  void sendNetworkStatus() const;
  void reportNetworkStatus() const;
  void sendHeartbeats() const;
  void queryProductInfo() const;
  void queryWorkingMode() const;

  String hexToString(uint8_t* data, uint16_t length) const;
};

#endif // TUYA_H