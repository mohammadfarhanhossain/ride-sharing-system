import sys
import requests


def sanitize_text(value):
    if value is None:
        return ""
    text = str(value).strip().replace("\t", " ").replace("\n", " ").replace(",", " ")
    while "  " in text:
        text = text.replace("  ", " ")
    return text


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
        "count": 10,
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

    printed = 0
    for row in results:
        lat = row.get("latitude")
        lon = row.get("longitude")
        if lat is None or lon is None:
            continue

        name = sanitize_text(row.get("name", "Unknown"))
        admin1 = sanitize_text(row.get("admin1", ""))
        country = sanitize_text(row.get("country", ""))

        parts = [name]
        if admin1:
            parts.append(admin1)
        if country:
            parts.append(country)

        display = " - ".join(parts)
        print(f"{display}\t{lat}\t{lon}")
        printed += 1

    if printed == 0:
        print("", end="")
        raise SystemExit(1)


if __name__ == "__main__":
    main()
