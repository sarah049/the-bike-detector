from fastapi import FastAPI, Query, Request, HTTPException
import math
import sqlite3
import datetime
import time
from fastapi.responses import HTMLResponse, FileResponse
from fastapi import Request
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
import pandas as pd
import plotly
import plotly.express as px
import json
import random
from fastapi import Query
from fastapi import Form
from fastapi.responses import JSONResponse
import os



# FOR WINDOWS RUN:
# python -m venv efi_venv
# source efi_venv/Scripts/activate

# fastapi dev fastapi_skeleton.py

# runs on http://127.0.0.1:8000/


templates = Jinja2Templates(directory="templates")
app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")

# app = FastAPI(
#     title="EFI Development",
#     openapi_url=f"/openapi.json",
#     docs_url=f"/docs",
#     redoc_url=f"/redoc",
#     root_path="/tunnel_inimai" # <------ root_path adjustment fixes the prepend on the tunnel
# )

MONITOR_DB = "data/server_monitor_data_agg.db"
SENSOR_DB = "data/server_sensor_data_count.db"
num_detectors = 1
FIRMWARE_DIR = "firmware_updates"
LATEST_FIRMWARE = "v1.0.0"  # Update this when new firmware is released

# POST ENDPOINTS
DEBUG_DB = "data/server_debug_data.db"  
@app.post("/debug_data", response_class=HTMLResponse)
def post_function(Failure_ID: int, Location_ID: int, count: int): #CHECK IF TIME WILL BE SENT OR WILL BE PUT WHEN POSTING TO DATABASE

  conn = sqlite3.connect(DEBUG_DB)  # connect to that database (will create if it doesn't already exist)
  c = conn.cursor()  # move cursor into database (allows us to execute commands)
  c.execute('''CREATE TABLE IF NOT EXISTS debug_table (
            Failure_ID real,
            Location_ID real,
            count real,
            time timestamp
            );''') 
  c.execute('''INSERT into rht_table VALUES (?,?,?,?,?,?);''', (Failure_ID, Location_ID, count, datetime.datetime.now()))

  # you can use it to access most recents
  conn.commit() # commit commands
  conn.close() # close connection to database
  return {
        "message": "Data logged successfully",
        "Failure_ID": Failure_ID,
        "Location_ID": Location_ID,
        "count": count,
        "Time": datetime.datetime.now(),
    }


DATA_DB = "data/server_post_data.db"
@app.post("/post_data", response_class=HTMLResponse)
def post_data(monitor_id: str, firmware_v: str, failure_id: str, batt_volt: str, start_time: str, time_list: str):
  
    # Update post_data database
    conn = sqlite3.connect(DATA_DB)
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS post_table (
            monitor_id TEXT,
            firmware_v TEXT,
            failure_id TEXT,
            batt_volt real,
            start_time TEXT,
            time_list TEXT,
            time timestamp
            );''') 
    c.execute('''INSERT into post_table VALUES (?,?,?,?,?,?,?);''', 
             (monitor_id, firmware_v, failure_id, batt_volt, start_time, time_list, datetime.datetime.now()))
    conn.commit()
    conn.close()

    # Update monitor_data database
    conn = sqlite3.connect(MONITOR_DB)
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS monitor_data (
            monitor_ID TEXT,
            time_of_last_ping TEXT,
            battery_SOC INTEGER,
            failure_ID INTEGER,
            state TEXT,
            location TEXT,
            firmware_version TEXT
            );''')
    
    # Convert battery voltage to percentage (assuming 3.7V is 100%)
    battery_soc = batt_volt
    
    # Get current time
    current_time = datetime.datetime.now().isoformat()
    
    # Update or insert monitor data
    c.execute('''INSERT OR REPLACE INTO monitor_data 
                (monitor_ID, time_of_last_ping, battery_SOC, failure_ID, state, firmware_version) 
                VALUES (?, ?, ?, ?, ?, ?)''',
             (monitor_id, current_time, battery_soc, failure_id, "active", firmware_v))
    conn.commit()
    conn.close()

    # Update bike data database
    conn = sqlite3.connect(SENSOR_DB)
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS sensor_data (
            monitor_ID TEXT,
            timestamp TEXT,
            count INTEGER
            );''')
    
    # Parse time_list and insert bike counts
    try:
        time_list_data = json.loads(time_list)
        # Convert start_time to datetime object
        start_datetime = datetime.datetime.fromisoformat(start_time)
        
        for time_offset in time_list_data:
            # Add the time offset to the start time
            bike_time = start_datetime + datetime.timedelta(seconds=time_offset)
            bike_time_str = bike_time.isoformat()
            
            c.execute('''INSERT INTO sensor_data (monitor_ID, timestamp, count) 
                        VALUES (?, ?, ?)''',
                     (monitor_id, bike_time_str, 1))  # Each timestamp represents one bike
    except (json.JSONDecodeError, ValueError) as e:
        print(f"Error processing time_list: {e}")  # Log error but continue
    
    conn.commit()
    conn.close()

    # Check if firmware needs updating
    needs_update = firmware_v != LATEST_FIRMWARE
    update_message = "Firmware update required" if needs_update else "Firmware is up to date"

    return {
        "message": "Data received successfully",
        "update_required": needs_update,
        "update_message": update_message,
        "Time": datetime.datetime.now(),
    }


# GET ENPOINTS


@app.get("/", response_class=HTMLResponse)
async def home(request: Request):
    return templates.TemplateResponse(
        name="home.html",context={"request": request}
    )


# @app.get("/")
# async def home(request: Request):
#     user = request.session.get("user")  # Assuming session-based auth
#     return templates.TemplateResponse("home.html", {"request": request, "user": user})

@app.get("/log_in", response_class=HTMLResponse)
async def log_in(request: Request):
    return templates.TemplateResponse(
        name="log_in.html",context={"request": request}
)

@app.delete("/delete_monitor")
def delete_monitor():
    conn = sqlite3.connect(MONITOR_DB)
    cursor = conn.cursor()

    monitor_id = "43"  

    cursor.execute("DELETE FROM monitor_data WHERE monitor_ID = ?", (monitor_id,))
    deleted = cursor.rowcount

    conn.commit()
    conn.close()

    if deleted:
        return JSONResponse(
            status_code=200,
            content={"success": True, "message": f"Monitor {monitor_id} deleted successfully!"}
        )
    else:
        return JSONResponse(
            status_code=404,
            content={"success": False, "message": f"Monitor {monitor_id} not found."}
        )
    
@app.post("/register_monitor")
async def register_monitor(
    monitor_id: str = Form(...),
    latitude: float = Form(...),
    longitude: float = Form(...),
    access_code: str = Form(...)
):
    
    if access_code != "12":
        return JSONResponse(
            status_code=403,
            content={"success": False, "message": "Access code is incorrect."}
        )

    conn = sqlite3.connect(MONITOR_DB)
    cursor = conn.cursor()

    
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS monitor_data (
            monitor_ID TEXT,
            time_of_last_ping TEXT,
            battery_SOC INTEGER,
            failure_ID INTEGER,
            state TEXT,
            location TEXT,
            firmware_version TEXT
        )
    ''')
    
    
    cursor.execute("SELECT 1 FROM monitor_data WHERE monitor_ID = ?", (monitor_id,))
    if cursor.fetchone():
        conn.close()
        return JSONResponse(
            status_code=409,
            content={"success": False, "message": f"Monitor ID '{monitor_id}' already exists."}
        )

    now = datetime.datetime.now().isoformat()
    battery_SOC = 100
    failure_ID = 0
    state = "light_sleep"
    firmware_version = "v1.0.0"
    location = f"{latitude}, {longitude}"

    cursor.execute('''
        INSERT INTO monitor_data (
            monitor_ID, time_of_last_ping, battery_SOC, failure_ID, state, location, firmware_version
        ) VALUES (?, ?, ?, ?, ?, ?, ?)
    ''', (monitor_id, now, battery_SOC, failure_ID, state, location, firmware_version))

    conn.commit()
    conn.close()

    num_detectors += 1

    return JSONResponse(
        status_code=200,
        content={"success": True, "message": f"Monitor '{monitor_id}' registered successfully!"}
    )


@app.get("/sign_up", response_class=HTMLResponse)
async def sign_up(request: Request):
    return templates.TemplateResponse(
        name="sign_up.html",context={"request": request}
    )

@app.get("/bikedata", response_class=HTMLResponse)
async def bikedata(
    request: Request,
    monitors: str = None,
    # directions: str = None,
    from_date: str = None,
    to_date: str = None,
    min_count: int = None,
    max_count: int = None,
    # min_speed: int = None,
    # max_speed: int = None,
    page: int = 1
):
    per_page = 10
    offset = (page - 1) * per_page
    conn = sqlite3.connect("data/server_sensor_data_count.db")
    cursor = conn.cursor()

    # Base query for filtering
    base_query = "FROM sensor_data"
    conditions = []
    values = []

    # Apply filters
    if monitors:
        monitor_list = monitors.split(",")
        conditions.append(f"monitor_ID IN ({','.join(['?'] * len(monitor_list))})")
        values.extend(monitor_list)

    # if directions:
    #     directions_list = directions.split(",")
    #     conditions.append(f"direction IN ({','.join(['?'] * len(directions_list))})")
    #     values.extend(directions_list)

    if from_date:
        conditions.append("timestamp >= ?")
        values.append(from_date)

    if to_date:
        conditions.append("timestamp <= ?")
        values.append(to_date)

    # if min_speed:
    #     conditions.append("speed >= ?")
    #     values.append(min_speed)

    # if max_speed:
    #     conditions.append("speed <= ?")
    #     values.append(max_speed)

    if min_count:
        conditions.append("count >= ?")
        values.append(min_count)

    if max_count:
        conditions.append("count <= ?")
        values.append(max_count)

    # Combine conditions with `AND`
    where_clause = " WHERE " + " AND ".join(conditions) if conditions else ""

    # Query for paginated data
    data_query = f"SELECT * {base_query} {where_clause} LIMIT ? OFFSET ?"
    cursor.execute(data_query, values + [per_page, offset])
    data = cursor.fetchall()

    # Query for filtered total count (WITHOUT LIMIT & OFFSET)
    count_query = f"SELECT COUNT(*) {base_query} {where_clause}"
    cursor.execute(count_query, values)
    filtered_total_entries = cursor.fetchone()[0]

    conn.close()

    return templates.TemplateResponse("bikedata.html", {"request": request, "data": data, "total_entries": filtered_total_entries, "current_page": page})


@app.get("/monitordata", response_class=HTMLResponse)
async def monitordata(
    request: Request,
    monitors: str = None, 
    failures: str = None, 
    state: str = None, 
    from_date: str = None,
    to_date: str = None,
    page: int = 1
):
    per_page = 10
    offset = (page - 1) * per_page
    conn = sqlite3.connect("data/server_monitor_data_agg.db")
    cursor = conn.cursor()

    # Base query for filtering
    base_query = "FROM monitor_data"
    conditions = []
    values = []

    # Apply filters
    if monitors:
        monitor_list = monitors.split(",")
        conditions.append(f"monitor_ID IN ({','.join(['?'] * len(monitor_list))})")
        values.extend(monitor_list)
    
    if failures:
        failures_list = failures.split(',')
        conditions.append(f"failure_ID IN ({','.join(['?'] * len(failures_list))})")
        values.extend(failures_list)

    if state:
        states_list = state.split(',')
        conditions.append(f"state IN ({','.join(['?'] * len(states_list))})")
        values.extend(states_list)

    if from_date:
        conditions.append("time_of_last_ping >= ?")
        values.append(from_date)

    if to_date:
        conditions.append("time_of_last_ping <= ?")
        values.append(to_date)


    # Combine conditions with `AND`
    where_clause = " WHERE " + " AND ".join(conditions) if conditions else ""

    # Query for paginated data
    data_query = f"SELECT * {base_query} {where_clause} LIMIT ? OFFSET ?"
    cursor.execute(data_query, values + [per_page, offset])
    data = cursor.fetchall()

    # Query for filtered total count (WITHOUT LIMIT & OFFSET)
    count_query = f"SELECT COUNT(*) {base_query} {where_clause}"
    cursor.execute(count_query, values)
    filtered_total_entries = cursor.fetchone()[0]

    conn.close()

    return templates.TemplateResponse("monitordata.html", {"request": request, "data": data, "total_entries": filtered_total_entries, "current_page": page})


@app.get("/api/graph-data")
async def graph_data(
    request: Request,
    monitor_id: str = None,
    graph_type: str = None,
    from_date: str = None,
    to_date: str = None
):
    if not monitor_id or not graph_type:
        return {"error": "Missing required parameters"}
    
    conn = sqlite3.connect("data/server_monitor_data_agg.db")
    cursor = conn.cursor()
    
    # Define what column to fetch based on graph type
    column_map = {
        "soc": "battery_SOC",
        "failure": "failure_ID",
        "state": "state"
    }
    
    y_column = column_map.get(graph_type.lower())
    if not y_column:
        return {"error": "Invalid graph type"}
    
    # Build query conditions
    conditions = ["monitor_ID = ?"]
    values = [monitor_id]
    
    if from_date:
        conditions.append("time_of_last_ping >= ?")
        values.append(from_date)
    
    if to_date:
        conditions.append("time_of_last_ping <= ?")
        values.append(to_date)
    
    where_clause = " WHERE " + " AND ".join(conditions)
    
    # Query data
    query = f"""
        SELECT time_of_last_ping, {y_column}
        FROM monitor_data
        {where_clause}
        ORDER BY time_of_last_ping
    """
    
    cursor.execute(query, values)
    rows = cursor.fetchall()
    
    # Format data for Plotly
    time_values = [row[0] for row in rows]
    y_values = [row[1] for row in rows]
    
    # For state data, convert to numeric if needed
    if graph_type.lower() == "state":
        # Create a mapping for states to numbers if they're strings
        distinct_states = list(set(y_values))
        state_to_num = {state: i for i, state in enumerate(distinct_states)}
        
        if not all(isinstance(val, (int, float)) for val in y_values):
            y_values = [state_to_num[state] for state in y_values]
            # Include state mapping in the response
            state_mapping = {i: state for state, i in state_to_num.items()}
        else:
            state_mapping = {}
    else:
        state_mapping = {}
    
    conn.close()
    
    # Set graph titles based on type
    titles = {
        "soc": {"title": f"Battery SOC for Monitor {monitor_id}", "yaxis": "Battery SOC (%)"},
        "failure": {"title": f"Failures for Monitor {monitor_id}", "yaxis": "Failure ID"},
        "state": {"title": f"State Changes for Monitor {monitor_id}", "yaxis": "State"}
    }
    
    graph_info = titles.get(graph_type.lower(), {"title": "Monitor Data", "yaxis": "Value"})
    
    return {
        "time": time_values,
        "y_values": y_values,
        "title": graph_info["title"],
        "xaxis": "Time",
        "yaxis": graph_info["yaxis"],
        "state_mapping": state_mapping
    }


@app.get("/api/graph-data-bike")
async def graph_data_bike(
    request: Request,
    monitor_id: str = None,
    graph_type: str = None,
    from_date: str = None,
    to_date: str = None
):
    if not monitor_id or not graph_type:
        return {"error": "Missing required parameters"}
    
    conn = sqlite3.connect("data/server_sensor_data_count.db")
    cursor = conn.cursor()
    
    # Define what column to fetch based on graph type
    column_map = {
        "count": "count",
    }
    
    y_column = column_map.get(graph_type.lower())
    if not y_column:
        return {"error": "Invalid graph type"}
    
    # Build query conditions
    conditions = ["monitor_ID = ?"]
    values = [monitor_id]
    
    if from_date:
        conditions.append("time_of_last_ping >= ?")
        values.append(from_date)
    
    if to_date:
        conditions.append("time_of_last_ping <= ?")
        values.append(to_date)
    
    where_clause = " WHERE " + " AND ".join(conditions)
    
    # Query data
    query = f"""
        SELECT timestamp, {y_column}
        FROM sensor_data
        {where_clause}
        ORDER BY timestamp
    """
    
    cursor.execute(query, values)
    rows = cursor.fetchall()
    
    # Format data for Plotly
    time_values = [row[0] for row in rows]
    y_values = [row[1] for row in rows]
    
    # For state data, convert to numeric if needed
    if graph_type.lower() == "state":
        # Create a mapping for states to numbers if they're strings
        distinct_states = list(set(y_values))
        state_to_num = {state: i for i, state in enumerate(distinct_states)}
        
        if not all(isinstance(val, (int, float)) for val in y_values):
            y_values = [state_to_num[state] for state in y_values]
            # Include state mapping in the response
            state_mapping = {i: state for state, i in state_to_num.items()}
        else:
            state_mapping = {}
    else:
        state_mapping = {}
    
    conn.close()
    
    # Set graph titles based on type
    titles = {
        "count": {"title": f"Count for Monitor {monitor_id}", "yaxis": "Count"},
    }
    
    graph_info = titles.get(graph_type.lower(), {"title": "Monitor Data", "yaxis": "Value"})
    
    return {
        "time": time_values,
        "y_values": y_values,
        "title": graph_info["title"],
        "xaxis": "Time",
        "yaxis": graph_info["yaxis"],
        "state_mapping": state_mapping
    }




@app.get("/contact", response_class=HTMLResponse)
async def contact(request: Request):
    return templates.TemplateResponse("contact.html", {"request": request})

@app.get("/map", response_class=HTMLResponse)
async def map_page(request: Request):
    # Connect to your database
    conn = sqlite3.connect("data/server_monitor_data_agg.db")
    cursor = conn.cursor()

    query = """
    SELECT monitor_ID, time_of_last_ping, battery_SOC, failure_ID, state, location, firmware_version
    FROM monitor_data
    WHERE monitor_ID IN (
        SELECT monitor_ID
        FROM monitor_data
        GROUP BY monitor_ID
        ORDER BY MAX(time_of_last_ping) DESC
    )
    ORDER BY time_of_last_ping DESC
    """
    cursor.execute(query)
    data = cursor.fetchall()  # Get all the rows

    # Close the connection
    conn.close()

    # Pass the data to the template
    return templates.TemplateResponse("map_page.html", {"request": request, "data": data})

@app.get("/test_plot/{num_points}")
def plotter(request: Request, num_points: int):
    xs = list(range(num_points)) # make 100 points
    ys = [random.random() for i in xs] #import random! then assign random y values
    # make a nice data frame using pandas
    df = pd.DataFrame({
       'x': xs,
       'y': ys,
    })
    #make a line plot using pandas:
    fig = px.line(df, x='x', y='y')
    graphJSON = json.dumps(fig, cls=plotly.utils.PlotlyJSONEncoder)
    return templates.TemplateResponse(
        request=request, name="plotly_template.html", context={"graphJSON": graphJSON}
    )

def fetch_data(column: str, num_points: int):
    conn = sqlite3.connect(SENSOR_DB)
    cursor = conn.cursor()
    cursor.execute(f"""
        SELECT timestamp, {column} FROM logs
        ORDER BY timestamp DESC
        LIMIT ?;
    """, (num_points,))
    data = cursor.fetchall()
    conn.commit()
    conn.close()

    return list(reversed(data))  

def plotter(request: Request, column: str, y_label: str, num_points: int):
    data = fetch_data(column, num_points)
    if not data:
        return templates.TemplateResponse(request=request, name="plotly_template.html", context={"graphJSON": "{}"})

    timestamps, values = zip(*data)
    # timestamps = [datetime.datetime.fromtimestamp(float(ts)).isoformat() for ts in timestamps]


    df = pd.DataFrame({"Time": timestamps, y_label: values})
    fig = px.line(df, x="Time", y=y_label, title=f"{y_label} vs Time")

    graphJSON = json.dumps(fig, cls=plotly.utils.PlotlyJSONEncoder)

    return templates.TemplateResponse(request=request, name="plotly_template.html", context={"graphJSON": graphJSON})


# Create firmware directory if it doesn't exist
os.makedirs(FIRMWARE_DIR, exist_ok=True)

@app.get("/api/check-update")
async def check_update(version: str):
    """Check if a firmware update is available"""
    try:
        needs_update = version != LATEST_FIRMWARE
        
        return {
            "update_available": needs_update,
            "current_version": version,
            "latest_version": LATEST_FIRMWARE,
            "update_url": f"/api/download-firmware/{LATEST_FIRMWARE}" if needs_update else None
        }
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))

@app.get("/api/download-firmware")
async def download_firmware(version: str):
    """Download the firmware binary"""
    firmware_path = os.path.join(FIRMWARE_DIR, f"firmware_{version}.bin")
    
    if not os.path.exists(firmware_path):
        raise HTTPException(status_code=404, detail="Firmware version not found")
    
    return FileResponse(
        firmware_path,
        media_type="application/octet-stream",
        filename=f"firmware_{version}.bin"
    )

@app.post("/api/upload-firmware")
async def upload_firmware(request: Request):
    """Upload a new firmware version (admin only)"""
    # TODO: Add authentication
    try:
        data = await request.body()
        version = request.headers.get("X-Firmware-Version")
        
        if not version:
            raise HTTPException(status_code=400, detail="Firmware version header required")
        
        firmware_path = os.path.join(FIRMWARE_DIR, f"firmware_{version}.bin")
        
        with open(firmware_path, "wb") as f:
            f.write(data)
        
        # Update the latest firmware version
        global LATEST_FIRMWARE
        LATEST_FIRMWARE = version
        
        return {"message": "Firmware uploaded successfully", "version": version}
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))