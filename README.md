# ESP32-based Sniffer for Tuya Water Quality MCU Messages

This repository provides code and resources for using an ESP32 development module to sniff and decode UART messages from a Tuya water quality MCU. 
This tool enables the interception, analysis, and interpretation of data sent by the MCU, giving insights into its communication protocols and data formats.

![image](https://github.com/user-attachments/assets/ecb3e75d-d8b2-45e2-a19d-f85f5a9ba3bd)


## Features
- **UART Message Sniffing**: Capture and decode messages sent via RX/TX.
- **Set Threshold**: Set new threshold for pH, temperature, and TDS levels without Tuya Apps. 
- **Checksum Calculation**: Verify data integrity by calculating and comparing checksums.
- **Data Structure Interpretation**: Analyze and interpret key data points such as pH, temperature, and TDS levels.

## Requirements
- **Hardware**: ESP32 Dev Module, Tuya water quality MCU, Jumper wires, 2 Diodes, and a 10k resistor
- **Software**: PlatformIO

## Schematic
To connect the ESP32 with the Tuya MCU, refer to the wiring schematic below:

![image](https://github.com/user-attachments/assets/8abe70a9-f820-483c-9f06-85a581a5c0dd)

> **Note**: Ensure voltage compatibility and grounding between the ESP32 and the Tuya MCU to avoid damage to the components.

## Samples
Below are example outputs from the Serial Monitor, showing captured messages and their decoded content.

![image](https://github.com/user-attachments/assets/b943d988-4506-4126-95c5-118df1564b31)
