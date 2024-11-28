import subprocess
import time
from watchdog.observers.polling import PollingObserver
from watchdog.events import FileSystemEventHandler

# Path to the directory to be monitored
directory_to_watch = "/app/data/images"  # Adjust to match the Docker path
# Path to the Python file you want to run
python_script = "/app/main_docker.py"  # No need to specify the venv interpreter

class NewImageHandler(FileSystemEventHandler):
    def on_created(self, event):
        print(f"Watching directory: {event.src_path}")
        if event.is_directory:
            return
        if event.src_path.endswith((".jpg", ".jpeg", ".png")):
            print(f"New image detected: {event.src_path}")
            # Run the Python script with the container's Python interpreter
            subprocess.run(["python", python_script, event.src_path])

if __name__ == "__main__":
    
    event_handler = NewImageHandler()
    observer = PollingObserver()
    observer.schedule(event_handler, path=directory_to_watch, recursive=False)

    print("Starting Watchdog script...")
    observer.start()
    print(f"Monitoring directory: {directory_to_watch}")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()

    observer.join()
