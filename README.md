# One_Wire_EXTENDED_Slave_Emulator


[![PlatformIO](https://img.shields.io/badge/PlatformIO-compatible-green)](https://platformio.org/)

**Author:** qqeOSAS  
**Repository:** [https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator](https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator)

---

## Overview

This library provides an **emulator for OneWire devices** with extended features.  
It is built on top of the [OneWireHub](https://github.com/qqeOSAS/OneWireHub) library, offering more control and flexibility on the OneWire bus.

The emulator creates a **virtual slave device** that can communicate with a OneWire master using extended commands.  
It supports multiple data types, scratchpad memory, CRC8-based communication, and custom command handling.

---

## Features

- Emulates a OneWire device.
- Supports sending and receiving multiple data types:  
  `int8`, `uint8`, `int16`, `uint16`, `int32`, `uint32`, `float32`, and custom structures.
- Scratchpad memory handling and packet transmission with CRC8.
- Custom command handling via callback (`setCustomHandler`).
- API for checking data availability:  
  `available()`, `availableType()`, `clearAvailable()`.
- Easy access to the last received data through getters:  
  `getInt8()`, `getFloat()`, `getUInt16()`, etc.
- Fully compatible with PlatformIO and Arduino/ESP8266.
- Easy integration with OneWireHub and other OneWire devices.

---

## Installation

### PlatformIO

Add to your `platformio.ini` under `lib_deps`:

```ini
lib_deps =
    https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator.git

