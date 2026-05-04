import sys
import requests


def main():
    if len(sys.argv) < 2:
        print("", end="")
        raise SystemExit(1)

    query = " ".join(sys.argv[1:]).strip()
    if not query:
        print("", end="")
        raise SystemExit(1)

    url = "https://geocoding-api.open-meteo.com/v1/search"
    params = {
        "name": query,
        "count": 1,
        "language": "en",
        "format": "json",
    }

    try:
        response = requests.get(url, params=params, timeout=10)
        response.raise_for_status()
        data = response.json()
    except Exception:
        print("", end="")
        raise SystemExit(1)

    results = data.get("results")
    if not results:
        print("", end="")
        raise SystemExit(1)

    top = results[0]
    lat = top.get("latitude")
    lon = top.get("longitude")
    if lat is None or lon is None:
        print("", end="")
        raise SystemExit(1)

    print(f"{lat},{lon}")


if __name__ == "__main__":
    main()
