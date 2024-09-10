# Smart-Medicine-Storage-System
Overview
The Medi Box is a smart system designed to store medicine in a healthy environment by monitoring temperature and humidity levels. It includes an alarm system to remind users to take their medication on time, retrieves time from an NTP server over Wi-Fi, and provides real-time environmental monitoring. The system is built using the ESP32 microcontroller and integrates an OLED display for user interaction.

# Features
Time Zone Setup: Set the time zone by inputting the UTC offset.
Alarm System:
Set up to 3 alarms for medication reminders.
Disable all alarms when not needed.
Stop alarms using a push button.
Real-Time Clock: Fetches the current time from an NTP server and displays it on an OLED.
Environmental Monitoring:
Monitors temperature and humidity levels.
Displays warnings if conditions exceed healthy levels (Temperature: 26°C - 32°C, Humidity: 60% - 80%).
Indications:
Uses a buzzer, LED, and OLED messages to ring the alarm and provide warnings.

# Components
ESP32 Microcontroller
DHT22 Sensor for temperature and humidity
OLED Display for real-time time and environmental information
Buzzer and LED for alarm notifications
Push Button to stop alarms

# How It Works
The system allows users to set the time zone and up to three medication alarms through a menu displayed on the OLED.
It retrieves the current time from the NTP server, adjusting for the selected time zone.
The system continuously monitors the environment, ensuring the storage temperature and humidity remain within healthy limits. If the limits are exceeded, a warning is displayed, and alerts are triggered.
When it's time for a medication reminder, the alarm is triggered using sound and visual indicators, which can be stopped using the push button.
Usage
Set the time zone and alarms through the OLED menu.
The system will display the current time and monitor the environment.
Alarms will go off at the set times, and environmental warnings will be triggered if conditions exceed healthy limits.
Use the button to stop alarms manually.
