# Autonomous Hospital Robot - Web Telemetry & Control Dashboard 🏥🤖

This repository contains the source code and web dashboard for the **"Design and Implementation of an Autonomous Hospital Robot for SLAM-Based Mapping and Patient Vital Monitoring."** This project was developed by **Tharun J, Manoj M, and Vimal Kumar S** under the guidance of **Prof. R.V. Nagarajan** at P.S.V. College of Engineering and Technology. The system has been successfully field-tested at Government Medical College and Hospital (GMCH), Krishnagiri.

🔗 **Live Dashboard:** [Insert your Netlify Link Here]
🔗 **Read the Published Paper:** [International Journal of Current Research (IJCR)](https://journalcra.com/article/design-and-implementation-autonomous-hospital-robot-slam-based-mapping-and-patient-vital)

---

## 🌟 Key Features

The project is a 3-in-1 autonomous robotic platform designed to enhance clinical safety and automate hospital workflows. The web dashboard provides full telemetry and control over the robot via a local Wi-Fi network.

### 1. Robot Telemetry & Controls
* **Live Sensor Feed:** Real-time data polling from the ESP32 for Obstacle Distance (Ultrasonic), Human Presence (PIR), and Air Quality (PPM).
* **Manual Overrides:** A web-based joypad to manually steer the robot and a slider to adjust DC motor speed (0-255 PWM).
* **Actuator Toggles:** Remote toggles for the **UV-C Disinfection Lamp** and **Mopping Water Pump**.
* **Emergency Stop:** A master kill-switch that instantly sends stop commands to all motors and halts operations.

### 2. Patient Registry & Automated Vaccine Alerts
* **RDBMS Integration:** Manages patient details, bed numbers, and cyclic vaccine schedules.
* **Automated Email Alerts:** Integrates **EmailJS** to automatically send notifications to doctors when a patient's next vaccine dose is due based on the interval timer.

---

## 🛠️ Tech Stack
* **Microcontroller:** ESP32 (C++ / Arduino IDE)
* **Frontend:** HTML5, CSS3, Vanilla JavaScript
* **Backend / API:** REST API (ESP32 Web Server)
* **Database:** Firebase Realtime Database (with LocalStorage fallback for demos)
* **Third-Party Services:** EmailJS (Automated Alerts)

---

## 📂 Folder Structure

```text
/
├── ESP32_Robot_Controller/      # C++ Arduino code for the ESP32 logic and SLAM
├── assets/                      # Contains logos, UI images, and circuit diagrams
│   ├── iso-logo.png
│   ├── psv-main-logo.png
│   └── robot-circuit.png
├── index.html                   # Main dashboard UI and JavaScript logic
└── README.md                    # Project documentation
