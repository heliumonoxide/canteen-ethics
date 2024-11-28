import os
import json
import shutil
import sys
import torch

# Import Firestore Connection_Docker functions
from services.firestore_docker import Connection_Docker

# Path to store tracking state
TRACKING_STATE_PATH = "/app/tracking_state.json"
WEIGHT_PATH = '/app/yolov5x.pt'
RESULT_PATH = '/app/results'

class Main:
    def __init__(self, img_path):
        # Load YOLOv5 model
        model = torch.hub.load('ultralytics/yolov5', 'custom', WEIGHT_PATH)
        results = model(img_path)
        results.save(save_dir='results')

        self.file_name = os.path.basename(img_path)
        self.result_dir_blob = "results/" + self.file_name
        self.result_dir_docker = RESULT_PATH + "/" + self.file_name
        print(f"Blob directory for upload image {self.result_dir_blob} ")
        print(f"Docker directory for result image {self.result_dir_docker} ")

        # Parse results
        df = results.pandas().xyxy[0]  # Dataframe of detection results
        has_person = not df[df['class'] == 0].empty
        has_bowl = not df[df['class'] == 45].empty

        print(df)

        # Update tracking state
        self.update_tracking(has_person, has_bowl)

        # Initialize Firestore and Firebase Storage
        connection = Connection_Docker()  # Create an instance of Connection_Docker
        connection.initialize_sdk()

        # Upload results to Firebase
        connection.upload_image(self.result_dir_docker, self.result_dir_blob)

        # Check if the folder exists and delete it
        if os.path.exists(RESULT_PATH):
            shutil.rmtree(RESULT_PATH)
            print(f"Folder '{RESULT_PATH}' and its contents have been deleted.")
        else:
            print(f"The folder '{RESULT_PATH}' does not exist.")

    def update_tracking(self, has_person, has_bowl):
        """Update tracking state based on YOLOv5 results."""
        if os.path.exists(TRACKING_STATE_PATH):
            with open(TRACKING_STATE_PATH, 'r') as f:
                state = json.load(f)
        else:
            state = {
                "bowl_count": 0,
                "violation_count": 0,
                "was_bowl_alone": False
            }

        # Check if the bowl is no longer detected after being alone
        if not has_bowl and state["was_bowl_alone"]:
            state["bowl_count"] = 0

        # Update bowl_count only if bowl is present without a person
        if has_bowl and not has_person:
            state["bowl_count"] += 1
            state["was_bowl_alone"] = True
        else:
            state["was_bowl_alone"] = False
            state["bowl_count"] = 0

        # Check for violation condition: Bowl count exceeds 10 frames
        if state["bowl_count"] >= 10:
            state["violation_count"] += 1
            print(f"Violation detected! Total violations: {state['violation_count']}")
            state["bowl_count"] = 0

        # Save updated state
        with open(TRACKING_STATE_PATH, 'w') as f:
            json.dump(state, f)


if __name__ == "__main__":
    if len(sys.argv) > 1:
        img_path = sys.argv[1]
        Main(img_path)
    else:
        print("No image path provided.")
