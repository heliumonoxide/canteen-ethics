import subprocess
import time
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler

# Path to the directory to be monitored
directory_to_watch = "data/images"
# Path to the Python file you want to run
python_script = "main.py"
# Path to your virtual environment's Python interpreter
venv_python = "Scripts/python.exe"  # On Windows: "path/to/venv/Scripts/python.exe"

class NewImageHandler(FileSystemEventHandler):
    def on_created(self, event):
        # Check if the new file is a picture
        if event.is_directory:
            return

        if event.src_path.endswith((".jpg", ".jpeg", ".png")):
            print(f"New image detected: {event.src_path}")
            # Run the Python script with the specified interpreter
            subprocess.run([venv_python, python_script, event.src_path])

if __name__ == "__main__":
    event_handler = NewImageHandler()
    observer = Observer()
    observer.schedule(event_handler, path=directory_to_watch, recursive=False)

    observer.start()
    print(f"Monitoring directory: {directory_to_watch}")

    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()

    observer.join()
