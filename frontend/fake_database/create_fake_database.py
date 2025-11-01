import sqlite3
import datetime
import random
from datetime import timedelta

# Create the databases
def create_sensor_database():
    conn = sqlite3.connect('server_sensor_data_count.db')
    cursor = conn.cursor()
    
    # Create table
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS sensor_data (
        measurement_ID INTEGER,
        timestamp TEXT,
        monitor_ID INTEGER,
        count INT
    )
    ''')
    
    conn.commit()
    return conn, cursor

def create_monitor_database():
    conn = sqlite3.connect('server_monitor_data_agg.db')
    cursor = conn.cursor()
    
    # Create table
    cursor.execute('''
    CREATE TABLE IF NOT EXISTS monitor_data (
        monitor_ID INTEGER,
        time_of_last_ping TEXT,
        battery_SOC REAL,
        failure_ID INTEGER,
        state TEXT,
        location TEXT,
        firmware_version TEXT
    )
    ''')
    
    conn.commit()
    return conn, cursor

# Generate random data for databases
def populate_sensor_database(conn, cursor, num_entries=1000):
    # Start timestamp
    base_time = datetime.datetime.now() - timedelta(days=30)
    
    for i in range(1, num_entries + 1):
        # Generate random data
        timestamp = base_time + timedelta(minutes=random.randint(0, 43200))  # Random time within last 30 days
        monitor_id = random.randint(0, 29)  # Monitor IDs from 0-29 (total 30)
        count = random.randint(1, 20)
        
        # Insert data
        cursor.execute('''
        INSERT INTO sensor_data (measurement_ID, timestamp, monitor_ID, count)
        VALUES (?, ?, ?, ?)
        ''', (i, timestamp.strftime('%Y-%m-%d %H:%M:%S'), monitor_id, count))
    
    conn.commit()
    print(f"Added {num_entries} sensor records")

def populate_monitor_database(conn, cursor):
    # Cambridge, MA approximate boundaries
    min_lat, max_lat = 42.3700, 42.3900
    min_long, max_long = -71.1500, -71.1000
    
    states = ['awake', 'light_sleep', 'deep_sleep']
    base_time = datetime.datetime.now() - timedelta(hours=random.randint(0, 72))
    
    for monitor_id in range(30):  # Create exactly 30 monitors (IDs 0-29)
        time_of_last_ping = base_time + timedelta(minutes=random.randint(0, 4320))
        battery_soc = round(random.uniform(0.0, 5.0), 2)  # Battery voltage between 0-5V
        failure_id = random.randint(0, 5)
        state = random.choice(states)
        
        # Generate random location in Cambridge, MA
        lat = round(random.uniform(min_lat, max_lat), 6)
        long = round(random.uniform(min_long, max_long), 6)
        location = f"{lat}, {long}"
        
        # Create firmware version (format: major.minor.patch)
        major = random.randint(1, 3)
        minor = random.randint(0, 9)
        patch = random.randint(0, 99)
        firmware_version = f"{major}.{minor}.{patch}"
        
        # Insert monitor data
        cursor.execute('''
        INSERT INTO monitor_data (monitor_ID, time_of_last_ping, battery_SOC, failure_ID, state, location, firmware_version)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (monitor_id, time_of_last_ping.strftime('%Y-%m-%d %H:%M:%S'), 
              battery_soc, failure_id, state, location, firmware_version))
    
    conn.commit()
    print("Added 30 monitor records")

# Main function
def main():
    print("Creating and populating sensor database...")
    sensor_conn, sensor_cursor = create_sensor_database()
    populate_sensor_database(sensor_conn, sensor_cursor)
    sensor_conn.close()
    
    # print("Creating and populating monitor database...")
    # monitor_conn, monitor_cursor = create_monitor_database()
    # populate_monitor_database(monitor_conn, monitor_cursor)
    # monitor_conn.close()
    
    print("Database population complete!")

if __name__ == "__main__":
    main()