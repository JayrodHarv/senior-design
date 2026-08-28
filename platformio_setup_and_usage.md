# PlatformIO Setup and Project Workflow

This guide explains how to install the PlatformIO extension in VS Code and how to use PlatformIO when working with this repository after cloning it.

The goal is that every team member can clone the repository, open it in VS Code, let PlatformIO install the required toolchain/dependencies, build the project, connect an ESP32, upload the firmware, and use the serial monitor.

---

## 1. What is PlatformIO?

PlatformIO is the development environment we use to build and program the ESP32.

It handles:

- C/C++ compilation
- ESP32 toolchains
- Board definitions
- Frameworks such as Arduino
- External libraries
- Firmware builds
- Firmware uploads
- Serial monitoring
- Project configuration

The main configuration file is:

```text
platformio.ini
```

This file belongs in the root of the repository and defines the board, framework, libraries, upload settings, and other project settings.

**Do not create a separate PlatformIO project after cloning this repository.**

The repository already contains the PlatformIO project configuration.

---

# 2. Install VS Code

Install Visual Studio Code for your operating system.

After installing it, open VS Code.

---

# 3. Install the PlatformIO Extension

In VS Code:

1. Open the **Extensions** panel.
2. Search for:

```text
PlatformIO IDE
```

3. Install the official **PlatformIO IDE** extension.
4. Restart VS Code if prompted.

PlatformIO IDE includes PlatformIO Core, so you normally do not need to install PlatformIO Core separately when using the VS Code extension.

### Linux users

PlatformIO recommends having Python's `venv` support installed.

On Arch/CachyOS, this is typically provided by:

```bash
sudo pacman -S python-virtualenv
```

If PlatformIO reports a missing Python virtual-environment component, install the appropriate Python/venv package for your distribution.

---

# 4. Install Git

This project is stored in Git, so Git must be installed.

Check:

```bash
git --version
```

If that command works, Git is installed.

On Arch/CachyOS:

```bash
sudo pacman -S git
```

---

# 5. Clone the Repository

Open a terminal and navigate to the directory where you keep your projects.

For example:

```bash
cd ~/Projects
```

Clone the repository:

```bash
git clone <REPOSITORY_URL>
```

Then enter the repository:

```bash
cd <REPOSITORY_DIRECTORY>
```

For example:

```bash
git clone https://github.com/example/example-project.git
cd example-project
```

---

# 6. Open the Repository in VS Code

From inside the repository:

```bash
code .
```

Or:

1. Open VS Code.
2. Select **File → Open Folder**.
3. Select the cloned repository directory.

You should see the repository files in the VS Code Explorer.

A typical PlatformIO project will look similar to:

```text
project/
├── include/
├── lib/
├── src/
│   └── main.cpp
├── test/
├── platformio.ini
└── README.md
```

The exact files may differ depending on the project.

---

# 7. Let PlatformIO Initialize the Project

When VS Code opens the repository, PlatformIO reads:

```text
platformio.ini
```

PlatformIO uses this file to determine the project's board, framework, libraries, and build configuration.

The first time you open the project, PlatformIO may automatically download:

- The ESP32 development platform
- The required compiler/toolchain
- The Arduino framework
- Project dependencies
- Other required PlatformIO packages

This can take a few minutes the first time.

**Do not commit these downloaded dependencies to Git.**

They are normally stored in PlatformIO's user directories rather than inside the repository.

---

# 8. Understand `platformio.ini`

The `platformio.ini` file is the most important PlatformIO configuration file.

For example:

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino

monitor_speed = 115200
```

The important settings are:

### Environment

```ini
[env:esp32dev]
```

This defines a PlatformIO build environment.

### Platform

```ini
platform = espressif32
```

This tells PlatformIO that we are using Espressif's ESP32 platform.

### Board

```ini
board = esp32dev
```

This identifies the target development board.

**The actual repository may use a different board ID. Do not change this unless you know that the hardware has changed.**

### Framework

```ini
framework = arduino
```

This tells PlatformIO to use the Arduino framework.

### Serial monitor

```ini
monitor_speed = 115200
```

This configures the serial monitor to use 115200 baud.

---

# 9. Where to Put Code

Application source code normally belongs in:

```text
src/
```

The main program is commonly:

```text
src/main.cpp
```

Header files that are specific to the project can go in:

```text
include/
```

Project-specific libraries can go in:

```text
lib/
```

Tests go in:

```text
test/
```

Do not put source files in the repository root unless the project specifically requires it.

---

# 10. Build the Project

Building compiles the C/C++ source code and produces firmware for the ESP32.

### Using the PlatformIO interface

Open the PlatformIO sidebar.

Under:

```text
Project Tasks
```

find the project's environment.

Then select:

```text
General → Build
```

### Using the terminal

From the repository root:

```bash
pio run
```

A successful build should end with something similar to:

```text
========================= [SUCCESS] =========================
```

If the build fails, read the first actual error rather than only looking at the final error message.

---

# 11. Upload / Flash the ESP32

Connect the ESP32 to your computer with a USB **data** cable.

Then use:

```text
PlatformIO → Project Tasks → Upload
```

or:

```bash
pio run --target upload
```

PlatformIO will compile the project if necessary and then upload the resulting firmware to the ESP32.

A successful upload should end with:

```text
========================= [SUCCESS] =========================
```

---

# 12. Selecting the ESP32 Serial Port

PlatformIO can often detect the ESP32 automatically.

If it cannot find the board, check the available serial devices.

On Linux:

```bash
ls /dev/ttyUSB*
```

and:

```bash
ls /dev/ttyACM*
```

You can also use:

```bash
pio device list
```

Example:

```text
/dev/ttyUSB0
```

If necessary, the port can be specified in `platformio.ini`:

```ini
upload_port = /dev/ttyUSB0
```

However, **do not commit a machine-specific `upload_port` setting unless the project specifically requires it**. Different team members may have different port names.

---

# 13. ESP32 Boot Button

Some ESP32 boards automatically enter the bootloader when PlatformIO starts an upload.

If uploading fails with a message such as:

```text
Failed to connect to ESP32
```

try:

1. Start the PlatformIO upload.
2. When PlatformIO begins trying to connect, hold the **BOOT** button on the ESP32.
3. Keep holding it for a few seconds.
4. Release it once the upload begins.

The exact behavior depends on the ESP32 board.

---

# 14. Serial Monitor

The serial monitor allows you to see messages printed by the ESP32.

If the program contains:

```cpp
Serial.begin(115200);
```

and:

```cpp
Serial.println("Hello!");
```

you can view the output with:

```bash
pio device monitor
```

Or use:

```text
PlatformIO → Project Tasks → Monitor
```

The baud rate should match the value in the code and `platformio.ini`.

For example:

```ini
monitor_speed = 115200
```

---

# 15. Typical Development Workflow

For normal development, use this workflow:

```text
Clone repository
       ↓
Open repository in VS Code
       ↓
PlatformIO reads platformio.ini
       ↓
PlatformIO installs required dependencies
       ↓
Edit code
       ↓
Build
       ↓
Connect ESP32
       ↓
Upload
       ↓
Run/test firmware
       ↓
Use Serial Monitor
       ↓
Make changes
       ↓
Build + Upload again
```

---

# 16. Working With Git

PlatformIO projects should be committed to Git like normal software projects.

After making changes:

```bash
git status
```

Review what changed:

```bash
git diff
```

Stage changes:

```bash
git add .
```

Commit:

```bash
git commit -m "Describe the change"
```

Push:

```bash
git push
```

Before starting new work, it is a good idea to update your local repository:

```bash
git pull
```

---

# 17. Important: Don't Commit Build Files

PlatformIO generates build artifacts locally.

For example:

```text
.pio/
```

should generally **not** be committed to Git.

A `.gitignore` file should normally include:

```gitignore
.pio/
.vscode/.browse.c_cpp.db*
.vscode/c_cpp_properties.json
```

The exact `.gitignore` used by this project may contain additional entries.

The important idea is:

**Commit source code and project configuration, not generated build files.**

---

# 18. Installing Libraries

If the project needs an external library, it should normally be declared in `platformio.ini`.

For example:

```ini
lib_deps =
    bblanchon/ArduinoJson
```

After another developer clones the repository, PlatformIO can install the declared dependency automatically.

This is preferable to manually installing libraries on each computer.

**If you add a library to the project, update `platformio.ini` and commit that change.**

---

# 19. Don't Modify Project Configuration Without Coordinating

Because `platformio.ini` controls how everyone builds the firmware, changes to it can affect the entire team.

Before changing things such as:

```ini
board =
framework =
platform =
lib_deps =
build_flags =
upload_port =
```

make sure the change is intentional.

In particular, avoid committing your personal serial port:

```ini
upload_port = /dev/ttyUSB0
```

unless there is a specific reason for doing so.

---

# 20. Common Problems

## PlatformIO icon isn't appearing

Check that the official **PlatformIO IDE** extension is installed.

Restart VS Code after installation.

---

## PlatformIO is stuck installing dependencies

Wait a few minutes on the first setup.

Check your internet connection.

You can also restart VS Code and reopen the repository.

---

## Build fails because a library is missing

Check whether the library is listed in:

```text
platformio.ini
```

Do not immediately install random copies of libraries manually.

---

## ESP32 isn't detected

First check whether Linux detects the USB device at all:

```bash
lsusb
```

Then:

```bash
pio device list
```

If the ESP32 does not appear, check:

- USB cable
- USB port
- USB-to-serial chip
- Linux USB permissions
- ESP32 board
- USB drivers

A power-only USB cable is a common cause.

---

## Upload fails

Try:

1. Disconnect and reconnect the ESP32.
2. Verify the correct serial port.
3. Close any program currently using the serial port.
4. Try holding the ESP32 **BOOT** button during connection.
5. Try a different USB cable.

---

## Serial monitor shows garbage

The baud rate probably doesn't match.

For example, if the program uses:

```cpp
Serial.begin(115200);
```

the monitor must use:

```text
115200
```

---

# 21. Useful PlatformIO Commands

These commands are useful when working from the repository root.

### Build

```bash
pio run
```

### Upload

```bash
pio run --target upload
```

### Clean build files

```bash
pio run --target clean
```

### List connected devices

```bash
pio device list
```

### Open serial monitor

```bash
pio device monitor
```

### Show the computed project configuration

```bash
pio project config
```

---

# 22. Team Member Quick Start

After the initial project setup, a new team member should be able to do approximately this:

```bash
git clone <REPOSITORY_URL>
cd <REPOSITORY_DIRECTORY>
code .
```

Then:

1. Make sure the **PlatformIO IDE** extension is installed.
2. Wait for PlatformIO to initialize the project.
3. Connect the ESP32.
4. Open **PlatformIO → Project Tasks**.
5. Select **Build**.
6. Select **Upload**.
7. Select **Monitor** if serial output is needed.

That's it.

The repository's `platformio.ini` should contain the information necessary for everyone to build the same firmware environment.

---

# 23. Recommended Team Rules

For this project, follow these rules:

- **Do** commit `platformio.ini`.
- **Do** commit source code.
- **Do** add project libraries to `lib_deps`.
- **Do** use Git branches for larger features.
- **Do** pull before starting work.
- **Do** build before pushing major changes.
- **Do not** commit `.pio/`.
- **Do not** commit personal IDE settings unless the team intentionally shares them.
- **Do not** hard-code your personal serial port into the shared configuration.
- **Do not** manually copy library source into the project unless there is a specific reason.

The goal is that another team member can clone the repository and reproduce the same build environment with minimal manual setup.

---

## References

- PlatformIO VS Code documentation: https://docs.platformio.org/en/latest/integration/ide/vscode.html
- PlatformIO project configuration (`platformio.ini`): https://docs.platformio.org/en/latest/projectconf/
- PlatformIO project initialization: https://docs.platformio.org/en/latest/core/userguide/project/cmd_init.html
