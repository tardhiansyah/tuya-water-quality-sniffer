# ESP32-based Sniffer for Tuya Water Quality MCU Messages

This repository provides code and resources for using an ESP32 development module to sniff and decode UART messages from a Tuya water quality MCU. 
This tool enables the interception, analysis, and interpretation of data sent by the MCU, giving insights into its communication protocols and data formats.

## Features
- **UART Message Sniffing**: Capture and decode messages sent via RX/TX.
- **Checksum Calculation**: Verify data integrity by calculating and comparing checksums.
- **Data Structure Interpretation**: Analyze and interpret key data points such as pH, temperature, and TDS levels.

## Requirements
- **Hardware**: ESP32 Dev Module, Tuya water quality MCU, Jumper wires
- **Software**: PlatformIO

## Schematic
To connect the ESP32 with the Tuya MCU, refer to the wiring schematic below:

![image](https://github.com/user-attachments/assets/cb184789-240d-4fef-a5ac-7fbf8a5cb7a7)

> **Note**: Ensure voltage compatibility and grounding between the ESP32 and the Tuya MCU to avoid damage to the components.

## Samples
Below are example outputs from the Serial Monitor, showing captured messages and their decoded content.

![image](https://github.com/user-attachments/assets/87ea550f-6134-4e59-ba16-4a1d7eb5c835)
