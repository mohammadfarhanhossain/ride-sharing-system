import pandas as pd
import folium
from folium.plugins import HeatMap
import sys
import os
import webbrowser
import requests

def load_locations():
    if not os.path.exists('location.csv'):
        return None
    try:
        df = pd.read_csv('location.csv', names=['name', 'lat', 'lon'])
        return df
    except Exception:
        return None

def get_route(p1, p2):
    """Fetches real road routing from OSRM API."""
    try:
        # OSRM expects coordinates as lon,lat;lon,lat
        url = f"http://router.project-osrm.org/route/v1/driving/{p1[1]},{p1[0]};{p2[1]},{p2[0]}?overview=full&geometries=geojson"
        r = requests.get(url, timeout=5)
        res = r.json()
        if res['code'] == 'Ok':
            # GeoJSON uses [lon, lat], Folium uses [lat, lon]
            route = [[point[1], point[0]] for point in res['routes'][0]['geometry']['coordinates']]
            return route
    except Exception as e:
        print(f"Routing Error: {e}")
    return [p1, p2] # Fallback to straight line

def main():
    if len(sys.argv) < 2:
        print("Usage: python map_generator.py <system|ride|heatmap> [from_loc] [to_loc]")
        sys.exit(1)

    mode = sys.argv[1].lower()
    df = load_locations()
    if df is None or df.empty:
        print("Error: Could not load locations.")
        sys.exit(1)

    # Dhaka center - Dark Mode Tile
    m = folium.Map(location=[23.8103, 90.4125], zoom_start=12, tiles='CartoDB dark_matter')

    if mode == "system":
        for _, row in df.iterrows():
            folium.Marker(
                location=[row['lat'], row['lon']],
                popup=row['name'],
                tooltip=row['name'],
                icon=folium.Icon(color='blue', icon='info-sign')
            ).add_to(m)
        filename = "system_map.html"
    
    elif mode == "heatmap":
        # Simulate density based on locations (In a real app, this would use ride request data)
        heat_data = [[row['lat'], row['lon'], 1] for _, row in df.iterrows()]
        HeatMap(heat_data, radius=25, blur=15, gradient={0.4: 'blue', 0.65: 'lime', 1: 'red'}).add_to(m)
        filename = "heatmap.html"

    elif mode == "ride":
        if len(sys.argv) < 4:
            print("Usage: python map_generator.py ride <from_loc> <to_loc>")
            sys.exit(1)
        
        from_name = sys.argv[2]
        to_name = sys.argv[3]
        
        from_row = df[df['name'].str.lower() == from_name.lower()]
        to_row = df[df['name'].str.lower() == to_name.lower()]
        
        if from_row.empty or to_row.empty:
            print(f"Error: One or both locations not found: {from_name}, {to_name}")
            sys.exit(1)
            
        p1 = [from_row.iloc[0]['lat'], from_row.iloc[0]['lon']]
        p2 = [to_row.iloc[0]['lat'], to_row.iloc[0]['lon']]
        
        folium.Marker(p1, popup=f"Pickup: {from_name}", icon=folium.Icon(color='green', icon='play')).add_to(m)
        folium.Marker(p2, popup=f"Destination: {to_name}", icon=folium.Icon(color='red', icon='stop')).add_to(m)
        
        # Real Road Routing
        route = get_route(p1, p2)
        folium.PolyLine(route, color="#3498db", weight=5, opacity=0.8).add_to(m)
        
        filename = "ride_map.html"
    else:
        print("Unknown mode.")
        sys.exit(1)

    m.save(filename)
    webbrowser.open('file://' + os.path.realpath(filename))
    print(f"Map saved as {filename} and opened in browser.")

if __name__ == "__main__":
    main()
