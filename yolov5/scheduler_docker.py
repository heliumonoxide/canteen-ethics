import time
from datetime import datetime as dt, timezone
import pytz
import schedule
import json
import os
from services.firestore_docker import Connection_Docker

# Path to store tracking state
TRACKING_STATE_PATH = "/app/tracking_state.json"

# Define Jakarta timezone
JAKARTA_TZ = pytz.timezone("Asia/Jakarta")


class Scheduler:
    def __init__(self):
        self.connection_docker = Connection_Docker()  # Create an instance of Connection_Docker
        self.connection_docker.initialize_sdk()  # Initialize Firebase SDK
        self.load_state()

    def load_state(self):
        """Load tracking state from JSON file."""
        if os.path.exists(TRACKING_STATE_PATH):
            with open(TRACKING_STATE_PATH, "r") as f:
                self.state = json.load(f)
        else:
            # Default state
            self.state = {
                "bowl_count": 0,  # Tracks consecutive frames where the bowl is present without a person
                "violation_count": 0,  # Tracks total violations
            }

    def save_state(self):
        """Save current state to JSON file."""
        with open(TRACKING_STATE_PATH, "w") as f:
            json.dump(self.state, f)

    def upload_daily_summary(self):
        """Submit daily summary to Firestore at 12:00 AM."""
        self.load_state()
        timestamp = dt.now().strftime("%c")
        print(f"Uploading daily summary at {timestamp} Jakarta time.")
        print("Current state before upload:", self.state)

        data_violations_perday = {
            "violation_sum": self.state["violation_count"],
            "time_added": dt.now(timezone.utc),  # UTC timestamp
        }

        print("Data to upload:", data_violations_perday)

        # Use Connection_Docker to post the document
        self.connection_docker.post_doc(data_violations_perday, "ethics-violations")
        print(f"Uploaded data successfully at {timestamp} Jakarta time.")

        # Reset bowl_count and violations after daily upload
        self.state["bowl_count"] = 0
        self.state["violation_count"] = 0
        self.save_state()

    def start_scheduler(self):
        """Start the scheduler to run daily tasks."""
        schedule.every().day.at("00:00").do(self.upload_daily_summary)

        while True:
            schedule.run_pending()
            time.sleep(1)  # Check for scheduled tasks every second


if __name__ == "__main__":
    scheduler = Scheduler()
    print(f"Current time: {dt.now(JAKARTA_TZ)}")
    scheduler.start_scheduler()
