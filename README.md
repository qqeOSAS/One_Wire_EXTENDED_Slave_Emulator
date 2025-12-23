# OneWire Extended Slave Emulator

[![PlatformIO](https://img.shields.io/badge/PlatformIO-compatible-green)](https://platformio.org/)

Author: qqeOSAS  
[PlatformIO Library Registry — OneWire Extended Slave Emulator](https://registry.platformio.org/libraries/qqeosas/One_Wire_EXTENDED_Slave_Emulator/installation)

---

       _____        ____  __        _                                         _       _             
      / _ \ \      / /\ \/ /    ___| | __ ___   _____     ___ _ __ ___  _   _| | __ _| |_ ___  _ __ 
     | | | \ \ /\ / /  \  /    / __| |/ _` \ \ / / _ \   / _ \ '_ ` _ \| | | | |/ _` | __/ _ \| '__|
     | |_| |\ V  V /   /  \    \__ \ | (_| |\ V /  __/  |  __/ | | | | | |_| | | (_| | || (_) | |   
      \___/  \_/\_/   /_/\_\___|___/_|\__,_| \_/ \___|___\___|_| |_| |_|\__,_|_|\__,_|\__\___/|_|   
                          |_____|                   |_____|


Short description
-----------------
A library for emulating OneWire devices with extended features. This project does not replicate any specific physical OneWire device; instead it implements a virtual slave device that communicates with a master using extended commands and flexible data packets.

Built on top of OneWireHub, this library gives more control over the OneWire bus and is useful for building modular or distributed systems where additional virtual peripherals are needed.

Key features
------------
- Emulates a OneWire slave device.
- Supports sending and receiving common data types: `int8`, `uint8`, `int16`, `uint16`, `int32`, `uint32`, `float32`, and custom structures.
- Scratchpad memory handling and packet transfer with CRC8 integrity checks.
- Custom command handling via callback: `setCustomHandler`.
- Simple API to check and read new incoming data: `available()`, `availableType()`, `clearAvailable()`.
- Getters for last received data: `getInt8()`, `getFloat()`, etc.
- Compatible with PlatformIO, Arduino, ESP8266 and similar platforms.
- Easy integration with OneWireHub and other bus devices.

When to use this library
------------------------
Use this library when you want to add virtual OneWire slaves to a system — for example, to expand the number of sensors/actuators without adding more GPIOs on the master controller. Each virtual slave can execute its own logic and return results to the master, enabling a distributed and modular architecture that is easy to scale and test.

Recommendation — OWX_master utilities (important)
-------------------------------------------------
When developing a project that uses the OWX_Slave_emulator on the slave side, I strongly recommend using a companion master-side utility library — OWX_master (or similar utilities). These utilities simplify command creation, packet parsing, and general master-slave interaction.

Why use OWX_master utilities:
- They reduce manual packet-building and parsing code on the master.
- They help ensure correct CRC and message formatting.
- They speed up development and reduce common integration bugs.

How to use:
- If you maintain a local repository with OWX_master utilities, add it as a dependency in your PlatformIO project.
- If you prefer an already published solution, check PlatformIO Library Registry or GitHub (search for OWX_master or visit my account: qqeOSAS) for available master-side utilities.
- Using OWX_master is not required, but it is highly recommended for robust and maintainable master-slave communication.

Installation
------------
You can install this library using PlatformIO.

PlatformIO:
Add to your `platformio.ini` under `lib_deps`:

```ini
lib_deps =
    https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator.git
```

Or install it from the PlatformIO Library Registry by searching for "OneWire Extended Slave Emulator".

Quick example
-------------
The following example shows a minimal sketch to initialize the emulator and read incoming data:

```cpp
#include <OneWireHub.h>
#include <OneWireExtendedSlave.h> // adjust include to actual header name in the library

OneWireHub hub(D2); // pin connected to the OneWire bus
OneWireExtendedSlave slave;

void setup() {
  Serial.begin(115200);
  hub.begin();
  slave.begin(&hub);
  // Optionally register a custom handler:
  // slave.setCustomHandler(myHandler);
}

void loop() {
  if (slave.available()) {
    int type = slave.availableType();
    if (type == TYPE_INT32) {
      int32_t val = slave.getInt32();
      Serial.println(val);
    } else if (type == TYPE_FLOAT32) {
      float f = slave.getFloat();
      Serial.println(f, 6);
    }
    slave.clearAvailable();
  }

  // your other code...
}
```

API (summary)
-------------
- begin(...) — initialize the slave.
- setCustomHandler(callback) — set a custom command handler.
- available(), availableType(), clearAvailable() — check and manage incoming data state.
- getInt8(), getUint16(), getFloat(), getStruct() — getters for received data.

See the library's header files and examples for full API details.

Tips
----
- Always validate CRC8 for packets to ensure integrity.
- Prefer using OWX_master utilities on the master side for reliable message formatting and parsing.
- Create small test slaves first to validate communication before integrating into a larger system.

Contributing and support
------------------------
Issues and pull requests are welcome. Please open an issue to report bugs or request features.

License
-------
Specify your project license here (for example, MIT).

---

If you want, I can also add a short example for the OWX_master utilities or prepare a PlatformIO example project that demonstrates both master and slave usage.
    https://github.com/qqeOSAS/One_Wire_EXTENDED_Slave_Emulator.git
