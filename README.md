![Status](https://img.shields.io/badge/status-planning-yellow)
![License](https://img.shields.io/badge/license-MIT-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B-blue)

# 📟 Intelligent Vehicle Instrumentation Dashboard

## 📌 Overview
This project aims to develop an intelligent instrumentation dashboard for an autonomous electric vehicle, capable of real-time adaptive monitoring of critical system parameters.

## ⚙️ Description
The system is designed as an embedded solution responsible for collecting, processing, and displaying essential vehicle data such as speed, battery level, and temperature in real time.

The architecture integrates sensors, microcontrollers, and communication interfaces, with a strong focus on performance, reliability, and scalability.

## 🏗️ System Architecture
The system is divided into two main modules:

- **HMI (Human-Machine Interface)**
  - Hardware: Raspberry Pi 5 with touch display
- **Data Acquisition Module**
  - Hardware: ESP32 with sensors

This separation follows key software engineering principles:
- Low coupling
- High cohesion
- Separation of concerns

## 🚧 Project Status
Currently in the early planning and theoretical research phase.

## 🛠️ Technologies
- C/C++
- Embedded Systems
- Single Board Computers (SBC)
- Microcontrollers (ESP32)
- Sensors
- Touch Display
- Communication Protocols:
  - SPI
  - UART
  - CAN Bus (planned)

## 🔬 Engineering Decisions
- Use of **C++** for firmware development
- Modular architecture for scalability
- Use of cross-compilation tools
- Emphasis on testability and system validation
- Monitoring of:
  - Update rate
  - Power consumption
  - System stability

## 🎯 Future Work
- System architecture refinement
- Integration with real sensors
- Firmware implementation
- Bench testing and validation
- Performance optimization (latency, energy consumption, thermal behavior)

## 📚 Research Context
This project is part of a graduate research initiative focused on embedded systems and intelligent interfaces for automotive applications from the Siva research group.


## 📄 License
This project is licensed under the MIT License – see the LICENSE file for details.

---

Developed by **Maria Eduarda P. de Jesus**