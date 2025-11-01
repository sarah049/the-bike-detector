import serial
from serial.tools import list_ports
import time

# Set your default port (update as needed)
DEFAULT_PORT = '/dev/cu.SLAB_USBtoUART'
BAUD_RATE = 921600

def list_serial_ports():
    """ Returns a list of available serial ports. """
    return [port.device for port in list_ports.comports()]

def connect_to_port(port, baud_rate):
    """ Attempts to open a serial connection on the given port. """
    try:
        ser = serial.Serial(port, baud_rate, timeout=0)
        if ser.is_open:
            print(f"Connected to {port}")
            return ser
    except serial.SerialException as e:
        print(f"Failed to connect on {port}: {e}")
    return None

def process_connection(ser):
    """ Processes incoming data from the serial port. """
    print("Waiting for data (3 seconds timeout)...")
    start_time = time.time()
    data_received = False

    while time.time() - start_time < 3:
        if ser.in_waiting:
            data = ser.read(ser.in_waiting)
            if data:
                print(data.decode(errors="replace"), end="", flush=True)
                data_received = True
        time.sleep(0.1)

    if not data_received:
        print("No data received. Closing port.")
        ser.close()
        return False

    print("\nData detected. Continuing to print incoming data. Press Ctrl+C to interrupt.")
    try:
        while True:
            if ser.in_waiting:
                data = ser.read(ser.in_waiting)
                if data:
                    print(data.decode(errors="replace"), end="", flush=True)
            time.sleep(0.1)
    except KeyboardInterrupt:
        print("\nInterrupted by user. Closing port and exiting.")
        ser.close()
    return True

def prioritize_ports(ports, default_port):
    """ Prioritizes ports: substring matches first, then exact match, then others. """
    substring_ports = [p for p in ports if default_port in p and p != default_port]
    exact_ports = [default_port] if default_port in ports else []
    other_ports = [p for p in ports if default_port not in p]
    return substring_ports + exact_ports + other_ports

def main():
    ports = list_serial_ports()
    print("Available ports:", ports)

    prioritized_ports = prioritize_ports(ports, DEFAULT_PORT)

    for port in prioritized_ports:
        print(f"Trying port: {port}")
        ser = connect_to_port(port, BAUD_RATE)
        if ser and process_connection(ser):
            return

    print("Could not establish a connection that receives data on any serial port.")

if __name__ == "__main__":
    main()