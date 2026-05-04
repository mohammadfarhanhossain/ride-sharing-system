import requests
import csv
import time
import os
from datetime import datetime

# --- CONFIGURATION ---
TOMTOM_API_KEY = "d9cd2979-3c65-4456-ab92-899b9d891379"
# Note: Weather API-r jonno apnar OpenWeatherMap key thakle ekhane boshaben. 
# Ekhonkar jonno ami ekta dynamic simulation logic rakhchi.
WEATHER_API_KEY = "PASTE_YOUR_WEATHER_API_KEY_HERE" 

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

def get_simulated_weather(hour):
    """Simulates weather based on time and a bit of randomness."""
    # Bristir sombhobona thakle multiplier barbe
    import random
    if 14 <= hour <= 19: # Bikeler dike bristi beshi hoy
        if random.random() < 0.2: return 1.5 # Rainy
    return 1.0

def run_live_update():
    locations = []
    location_file = 'location.csv'
    
    if not os.path.exists(location_file):
        print("Error: location.csv not found!")
        return

    # Load Locations
    with open(location_file, mode='r') as file:
        reader = csv.reader(file)
        for row in reader:
            if len(row) >= 3:
                locations.append({"name": row[0], "lat": row[1], "lon": row[2]})

    now = datetime.now()
    weather_rows = []
    traffic_rows = []

    print(f"[{now.strftime('%H:%M:%S')}] Updating {len(locations)} locations using TomTom API...")

    for loc in locations:
        # Get Real Traffic
        t_mult = get_thana_traffic(loc['lat'], loc['lon'])
        
        # Get Weather (Simulated or API)
        w_mult = get_simulated_weather(now.hour)
        
        traffic_rows.append([loc['name'], now.year, now.month, now.day, now.hour, f"{t_mult:.1f}"])
        weather_rows.append([loc['name'], now.year, now.month, now.day, now.hour, f"{w_mult:.1f}"])
        
        # API Rate limit respect korte chotto pause
        time.sleep(0.2)

    # Save to CSVs
    with open('traffic.csv', mode='w', newline='') as f:
        csv.writer(f).writerows(traffic_rows)
    with open('weather.csv', mode='w', newline='') as f:
        csv.writer(f).writerows(weather_rows)

    print("Success: traffic.csv and weather.csv updated with Live Data.")

if __name__ == "__main__":
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
