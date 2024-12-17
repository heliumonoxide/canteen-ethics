# Canteen Ethics Monitoring System

## Overview

![Banner Github Canteen Ethics](https://github.com/user-attachments/assets/78cd4ddb-257f-4527-b6e6-f19bd1293e9d)

The **Canteen Ethics Monitoring System** is an AI-powered solution designed to monitor and enforce ethics in canteen environments. It integrates various hardware, software, and cloud services to analyze, process, and respond to visual data collected in real time.

---

## Table of Contents

1. [Overview](#overview)
2. [System Architecture](#system-architecture)
    - [User Level](#1-user-level)
    - [Software Level](#2-software-level)
    - [Database Level](#3-database-level)
    - [Hardware Level](#4-hardware-level)
3. [Flow Diagram](#flow-diagram)
4. [Project Directory Structure](#project-directory-structure)
5. [Technologies Used](#technologies-used)
6. [How It Works](#how-it-works)
7. [Setup and Deployment](#setup-and-deployment)
    - [Frontend Setup](#frontend-setup)
    - [Backend Setup](#backend-setup)
    - [Google Cloud VM Configuration](#google-cloud-vm-configuration)
    - [ESP32 CAM Configuration](#esp32-cam-configuration)
    - [AI Model Training and Deployment](#ai-model-training-and-deployment)
8. [Credits](#credits)
9. [License](#License)

---

## System Architecture

The project follows a modular architecture divided into **four levels**:

### **1. User Level**
   - **Frontend**:  
     - Built with **React Vite** and styled using **Tailwind CSS**.
     - Deployed using **Vercel** for seamless and scalable hosting.
   - Purpose: Provides an intuitive interface for users to view monitoring results and system status.

### **2. Software Level**
   - **Backend**:  
     - Powered by **Express.js** with **Firebase Admin SDK**.
     - Deployed as a **Vercel Serverless Function**, optimized for fetching and displaying data.
   - **Google Cloud Compute Engine (VM Instance)**:  
     - Hosts Docker containers and manages image processing pipelines.
     - Includes a **supervisord** service for running monitoring scripts:
       - `scheduler_docker.py`: Schedules data uploads and synchronizations.
       - `watch_file_docker.py`: Monitors file changes and triggers the processing pipeline.

### **3. Database Level**
   - **Firestore Database & Firebase Storage**:  
     - Used for storing processed results, metadata, and uploaded images.
     - Acts as a centralized data hub for analysis and reporting.

### **4. Hardware Level**
   - **ESP32 CAM**:  
     - Captures images and transfers them to the server via **FTP**.
   - **AI Level**:  
     - **YOLOv5x AI Model**:
       - Trained to detect specific behaviors and objects in the images.
       - Developed and tested using Docker, ensuring consistent environments for training and deployment.

---

## Flow Diagram

![FlowDiagram drawio](https://github.com/user-attachments/assets/d1a4ecd8-cb68-480d-acc6-70ca5161f916)

The flow diagram illustrates the interaction between various components of the system.

---

## Project Directory Structure

```plaintext
canteen-ethics-monitoring/
├── .git/                    # Git repository for version control
├── CameraWebServer/         # For testing camera OV2640 via local IP
├── client/                  # Frontend application (React Vite + Tailwind CSS)
├── esp32_cam/               # ESP32 CAM scripts for capturing and transferring images
├── fomo_esp32/              # ESP32 CAM scripts for edge computing using FOMO model AI builtins // Just for testing
├── vm_config_notes/         # Notes for configuring the Google Cloud VM instance
├── webapi/                  # Backend API built with Express.js for handling data and Firebase integration
├── yolov5/                  # Main algorithm of the model inference, tracking state, and firebase integration
│   ├── scheduler_docker.py  # Schedules data upload
│   └── watch_file_docker.py # Watches for file changes to trigger processing
├── FlowDiagram.drawio       # Editable flow diagram of the system
├── FlowDiagram.drawio.png   # PNG version of the system flow diagram
├── LICENSE                  # License file for the project
└── README.md                # Project documentation (this file)
```

---

## Technologies Used

1. **Frontend**:  
   - React Vite
   - Tailwind CSS
   - Vercel for hosting

2. **Backend**:  
   - Express.js
   - Firebase Admin SDK
   - Vercel Serverless Functions

3. **Database**:  
   - Firestore Database
   - Firebase Storage

4. **Hardware**:  
   - ESP32 CAM for image capturing

5. **AI**:  
   - YOLOv5x for image recognition and behavior analysis
   - Docker for consistent development environments

6. **Cloud**:  
   - Google Cloud Compute Engine (VM) for hosting Dockerized services

7. **File Transfer**:  
   - FTP for transferring images from ESP32 CAM to the VM instance

---

## How It Works

1. **Image Capture**:  
   - ESP32 CAM captures images in the canteen environment.
   - Images are transferred to the Google Cloud VM instance via FTP.

2. **Monitoring & Processing**:  
   - `watch_file_docker.py` detects new images in the VM directory.
   - The trained YOLOv5s model processes the images for specific behaviors.
   - Violation behavior will be save into tracking_state.json
   - Results photo are stored in Firebase.

3. **Scheduling Tasks**:  
   - `scheduler_docker.py` handles periodic tasks, such as data upload from tracking_state.json.

4. **User Interaction**:  
   - Frontend displays processed results, accessible to users through a Vercel-hosted app.

---

## Setup and Deployment

### **Frontend Setup**
1. Navigate to the `client/` directory:
   ```bash
   cd client
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Start the development server:
   ```bash
   npm run dev
   ```

### **Backend Setup**
1. Navigate to the backend directory (if applicable):
   ```bash
   cd backend
   ```
2. Install dependencies:
   ```bash
   npm install
   ```
3. Deploy as a Vercel Serverless Function

### **Google Cloud VM Configuration**
1. Create a Google Cloud VM instance with Debian 11.
2. Install Docker and Supervisord:
   ```bash
   sudo apt update
   sudo apt install docker.io supervisor
   ```
3. Deploy the Docker containers:
   ```bash
   docker build -t canteen-monitor .
   docker run -d --name monitor-container canteen-monitor
   ```

### **ESP32 CAM Configuration**
1. Flash the ESP32 CAM with the provided script in the `esp32_cam/` directory.
2. Configure the Wi-Fi credentials and FTP details in the script.

### **AI Model Training and Deployment**
1. Navigate to the `yolov5/` directory:
   ```bash
   cd yolov5
   ```
2. Install dependencies in a virtual environment:
   ```bash
   python -m venv venv
   source venv/bin/activate
   pip install -r requirements.txt
   ```
3. Deploy the trained model in the Docker container using dockerfile.

---

## Credits

Special thanks to the following for their contributions and resources that made this project possible:
- Firebase for providing a robust database and storage solution.
- Google Cloud for their reliable infrastructure.
- Leonardo Bispo Team: big thanks to Leonardo Bispo in https://github.com/ldab for developing ESP32_FTPClient.

## License
This project is licensed under the MIT License. See `LICENSE` for more details.
