import os
import csv
import time
from datetime import datetime
from pynput import keyboard


def record_event_timestamps(event_mapping):
  """
  Records timestamps of specified events to a CSV file.
  """
  import pathlib

  print("Available Event Types:")
  for key, event in event_mapping.items():
    key_name = str(key).replace("Key.", "") if isinstance(key, keyboard.Key) else str(key)
    print(f"  {event}: Press '{key_name}'")

  # Get CSV file path from user
  default_dir = '../tests'
  pathlib.Path(default_dir).mkdir(parents=True, exist_ok=True)
  existing_files = [f for f in os.listdir(default_dir) if f.startswith("timestamps") and f.endswith(".csv")]
  next_n = 1
  while f"timestamps{next_n}.csv" in existing_files:
    next_n += 1

  default_filename = f"timestamps{next_n}.csv"
  default_path = os.path.join(default_dir, default_filename)

  user_input_path = input(f"Enter CSV path to save (default: {default_path}): ").strip()
  csv_path = user_input_path if user_input_path else default_path

  # Wait for start key
  print("Press 's' to START recording...")
  start_recording = False

  def on_press_start(key):
    nonlocal start_recording
    if hasattr(key, 'char') and key.char == 's':
      start_recording = True
      return False  # stop listener

  with keyboard.Listener(on_press=on_press_start) as listener:
    listener.join()

  print(f"START at {datetime.now().isoformat()}")

  # Write header and START event
  with open(csv_path, mode='w', newline='') as file:
    writer = csv.writer(file)
    writer.writerow(['event', 'timestamp'])
    writer.writerow(['START', datetime.now().isoformat()])

    print("Recording... Press associated keys to record events. Press ESC to stop.")

    def on_press(key):
      if key == keyboard.Key.esc:
        # Stop listener
        return False
      if key in event_mapping:
        timestamp = datetime.now().isoformat()
        event_name = event_mapping[key]
        writer.writerow([event_name, timestamp])
        print(f"Recorded {event_name} at {timestamp}")

    with keyboard.Listener(on_press=on_press) as listener:
      listener.join()

  print("Recording stopped.")



def load_event_data(csv_path):
  """
  Loads event data from a CSV file and returns a dictionary with event names as keys
  """
  from datetime import datetime

  event_data = {}
  start_time = None

  with open(csv_path, mode='r') as file:
    reader = csv.reader(file)
    header = next(reader)
    for row in reader:
      event, timestamp_str = row
      timestamp = datetime.fromisoformat(timestamp_str)
      if event == 'START':
        start_time = timestamp
        continue
      if start_time is None:
        raise ValueError("START event must be the first event in the CSV file.")
      seconds_from_start = (timestamp - start_time).total_seconds()
      if event not in event_data:
        event_data[event] = []
      event_data[event].append(seconds_from_start)

  return event_data


if __name__ == '__main__':
  from pynput.keyboard import Key
  event_mapping = {
    Key.space: 'BIKE',
    Key.enter: 'RUNNER',
    Key.shift_l: 'LARGE TRUCK'
  }
  record_event_timestamps(event_mapping)