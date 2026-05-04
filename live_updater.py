import requests
import csv
import time
import os
import sys
from datetime import datetime

# --- CONFIGURATION ---
TOMTOM_API_KEY = "d9cd2979-3c65-4456-ab92-899b9d891379"
# Note: Open-Meteo API - No API key required!
# Free weather data from https://open-meteo.com/

UPDATE_INTERVAL = 900 # 15 minutes

def get_thana_traffic(lat, lon):
    """Fetches real-time traffic factor using TomTom Traffic API."""
    try:
        # TomTom Traffic Flow API Call
        url = f"https://api.tomtom.com/traffic/services/4/flowSegmentData/absolute/10/json?key={TOMTOM_API_KEY}&point={lat},{lon}"
        response = requests.get(url, timeout=10)
        data = response.json()
        
        if 'flowSegmentData' in data:
            current_speed = data['flowSegmentData']['currentSpeed']
            free_flow_speed = data['flowSegmentData']['freeFlowSpeed']
            
            # Ratio calculation: speed joto kom, traffic toto beshi
            ratio = current_speed / free_flow_speed
            if ratio < 0.3: return 2.0  # Heavy Traffic
            elif ratio < 0.6: return 1.6 # Medium Traffic
            elif ratio < 0.8: return 1.3 # Light-Medium
            return 1.0 # Free Flow
        return 1.0
    except Exception as e:
        print(f"Traffic API Error: {e}")
        return 1.0


def describe_traffic_factor(factor):
    if factor >= 1.9:
        return "Heavy Traffic"
    elif factor >= 1.5:
        return "Medium Traffic"
    elif factor >= 1.2:
        return "Light-Medium Traffic"
    return "Free Flow"

def get_real_weather(lat, lon):
    """Fetches real weather data from Open-Meteo API (no key required)."""
    try:
        # Open-Meteo API - completely free, no registration needed
        url = f"https://api.open-meteo.com/v1/forecast?latitude={lat}&longitude={lon}&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m&timezone=auto"
        response = requests.get(url, timeout=10)
        data = response.json()
        
        if 'current' in data:
            current = data['current']
            temp = current.get('temperature_2m', 25)
            humidity = current.get('relative_humidity_2m', 70)
            weather_code = current.get('weather_code', 0)
            wind_speed = current.get('wind_speed_10m', 5)
            
            # Convert WMO weather code to condition
            condition = get_weather_condition(weather_code)
            
            # Calculate multiplier based on weather
            multiplier = calculate_weather_multiplier(weather_code, wind_speed)
            
            return {
                "condition": condition,
                "temp": temp,
                "humidity": humidity,
                "wind_speed": wind_speed,
                "multiplier": multiplier
            }
        return get_default_weather()
    except Exception as e:
        print(f"Weather API Error: {e}")
        return get_default_weather()

def get_weather_condition(weather_code):
    """Convert WMO weather code to readable condition."""
    if weather_code == 0:
        return "Clear"
    elif weather_code in [1, 2]:
        return "Cloudy"
    elif weather_code == 3:
        return "Overcast"
    elif weather_code in [45, 48]:
        return "Foggy"
    elif weather_code in [51, 53, 55, 61, 63, 65]:
        return "Rainy"
    elif weather_code in [80, 81, 82]:
        return "Showers"
    elif weather_code in [71, 73, 75, 77, 85, 86]:
        return "Snowy"
    else:
        return "Unknown"

def calculate_weather_multiplier(weather_code, wind_speed):
    """Calculate ride price multiplier based on weather conditions."""
    multiplier = 1.0
    
    # Weather impact
    if weather_code in [51, 53, 55, 61, 63, 65]:  # Rain
        multiplier += 0.5
    elif weather_code in [80, 81, 82]:  # Showers (heavy rain)
        multiplier += 0.8
    elif weather_code in [71, 73, 75, 77, 85, 86]:  # Snow
        multiplier += 0.6
    elif weather_code in [45, 48]:  # Fog
        multiplier += 0.3
    
    # Wind impact (above 30 km/h)
    if wind_speed > 30:
        multiplier += 0.2
    
    return round(multiplier, 2)

def get_default_weather():
    """Returns default weather when API fails."""
    return {
        "condition": "Clear",
        "temp": 25,
        "humidity": 70,
        "wind_speed": 5,
        "multiplier": 1.0
    }


def load_locations_from_file(location_file):
    locations = []
    if not os.path.exists(location_file):
        return locations

    with open(location_file, mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            if len(row) >= 3:
                locations.append({"name": row[0], "lat": row[1], "lon": row[2]})

    return locations


def write_live_csvs(traffic_rows, weather_rows):
    with open('traffic.csv', mode='w', newline='') as f:
        csv.writer(f).writerows(traffic_rows)
    with open('weather.csv', mode='w', newline='') as f:
        csv.writer(f).writerows(weather_rows)

def run_live_update():
    location_file = 'location.csv'

    locations = load_locations_from_file(location_file)
    if not locations:
        print("Error: location.csv not found!")
        return

    now = datetime.now()
    weather_rows = []
    traffic_rows = []
    
    # Add headers
    traffic_rows.append(["Location", "Year", "Month", "Day", "Hour", "TrafficMultiplier"])
    weather_rows.append(["Location", "Year", "Month", "Day", "Hour", "Condition", "Temperature", "Humidity", "WindSpeed", "WeatherMultiplier"])

    print(f"[{now.strftime('%H:%M:%S')}] Updating {len(locations)} locations using Real APIs...")
    print(f"Traffic: TomTom API | Weather: Open-Meteo API")

    for loc in locations:
        # Get Real Traffic
        t_mult = get_thana_traffic(loc['lat'], loc['lon'])
        
        # Get Real Weather from Open-Meteo
        weather_data = get_real_weather(loc['lat'], loc['lon'])

        print(
            f"- {loc['name']}: traffic {t_mult:.1f} => {describe_traffic_factor(t_mult)} | "
            f"weather {weather_data['multiplier']:.1f} => {weather_data['condition']}"
        )
        
        traffic_rows.append([
            loc['name'], 
            now.year, now.month, now.day, now.hour, 
            f"{t_mult:.1f}"
        ])
        
        weather_rows.append([
            loc['name'], 
            now.year, now.month, now.day, now.hour,
            weather_data['condition'],
            f"{weather_data['temp']:.1f}",
            f"{weather_data['humidity']}",
            f"{weather_data['wind_speed']:.1f}",
            f"{weather_data['multiplier']}"
        ])
        
        # API Rate limit respect korte chotto pause
        time.sleep(0.3)

    # Save to CSVs
    write_live_csvs(traffic_rows, weather_rows)

    print("Success: traffic.csv and weather.csv updated with Live Data.")


def run_request_update(location_name, lat, lon):
    now = datetime.now()
    t_mult = get_thana_traffic(lat, lon)
    weather_data = get_real_weather(lat, lon)

    traffic_rows = [
        [location_name, now.year, now.month, now.day, now.hour, f"{t_mult:.1f}"],
    ]
    weather_rows = [
        [
            location_name,
            now.year,
            now.month,
            now.day,
            now.hour,
            f"{weather_data['multiplier']}",
        ],
    ]

    print(f"[{now.strftime('%H:%M:%S')}] Request-time live fetch for {location_name}")
    print(f"Traffic factor: {t_mult:.1f} => {describe_traffic_factor(t_mult)}")
    print(f"Weather factor: {weather_data['multiplier']:.1f} => {weather_data['condition']}")
    print(f"Weather details: temp {weather_data['temp']:.1f} C, humidity {weather_data['humidity']}%, wind {weather_data['wind_speed']:.1f} km/h")

    write_live_csvs(traffic_rows, weather_rows)
    print("Success: request-time traffic.csv and weather.csv updated.")

if __name__ == "__main__":
    if len(sys.argv) >= 2 and sys.argv[1].lower() == "request":
        if len(sys.argv) != 5:
            print("Usage: python live_updater.py request <location_name> <lat> <lon>")
            sys.exit(1)
        run_request_update(sys.argv[2], sys.argv[3], sys.argv[4])
    elif len(sys.argv) >= 2 and sys.argv[1].lower() == "once":
        run_live_update()
    else:
        # First update immediately
        run_live_update()
        
        # Loop for every 15 mins
        print(f"Scheduler started. Next update in {UPDATE_INTERVAL/60} minutes...")
        while True:
            try:
                time.sleep(UPDATE_INTERVAL)
                run_live_update()
            except KeyboardInterrupt:
                print("\nStopped by user.")
                break
