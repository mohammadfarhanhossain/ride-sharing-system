import csv
import random
from datetime import datetime, timedelta

def generate_data():
    locations = []
    try:
        with open('location.csv', mode='r') as file:
            reader = csv.reader(file)
            for row in reader:
                if row:
                    locations.append(row[0])
    except FileNotFoundError:
        locations = ["Agargaon", "Airport", "Badda", "Banani", "Gulshan-1", "Gulshan-2", "Dhanmondi"]

    start_date = datetime.now()
    # Generate data for 7 days
    num_days = 7
    
    traffic_rows = []
    weather_rows = []

    for day_offset in range(num_days):
        current_date = start_date + timedelta(days=day_offset)
        year = current_date.year
        month = current_date.month
        day = current_date.day

        for hour in range(24):
            for loc in locations:
                # Traffic logic: Rush hours (8-10 AM and 5-9 PM) have higher multipliers
                traffic_multiplier = 1.0
                if 8 <= hour <= 10:
                    traffic_multiplier = round(random.uniform(1.4, 1.8), 1)
                elif 17 <= hour <= 21:
                    traffic_multiplier = round(random.uniform(1.5, 2.0), 1)
                else:
                    traffic_multiplier = round(random.uniform(1.0, 1.3), 1)
                
                traffic_rows.append([loc, year, month, day, hour, traffic_multiplier])

                # Weather logic: Random rain/storm
                weather_multiplier = 1.0
                # 10% chance of bad weather
                if random.random() < 0.1:
                    weather_multiplier = round(random.uniform(1.3, 1.8), 1)
                
                weather_rows.append([loc, year, month, day, hour, weather_multiplier])

    with open('traffic.csv', mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(traffic_rows)

    with open('weather.csv', mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerows(weather_rows)

    print(f"Generated data for {len(locations)} locations over {num_days} days.")

if __name__ == "__main__":
    generate_data()
