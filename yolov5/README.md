# YOLOv5 Docker Project Guide

This guide provides step-by-step instructions to build and run a Docker image for a YOLOv5-based project. It includes commands for local development and deployment on a virtual machine (VM).

---

## Build the Docker Image

To create a Docker image from your project, use the following command:

```bash
docker build -t yolov5-project .
```

---

## Run the Docker Image Locally

To create and run a container from the Docker image while sharing the entire project directory with the host:

```bash
docker run --privileged -it --rm -v "your\path\to\yolo\project":/app yolov5-project
```

Replace `your\path\to\yolo\project` with the absolute path to your project directory.

---

## Deployment on a Virtual Machine (VM)

### 1. Load the Docker Image

Transfer the Docker image (e.g., a `.tar` file) to your VM and load it:

```bash
docker load -i yolov5_project_monitoring.tar
```

---

### 2. Check the Folder Path for Watched Images

Verify the absolute path of the folder where images will be processed:

```bash
realpath -s esp_images
```

---

### 3. Run the Docker Container

Run the Docker container with the `esp_images` directory mounted as a volume:

```bash
docker run -d -v /home/niken/esp_images:/app/data/images yolov5-project-v9
```

---

### 4. Verify Running Containers

To see the container ID of the running container:

```bash
docker ps
```

---

### 5. Access the Running Container

Use the container ID (e.g., `9cae5011a3ff`) to access the container:

```bash
docker exec -it 9cae5011a3ff /bin/sh
```

---

### 6. Run the Watchdog Script

Once inside the container, run the watchdog script to monitor the images folder:

```bash
python watch_file_docker.py
```

---

## Stopping the Container

To stop the running container, use the following command with the container ID:

```bash
docker stop 9cae5011a3ff
```

Replace `9cae5011a3ff` with your actual container ID.

---

## Notes

- **Folder Mounting**: Ensure the folder paths used in the `docker run` command are correct and accessible.
- **Container Management**: Use `docker ps` frequently to keep track of active containers and their IDs.
- **Script Execution**: Ensure that `watch_file_docker.py` is correctly configured for your use case.

--- 

Enjoy using YOLOv5 with Docker! 🚀