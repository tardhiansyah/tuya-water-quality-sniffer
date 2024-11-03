#ifndef TUYA_WATER_QUALITY_H
#define TUYA_WATER_QUALITY_H

#include <Arduino.h>
#include <HardwareSerial.h>

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

enum TuyaDataPoint {
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

struct TuyaFrame {
  uint8_t header[2];
  uint8_t version;
  uint8_t command;
  uint8_t length[2];
  uint8_t data[1024];
  uint8_t checksum;
};

struct SensorData {
  float value;
  float highThreshold;
  float lowThreshold;
};

struct TuyaSensorData {
  SensorData temperature;
  SensorData ph;
  SensorData tds;
};

class TuyaWaterQuality {
public:
  /// @brief Construct a new Tuya Water Quality object
  /// @param serial HardwareSerial object for communication
  TuyaWaterQuality(HardwareSerial& serial);

  /// @brief Start the Tuya Water Quality sensor sniffer
  /// @param rxPin HardwareSerial RX pin
  /// @param txPin HardwareSerial TX pin
  void begin(uint8_t rxPin, uint8_t txPin);

  /// @brief Call this function in the loop
  void loop();

  /// @brief Get the Temperature value
  /// @return Temperature value in Celsius 
  float getTemperature();

  /// @brief Get the PH value
  /// @return PH value (0-14)
  float getPh();

  /// @brief Get the TDS value
  /// @return TDS value in ppm
  int32_t getTds();

  /// @brief Get the Temperature high threshold value
  /// @return Temperature value in Celsius
  float getTemperatureHighThreshold();

  /// @brief Set the Temperature high threshold value of the sensor
  /// @param value Temperature * 10 (Celsius)
  /// @return true if success, false if failed
  bool setTemperatureHighThreshold(int32_t value);

  /// @brief Get the Temperature low threshold value
  /// @return Temperature value in Celsius
  float getTemperatureLowThreshold();

  /// @brief Set the Temperature low threshold value of the sensor
  /// @param value Temperature * 10 (Celsius)
  /// @return true if success, false if failed
  bool setTemperatureLowThreshold(int32_t value);

  /// @brief Get the PH high threshold value
  /// @return PH value (0-14)
  float getPhHighThreshold();

  /// @brief Set the PH high threshold value of the sensor
  /// @param value PH * 100
  /// @return true if success, false if failed
  bool setPhHighThreshold(int32_t value);

  /// @brief Get the PH low threshold value
  /// @return PH value (0-14)
  float getPhLowThreshold();

  /// @brief Set the PH low threshold value of the sensor
  /// @param value PH * 100
  /// @return true if success, false if failed
  bool setPhLowThreshold(int32_t value);

  /// @brief Get the TDS high threshold value
  /// @return TDS value in ppm
  int32_t getTdsHighThreshold();

  /// @brief Set the TDS high threshold value of the sensor
  /// @param value TDS value in ppm
  /// @return true if success, false if failed
  bool setTdsHighThreshold(int32_t value);

  /// @brief Get the TDS low threshold value
  /// @return TDS value in ppm
  int32_t getTdsLowThreshold();

  /// @brief Set the TDS low threshold value of the sensor
  /// @param value TDS value in ppm
  /// @return true if success, false if failed
  bool setTdsLowThreshold(int32_t value);

  /// @brief Set the callback function for frame
  /// @param callback function to be called when a new frame is received
  void setCallbackFrame(void (*callback)(TuyaFrame& frame));

  /// @brief Set the callback function for sensor
  /// @param callback function to be called when a new sensor data is received
  void setCallbackSensor(void (*callback)(TuyaSensorData& sensor));

  /// @brief Set the delay for the loop function, default is 250ms
  /// @param delay delay in milliseconds
  void setDelay(uint32_t delay);

  /// @brief Get the latest TuyaFrame
  /// @return TuyaFrame object
  TuyaFrame getLatestTuyaFrame();

private:
  TuyaSensorData _sensorData;
  TuyaFrame _newFrame;
  HardwareSerial& _serial;
  uint32_t _delay;
  void (*_callbackFrame)(TuyaFrame& frame);
  void (*_callbackSensor)(TuyaSensorData& sensor);

  /// @brief Listen for new message from the tuya mcu
  /// @param frame TuyaFrame object to store the new message
  /// @return TuyaErrorTransmission error code
  TuyaErrorTransmission listeningMessage(TuyaFrame& frame);

  /// @brief Get the raw value from the sensor data point
  /// @param data Decoded TuyaFrame's sensor data
  /// @return float raw value
  float getSensorRawValue(uint8_t* data);

  /// @brief Decode the sensor data from the received frame
  /// @param frame TuyaFrame object to decode
  /// @return true if success, false if failed
  bool decodeSensorData(TuyaFrame& frame);

  /// @brief Validate the checksum of the frame
  /// @param frame TuyaFrame
  /// @return true if valid, false if not valid
  bool validateChecksum(TuyaFrame& frame);

  /// @brief Generate the checksum for the frame
  /// @param frame TuyaFrame
  /// @return Checksum value
  uint8_t generateChecksum(TuyaFrame& frame);

  /// @brief Invoke the callback function for frame
  /// @param frame Buffer to store the frame
  void invokeCallbackFrame(TuyaFrame& frame);

  /// @brief Invoke the callback function for sensor data
  /// @param sensor Buffer to store the sensor data
  void invokeCallbackSensor(TuyaSensorData& sensor);

  /// @brief Set the latest TuyaFrame
  /// @param frame latest received TuyaFrame
  void setLatestTuyaFrame(TuyaFrame& frame);

  /// @brief Set the sensor data point value
  /// @param datapoint Target TuyaDataPoint
  /// @param value Value to be set
  /// @return true if success, false if failed
  bool setSensorData(TuyaDataPoint datapoint, int32_t value);

  /// @brief Generate new TuyaFrame
  /// @param frame Buffer to store the generated frame
  /// @param version Version of the frame (0x00: indicate tuya module, 0x03: indicate tuya mcu)
  /// @param command Command of the frame
  /// @param data Data to be included in the frame (max 1024 bytes)
  /// @param dataLength Length of the data
  /// @return true if success, false if failed
  bool generateFrame(TuyaFrame& frame, uint8_t version, uint8_t command, uint8_t* data, uint16_t dataLength);

  /// @brief Generate new TuyaFrame
  /// @param frame Buffer to store the generated frame
  /// @param version Version of the frame (0x00: indicate tuya module, 0x03: indicate tuya mcu)
  /// @param command Command of the frame
  /// @return true if success, false if failed
  bool generateFrame(TuyaFrame& frame, uint8_t version, uint8_t command);

  /// @brief Generate Sensor Data to be included in the TuyaFrame
  /// @param buffer Buffer to store the generated sensor data
  /// @param dataPoint target data point
  /// @param value data point value
  /// @return true if success, false if failed
  bool generateSensorData(uint8_t(&buffer)[8], TuyaDataPoint dataPoint, int32_t value);

  /// @brief Send the frame to the tuya mcu
  /// @param frame TuyaFrame object to send
  /// @return true if success, false if failed
  bool sendFrame(TuyaFrame& frame);
};

#endif