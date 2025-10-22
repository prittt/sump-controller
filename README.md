# Sump Controller

An basic IoT project to monitor and control the water level in a collecting sump using ESP8266 NodeMCU with email notifications and visual/audio alerts.

<p align="center">
  <img src="imgs/Img%2002.jpeg" width="19%" />
  <img src="imgs/Img%2004.jpeg" width="19%" />
  <img src="imgs/Img%2001.jpeg" width="19%" />
  <img src="imgs/Img%2010.jpeg" width="19%" />
  <img src="imgs/Img%2005.jpeg" width="19%" />
</p>

## Overview

This project implements a smart water level monitoring system for sump pumps or water collection basins. The system uses two float switches to detect water levels and provides real-time notifications via email, visual LED indicators, and an audible alarm. The ESP8266 NodeMCU connects to WiFi to send status updates to multiple recipients.

## Features

- **Dual Water Level Detection**: Uses two float switches (low and high) to monitor water levels
- **Email Notifications**: Sends automatic email alerts via Gmail SMTP for different water level conditions:
  - Critical: High water level reached
  - Warning: Low water level reached
  - Restoring: High float deactivated but low float still submerged
  - OK: System returned to normal
  - Weekly checkup: Automatic status email every week
- **Visual Indicators**: RGB LED provides color-coded status:
  - 🟢 Green: Normal operation
  - 🔵 Blue: Warning state
  - 🔴 Red: Critical state
- **Audio Alert**: Buzzer alarm for abnormal water levels with manual disable option
- **Alarm Control**: Additional button to temporarily disable/enable the audio alarm
- **Watchdog Protection**: Prevents ESP8266 crashes during operation

## Hardware Requirements

- ESP8266 NodeMCU development board
- 2x Float switches (low and high water level)
- RGB LED module (common cathode)
- Passive buzzer
- Push button (for alarm control)
- Connecting wires
- Power supply (5V USB or appropriate power source)

## Pin Configuration

| Component | GPIO Pin | NodeMCU Pin |
|-----------|----------|-------------|
| Low Float Switch | GPIO 12 | D6 |
| High Float Switch | GPIO 13 | D7 |
| Buzzer | GPIO 14 | D5 |
| Red LED | GPIO 5 | D1 |
| Blue LED | GPIO 4 | D2 |
| Green LED | GPIO 16 | D0 |
| Additional Button | GPIO 10 | SD3 |

## Circuit Diagram

The Fritzing schematic is available in the `fritzing/` folder. Additional custom modules used in the design:
- ESP8266 NodeMCU module
- Passive Buzzer module
- RGB Full Color LED module

## Software Dependencies

### Arduino Libraries Required

1. **ESP8266WiFi** - WiFi connectivity for ESP8266
2. **ESP_Mail_Client** - For sending emails via SMTP
3. **aplayer** - Arduino Buzzer Player library from [https://github.com/prittt/ArduinoBuzzerPlayer](https://github.com/prittt/ArduinoBuzzerPlayer)

### Installation

Install the required libraries through the Arduino IDE Library Manager or manually download them from their respective repositories.

## Configuration

### 1. Arduino IDE Setup for ESP8266

Follow the instructions here to enable programming of ESP8266 NodeMCU with Arduino IDE:
[Setting Up the Arduino IDE to Program ESP8266](https://www.instructables.com/Setting-Up-the-Arduino-IDE-to-Program-ESP8266/)

### 2. WiFi Configuration

Edit the following lines in `code/code.ino`:

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
```

### 3. Email Configuration

Configure your Gmail account for SMTP:

```cpp
#define AUTHOR_EMAIL "your_email@gmail.com"
#define AUTHOR_PASSWORD "your_app_password"
```

**Important**: Use an [App Password](https://support.google.com/accounts/answer/185833) instead of your regular Gmail password for security.

Add recipient emails and names:

```cpp
String emails[] = {"recipient1@example.com", "recipient2@example.com"};
size_t emails_size = sizeof(emails)/sizeof(String);
String names[]  = {"Name1", "Name2"};
```

### 4. Timing Configuration

Adjust the following constants as needed:

```cpp
#define STATUS_EMAIL_INTERVAL 604800000  // Weekly status email (in milliseconds)
#define WIFI_CONNECTION_TIMEOUT 15000    // WiFi connection timeout (15 seconds)
#define LOOP_DELAY_NORMAL_MODE 10000     // Normal mode delay (10 seconds)
#define LOOP_DELAY_PROBLEM_MODE 10000    // Problem mode delay (10 seconds)
```

## System States

The system operates in four different states:

1. **FINE (Green LED)**: Both float switches are inactive, water level is normal
2. **WARNING (Blue LED)**: Low float switch is activated, water level is rising
3. **WARNING_RESTORING (Blue LED)**: High float switch deactivated but low float still active
4. **CRITICAL (Red LED)**: High float switch is activated, water level is dangerously high

## Operation

### Normal Operation

1. When powered on, the system connects to WiFi and initializes (Green LED)
2. Continuously monitors both float switches
3. Sends a status email every week to confirm the system is running
4. Changes state and sends notifications when water level changes

### Alarm Control

- Press the additional button briefly to toggle the audio alarm on/off
- When disabled: LED blinks red 3 times
- When enabled: LED blinks green 3 times
- The visual LED status indicator remains active regardless of alarm state

### Email Notifications

The system sends emails for the following events:
- **CRITICAL**: High float switch triggered
- **WARNING**: Low float switch triggered
- **RESTORING?**: Transitioning from critical to warning state
- **OK**: System returned to normal
- **CHECKUP**: Weekly status confirmation (sent every 7 days)

## Debug Mode

To enable serial debug output, set:

```cpp
#define DEBUG true
```

This will print detailed status information to the Serial Monitor at 115200 baud.
