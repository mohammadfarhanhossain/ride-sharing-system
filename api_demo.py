from __future__ import annotations

import csv
import math
from datetime import datetime
from pathlib import Path
from typing import Any

from fastapi import FastAPI, HTTPException, Query
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel, Field


BASE_DIR = Path(__file__).resolve().parent
USERS_CSV = BASE_DIR / "users.csv"
DRIVERS_CSV = BASE_DIR / "drivers.csv"
RIDES_CSV = BASE_DIR / "rides.csv"
LOCATIONS_CSV = BASE_DIR / "location.csv"
TRAFFIC_CSV = BASE_DIR / "traffic.csv"
WEATHER_CSV = BASE_DIR / "weather.csv"
PRICING_CSV = BASE_DIR / "pricing_config.csv"

DEFAULT_PRICING = {
    "bike_base": 81.0,
    "bike_per_km": 7.8,
    "cng_base": 125.0,
    "cng_per_km": 28.4,
    "uberx_base": 331.0,
    "uberx_per_km": 22.6,
    "premium_base": 627.0,
    "premium_per_km": 36.2,
    "intercity_base": 127.1599332,
    "intercity_per_km": 30.7022556,
    "traffic_weight": 0.4,
    "weather_weight": 0.5,
    "demand_tier1_ratio": 0.3,
    "demand_tier1_surge": 0.20,
    "demand_tier2_ratio": 0.6,
    "demand_tier2_surge": 0.15,
    "demand_tier3_ratio": 1.0,
    "demand_tier3_surge": 0.05,
    "max_total_surge": 1.0,
}

VEHICLE_LABELS = {
    "bike": "Bike",
    "cng": "CNG",
    "uberx": "UberX",
    "premium": "Premium",
    "intercity": "Intercity",
}

app = FastAPI(title="RideShare API", version="1.0.0")
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class LoginRequest(BaseModel):
    username: str
    password: str


class FareRequest(BaseModel):
    distance: float = Field(gt=0)
    vehicle_type: str = Field(default="Bike")
    location: str = Field(default="")


class RideRequest(BaseModel):
    username: str
    password: str
    from_location: str
    to_location: str
    vehicle_type: str = Field(default="Bike")


def _read_rows(path: Path) -> list[list[str]]:
    if not path.exists():
        return []
    with path.open("r", newline="", encoding="utf-8") as file:
        return [row for row in csv.reader(file) if row]


def _write_rows(path: Path, rows: list[list[Any]]) -> None:
    with path.open("w", newline="", encoding="utf-8") as file:
        writer = csv.writer(file)
        writer.writerows(rows)


def _append_row(path: Path, row: list[Any]) -> None:
    with path.open("a", newline="", encoding="utf-8") as file:
        csv.writer(file).writerow(row)


def _split_favorites(value: str) -> list[str]:
    if not value:
        return []
    return [item.strip() for item in value.split(":") if item.strip()]


def _join_favorites(values: list[str]) -> str:
    return ":".join(values)


def _load_users() -> list[dict[str, Any]]:
    users: list[dict[str, Any]] = []
    for row in _read_rows(USERS_CSV):
        username = row[0].strip() if len(row) > 0 else ""
        password = row[1].strip() if len(row) > 1 else ""
        wallet = float(row[2]) if len(row) > 2 and row[2].strip() else 1000.0
        favorites = _split_favorites(row[3]) if len(row) > 3 else []
        if username:
            users.append(
                {
                    "username": username,
                    "password": password,
                    "wallet": wallet,
                    "favorites": favorites,
                }
            )
    return users


def _save_users(users: list[dict[str, Any]]) -> None:
    rows = []
    for user in users:
        rows.append(
            [
                user["username"],
                user["password"],
                f"{float(user['wallet']):.2f}",
                _join_favorites(list(user.get("favorites", []))),
            ]
        )
    _write_rows(USERS_CSV, rows)


def _load_drivers() -> list[dict[str, Any]]:
    drivers: list[dict[str, Any]] = []
    for row in _read_rows(DRIVERS_CSV):
        if len(row) < 9:
            continue
        try:
            drivers.append(
                {
                    "username": row[0].strip(),
                    "password": row[1].strip(),
                    "vehicle_type": row[2].strip(),
                    "location": row[3].strip(),
                    "available": row[4].strip() != "0",
                    "total_earn": float(row[5]) if row[5].strip() else 0.0,
                    "trip_count": int(row[6]) if row[6].strip() else 0,
                    "avg_rating": float(row[7]) if row[7].strip() else 0.0,
                    "rating_count": int(row[8]) if row[8].strip() else 0,
                }
            )
        except ValueError:
            continue
    return drivers


def _save_drivers(drivers: list[dict[str, Any]]) -> None:
    rows = []
    for driver in drivers:
        rows.append(
            [
                driver["username"],
                driver["password"],
                driver["vehicle_type"],
                driver["location"],
                "1" if driver["available"] else "0",
                f"{float(driver['total_earn']):.2f}",
                str(int(driver["trip_count"])),
                f"{float(driver['avg_rating']):.1f}",
                str(int(driver["rating_count"])),
            ]
        )
    _write_rows(DRIVERS_CSV, rows)


def _load_locations() -> dict[str, tuple[float, float]]:
    locations: dict[str, tuple[float, float]] = {}
    for row in _read_rows(LOCATIONS_CSV):
        if len(row) < 3:
            continue
        try:
            locations[row[0].strip()] = (float(row[1]), float(row[2]))
        except ValueError:
            continue
    return locations


def _load_pricing() -> dict[str, float]:
    pricing = dict(DEFAULT_PRICING)
    if not PRICING_CSV.exists():
        return pricing

    for row in _read_rows(PRICING_CSV):
        if len(row) < 2 or row[0].strip() == "key":
            continue
        key = row[0].strip()
        try:
            pricing[key] = float(row[1])
        except ValueError:
            continue
    return pricing


def _distance_km(from_location: str, to_location: str) -> float:
    locations = _load_locations()
    if from_location not in locations or to_location not in locations:
        raise HTTPException(status_code=404, detail="Unknown location")

    lat1, lon1 = locations[from_location]
    lat2, lon2 = locations[to_location]
    dx = lat1 - lat2
    dy = lon1 - lon2
    return math.sqrt(dx * dx + dy * dy) * 111.0


def _current_factor(path: Path, location: str) -> float:
    now = datetime.now()
    factor = 1.0
    for row in _read_rows(path):
        if len(row) < 6:
            continue
        if row[0].strip() != location:
            continue
        try:
            year = int(row[1])
            month = int(row[2])
            day = int(row[3])
            hour = int(row[4])
            if (year, month, day, hour) == (now.year, now.month, now.day, now.hour):
                factor = float(row[-1])
        except ValueError:
            continue
    return factor


def _vehicle_key(vehicle_type: str) -> str:
    normalized = vehicle_type.strip().lower()
    if normalized not in VEHICLE_LABELS:
        raise HTTPException(status_code=400, detail="Unsupported vehicle type")
    return normalized


def _vehicle_label(vehicle_type: str) -> str:
    return VEHICLE_LABELS[_vehicle_key(vehicle_type)]


def _base_fare(distance: float, vehicle_type: str, pricing: dict[str, float]) -> tuple[float, float]:
    key = _vehicle_key(vehicle_type)
    base_map = {
        "bike": (pricing["bike_base"], pricing["bike_per_km"]),
        "cng": (pricing["cng_base"], pricing["cng_per_km"]),
        "uberx": (pricing["uberx_base"], pricing["uberx_per_km"]),
        "premium": (pricing["premium_base"], pricing["premium_per_km"]),
        "intercity": (pricing["intercity_base"], pricing["intercity_per_km"]),
    }
    base, per_km = base_map[key]
    return base, base + (distance * per_km)


def _fare_breakdown(distance: float, vehicle_type: str, location: str = "") -> dict[str, Any]:
    pricing = _load_pricing()
    base_charge, base_fare = _base_fare(distance, vehicle_type, pricing)

    traffic_factor = _current_factor(TRAFFIC_CSV, location) if location else 1.0
    weather_factor = _current_factor(WEATHER_CSV, location) if location else 1.0

    traffic_surge = max(traffic_factor - 1.0, 0.0) * pricing["traffic_weight"]
    weather_surge = max(weather_factor - 1.0, 0.0) * pricing["weather_weight"]
    total_surge = min(max(traffic_surge + weather_surge, 0.0), pricing["max_total_surge"])

    return {
        "pricing": pricing,
        "base_charge": base_charge,
        "base_fare": base_fare,
        "traffic_factor": traffic_factor,
        "weather_factor": weather_factor,
        "traffic_surge": traffic_surge,
        "weather_surge": weather_surge,
        "total_surge": total_surge,
        "estimated_fare": round(base_fare * (1.0 + total_surge), 2),
    }


def _find_login(username: str, password: str) -> dict[str, Any]:
    if username == "admin" and password == "123456":
        return {"role": "admin", "username": "admin"}

    for user in _load_users():
        if user["username"] == username and user["password"] == password:
            return {"role": "user", **user}

    for driver in _load_drivers():
        if driver["username"] == username and driver["password"] == password:
            return {"role": "driver", **driver}

    raise HTTPException(status_code=401, detail="Invalid credentials")


def _choose_driver(vehicle_type: str, from_location: str) -> dict[str, Any]:
    drivers = _load_drivers()
    matches = [driver for driver in drivers if driver["vehicle_type"].lower() == _vehicle_key(vehicle_type) and driver["available"]]
    if not matches:
        raise HTTPException(status_code=404, detail="No available driver for that vehicle type")

    locations = _load_locations()
    if from_location not in locations:
        raise HTTPException(status_code=404, detail="Unknown pickup location")

    def sort_key(driver: dict[str, Any]) -> tuple[float, int, str]:
        driver_location = driver["location"]
        if driver_location in locations:
            lat1, lon1 = locations[from_location]
            lat2, lon2 = locations[driver_location]
            dx = lat1 - lat2
            dy = lon1 - lon2
            distance = math.sqrt(dx * dx + dy * dy)
        else:
            distance = float("inf")
        return (distance, driver["trip_count"], driver["username"])

    return min(matches, key=sort_key)


def _save_ride(
    username: str,
    driver_username: str,
    from_location: str,
    to_location: str,
    distance: float,
    fare: float,
    vehicle_type: str,
) -> None:
    _append_row(
        RIDES_CSV,
        [
            datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            username,
            driver_username,
            from_location,
            to_location,
            f"{distance:.5f}",
            f"{fare:.2f}",
            _vehicle_label(vehicle_type),
            "Wallet",
        ],
    )


@app.get("/")
def home() -> dict[str, str]:
    return {"message": "Welcome to the RideShare API", "status": "Online"}


@app.get("/health")
def health() -> dict[str, str]:
    return {"status": "ok"}


@app.post("/login")
def login(payload: LoginRequest) -> dict[str, Any]:
    return _find_login(payload.username, payload.password)


@app.get("/users")
def list_users() -> list[dict[str, Any]]:
    return _load_users()


@app.get("/drivers")
def list_drivers(vehicle_type: str | None = None, available: bool | None = None) -> list[dict[str, Any]]:
    drivers = _load_drivers()
    if vehicle_type is not None:
        vehicle_key = _vehicle_key(vehicle_type)
        drivers = [driver for driver in drivers if driver["vehicle_type"].lower() == vehicle_key]
    if available is not None:
        drivers = [driver for driver in drivers if driver["available"] is available]
    return drivers


@app.get("/locations")
def list_locations() -> list[str]:
    return sorted(_load_locations().keys())


@app.get("/fare")
def calculate_fare(distance: float, vehicle_type: str = "Bike", location: str = "") -> dict[str, Any]:
    quote = _fare_breakdown(distance, vehicle_type, location)
    vehicle_label = _vehicle_label(vehicle_type)

    return {
        "vehicle": vehicle_label,
        "distance": round(distance, 2),
        "base_charge": round(quote["base_charge"], 2),
        "estimated_fare": quote["estimated_fare"],
        "currency": "Taka",
        "traffic_factor": round(quote["traffic_factor"], 2),
        "weather_factor": round(quote["weather_factor"], 2),
        "surge_percent": round(quote["total_surge"] * 100.0, 2),
    }


@app.post("/rides/book")
def book_ride(payload: RideRequest) -> dict[str, Any]:
    user_login = _find_login(payload.username, payload.password)
    if user_login["role"] != "user":
        raise HTTPException(status_code=403, detail="Only users can book rides")

    users = _load_users()
    user = next((entry for entry in users if entry["username"] == payload.username), None)
    if user is None:
        raise HTTPException(status_code=404, detail="User not found")

    distance = _distance_km(payload.from_location, payload.to_location)
    quote = _fare_breakdown(distance, payload.vehicle_type, payload.from_location)
    fare = quote["estimated_fare"]

    if user["wallet"] < fare:
        raise HTTPException(status_code=400, detail="Insufficient wallet balance")

    driver = _choose_driver(payload.vehicle_type, payload.from_location)
    drivers = _load_drivers()
    driver_entry = next((entry for entry in drivers if entry["username"] == driver["username"]), None)
    if driver_entry is None:
        raise HTTPException(status_code=404, detail="Driver not found")

    user["wallet"] = round(user["wallet"] - fare, 2)
    driver_entry["total_earn"] = round(driver_entry["total_earn"] + (fare * 0.8), 2)
    driver_entry["trip_count"] += 1
    driver_entry["location"] = payload.to_location
    driver_entry["available"] = True

    _save_users(users)
    _save_drivers(drivers)
    _save_ride(
        username=payload.username,
        driver_username=driver_entry["username"],
        from_location=payload.from_location,
        to_location=payload.to_location,
        distance=distance,
        fare=fare,
        vehicle_type=payload.vehicle_type,
    )

    return {
        "status": "confirmed",
        "username": payload.username,
        "driver": driver_entry["username"],
        "vehicle": _vehicle_label(payload.vehicle_type),
        "from_location": payload.from_location,
        "to_location": payload.to_location,
        "distance_km": round(distance, 2),
        "fare": round(fare, 2),
        "remaining_wallet": round(user["wallet"], 2),
    }


@app.get("/rides")
def list_rides() -> list[dict[str, Any]]:
    rides: list[dict[str, Any]] = []
    for row in _read_rows(RIDES_CSV):
        if len(row) < 9:
            continue
        try:
            rides.append(
                {
                    "timestamp": row[0],
                    "user": row[1],
                    "driver": row[2],
                    "from_location": row[3],
                    "to_location": row[4],
                    "distance": float(row[5]),
                    "fare": float(row[6]),
                    "vehicle": row[7],
                    "payment": row[8],
                }
            )
        except ValueError:
            continue
    return rides


@app.post("/wallet/topup")
def topup_wallet(payload: LoginRequest, amount: float = Query(gt=0)) -> dict[str, Any]:
    _find_login(payload.username, payload.password)
    users = _load_users()
    user = next((entry for entry in users if entry["username"] == payload.username), None)
    if user is None:
        raise HTTPException(status_code=404, detail="User not found")

    user["wallet"] = round(user["wallet"] + amount, 2)
    _save_users(users)
    return {"username": payload.username, "wallet": user["wallet"]}
