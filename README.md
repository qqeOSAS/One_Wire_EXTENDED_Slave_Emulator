# OneWire Extended Slave Emulator

[![PlatformIO](https://img.shields.io/badge/PlatformIO-compatible-green)](https://platformio.org/)

**Author:** qqeOSAS  
**Repository:** [https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator](https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator)

---

       _____        ____  __        _                                         _       _             
      / _ \ \      / /\ \/ /    ___| | __ ___   _____     ___ _ __ ___  _   _| | __ _| |_ ___  _ __ 
     | | | \ \ /\ / /  \  /    / __| |/ _` \ \ / / _ \   / _ \ '_ ` _ \| | | | |/ _` | __/ _ \| '__|
     | |_| |\ V  V /   /  \    \__ \ | (_| |\ V /  __/  |  __/ | | | | | |_| | | (_| | || (_) | |   
      \___/  \_/\_/   /_/\_\___|___/_|\__,_| \_/ \___|___\___|_| |_| |_|\__,_|_|\__,_|\__\___/|_|   
                          |_____|                   |_____|



**Library for emulating OneWire devices with extended features.**  
Built over the OneWireHub library to give more access to the OneWire bus.  
This library does **not copy any existing OneWire device**, but creates a **virtual device** that communicates with a master using extended commands.  
It creates a flexible slave device that can receive various data types and respond accordingly.

---

## Features

- Implements a OneWire device emulator.
- Supports sending and receiving various data types: `int8`, `uint8`, `int16`, `uint16`, `int32`, `uint32`, `float32`, and structures.
- Handles scratchpad memory and packet transmission with CRC8.
- Allows custom command handling via callback (`setCustomHandler`).
- API for checking new data availability: `available()`, `availableType()`, `clearAvailable()`.
- Easy access to the last received data through getters: `getInt8()`, `getFloat()`, etc.
- Fully compatible with PlatformIO and Arduino/ESP8266.
- Easy integration with OneWireHub and other bus devices.

---

## When to Use

This library is particularly useful in large or modular projects where the main controller has limited pins or resources.  
For example, if the master controller lacks enough pins to handle all sensors or actuators, you can add several emulator slaves. Each slave processes its specific task and sends results back to the master.  
This enables a **distributed, modular architecture**, making it easy to scale the system and simplifying testing.

---

## Installation
You an find this library in offcial Platformio register by searching `OneWire Extended Slave Emulator`

Or include it from github directly


### PlatformIO

Add to your `platformio.ini` under `lib_deps`:

```ini
lib_deps =
    https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator.git
