#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <cctype>
#include <cstdio>
#include <windows.h>
#include <conio.h>

using namespace std;

class Money
{
private:
    double amount;

public:
    Money() : amount(0) {}
    Money(double a) : amount(a) {}

    double get() const { return amount; }

    Money operator+(const Money &other) const { return Money(amount + other.amount); }
    Money operator-(const Money &other) const { return Money(amount - other.amount); }

    friend ostream &operator<<(ostream &out, const Money &m)
    {
        out << fixed << setprecision(2) << m.amount;
        return out;
    }
};

struct Location
{
    string name;
    double lat, lon;
    Location(string n, double la, double lo) : name(n), lat(la), lon(lo) {}
};

class Company
{
private:
    double totalRevenue, commissionRate;

public:
    Company() : totalRevenue(0), commissionRate(0.20) {}

    void addRideRevenue(double fare) { totalRevenue += fare * commissionRate; }

    double getTotalRevenue() const { return totalRevenue; }
    double getDriverPart(double fare) const { return fare * (1.0 - commissionRate); }
};

Company company;

class Person
{
protected:
    string username, password;
    
public:
    Person(string u, string p) : username(u), password(p) {}
    virtual ~Person() {}

    string getUsername() const { return username; }
    string getPassword() const { return password; }

    bool login(string u, string p) { return (username == u && password == p); }

};

class User : public Person
{
private:
    Money wallet;

public:
    User(string u, string p, Money w = Money(1000)) : Person(u, p), wallet(w) {}


    Money getWallet() const { return wallet; }

    // function overloading
    void addMoney(int x) { wallet = wallet + Money((double)x); }
    void addMoney(double x) { wallet = wallet + Money(x); }

    bool pay(double fare)
    {
        if (wallet.get() < fare)
            return false;
        wallet = wallet - Money(fare);
        return true;
    }
};

class Driver : public Person
{
private:
    string vehicleType;
    string location;
    bool available;
    int tripCount;
    double totalEarn;
    double avgRating;
    int ratingCount;

public:
    Driver(string u, string p, string v, string l)
        : Person(u, p), vehicleType(v), location(l), available(true), tripCount(0), totalEarn(0), avgRating(0.0), ratingCount(0) {}

    // constructor with persisted stats
    Driver(string u, string p, string v, string l, bool avail, double totalEarn_, int tripCount_, double avgRating_, int ratingCount_)
        : Person(u, p), vehicleType(v), location(l), available(avail), tripCount(tripCount_), totalEarn(totalEarn_), avgRating(avgRating_), ratingCount(ratingCount_) {}


    string getVehicleType() const { return vehicleType; }
    string getLocation() const { return location; }
    bool isAvailable() const { return available; }
    int getTripCount() const { return tripCount; }
    double getTotalEarn() const { return totalEarn; }
    double getAvgRating() const { return avgRating; }
    int getRatingCount() const { return ratingCount; }

    void setLocation(string l) { this->location = l; }
    void setAvailable(bool value) { available = value; }

    void addTrip(double earning)
    {
        tripCount++;
        totalEarn += earning;
    }
    
    void addRating(int r)
    {
        double totalPoints = (avgRating * ratingCount) + r;
        ratingCount++;
        avgRating = totalPoints / ratingCount;
    }
};

class Admin : public Person
{
public:
    Admin() : Person("admin", "123456") {}
};

class Vehicle
{
public:
    virtual ~Vehicle() {}
    virtual string name() const = 0;
    virtual double factor() const = 0;
    virtual double speed() const = 0;
};

class Bike : public Vehicle
{
public:
    string name() const override { return "Bike"; }
    double factor() const override { return 17.8; }
    double speed() const override { return 24; }
};

class CNG : public Vehicle
{
public:
    string name() const override { return "CNG"; }
    double factor() const override { return 22.6; }
    double speed() const override { return 20; }
};

class UberX : public Vehicle
{
public:
    string name() const override { return "UberX"; }
    double factor() const override { return 28.4; }
    double speed() const override { return 27; }
};

class Premium : public Vehicle
{
public:
    string name() const override { return "Premium"; }
    double factor() const override { return 36.2; }
    double speed() const override { return 24; }
};

struct Ride
{
    string timeStamp;
    string user;
    string driver;
    string from;
    string to;
    double distance;
    double fare;
    string vehicle;
    string payment;

    Ride() : distance(0), fare(0), payment("Wallet") {}
};

template <typename T>
class InputValidator {
public:
    static T getInput(const string &value) {
        T x;
        while (true) {
            cout << value;
            if (cin >> x)
                return x;
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Try again.\n";
        }
    }
};

class RideShareSystem
{
private:
    struct PricingConfig
    {
        double bikeBase = 81.0;
        double bikePerKm = 7.8;
        double cngBase = 125.0;
        double cngPerKm = 28.4;
        double uberXBase = 331.0;
        double uberXPerKm = 22.6;
        double premiumBase = 627.0;
        double premiumPerKm = 36.2;

        double trafficWeight = 0.4;
        double weatherWeight = 0.5;

        double demandTier1Ratio = 0.3;
        double demandTier1Surge = 0.20;
        double demandTier2Ratio = 0.6;
        double demandTier2Surge = 0.15;
        double demandTier3Ratio = 1.0;
        double demandTier3Surge = 0.05;

        double maxTotalSurge = 1.0;
    };

    vector<Location> locations;
    vector<User> users;
    vector<Driver> drivers;
    struct TrafficData
    {
        string location;
        int year, month, day, hour;
        double factor;

        TrafficData(string loc, int y, int m, int d, int h, double f)
            : location(loc), year(y), month(m), day(d), hour(h), factor(f) {}
    };
    struct WeatherData
    {
        string location;
        int year, month, day, hour;
        double factor;

        WeatherData(string loc, int y, int m, int d, int h, double f)
            : location(loc), year(y), month(m), day(d), hour(h), factor(f) {}
    };
    vector<TrafficData> trafficData;
    vector<WeatherData> weatherData;
    vector<Ride> rides;
    Admin admin;
    PricingConfig pricing;

private:
    unique_ptr<Vehicle> makeVehicle(int choice) const
    {
        switch (choice)
        {
        case 1:
            return make_unique<Bike>();
        case 2:
            return make_unique<CNG>();
        case 3:
            return make_unique<UberX>();
        case 4:
            return make_unique<Premium>();
        default:
            return nullptr;
        }
    }

string nowTime()
{
    time_t currentTime = time(0);
    tm *localTime = localtime(&currentTime);

    char timeText[30];
    strftime(timeText, 30, "%Y-%m-%d %H:%M:%S", localTime);

    return timeText;
}

    // Simulate live countdown with carriage return (no new lines)
    void simulateRideProgress(int driverWaitTime, int journeyTime)
    {
        cout << "\n====================================\n";
        cout << "    LIVE RIDE TRACKING SYSTEM       \n";
        cout << "====================================\n\n";

        // Phase 1: Driver arriving
        cout << "[Status] Driver is on the way...\n";
        for (int t = driverWaitTime; t > 0; t--)
        {
            // \r moves cursor to beginning of same line, flush sathe screen sathe sathe update kora hoy
            cout << "\r[ETA] Driver arriving in: " << t << " mins   " << flush;
            Sleep(1000); // 1 second in milliseconds
        }
        cout << "\r[Status] Driver has arrived! Hop in.         \n\n";

        // Phase 2: Ride in progress
        cout << "[Status] Ride Started!\n";
        for (int t = journeyTime; t > 0; t--)
        {
            cout << "\r[Ride] Time to Destination: " << t << " mins remaining...  " << flush;
            Sleep(1000); // 1 second in milliseconds
        }

        cout << "\r[Status] Destination reached! Ride completed successfully.\n";
        cout << "====================================\n\n";
    }

    double getVehicleBase(const string &vehicleType) const
    {
        if (vehicleType == "Bike") return pricing.bikeBase;
        if (vehicleType == "CNG") return pricing.cngBase;
        if (vehicleType == "UberX") return pricing.uberXBase;
        if (vehicleType == "Premium") return pricing.premiumBase;
        return 0.0;
    }

    double getVehiclePerKm(const string &vehicleType) const
    {
        if (vehicleType == "Bike") return pricing.bikePerKm;
        if (vehicleType == "CNG") return pricing.cngPerKm;
        if (vehicleType == "UberX") return pricing.uberXPerKm;
        if (vehicleType == "Premium") return pricing.premiumPerKm;
        return 0.0;
    }

    string lowerCopy(string s) const
    {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(tolower(c));
        });
        return s;
    }

    int findLocationIndexCaseInsensitive(const string &name) const
    {
        string target = lowerCopy(name);
        for (size_t i = 0; i < locations.size(); ++i)
        {
            if (lowerCopy(locations[i].name) == target)
                return static_cast<int>(i);
        }
        return -1;
    }

    bool sanitizePricingConfig(bool verbose)
    {
        bool changed = false;

        auto clampVal = [&](double &value, double lo, double hi, const string &label) {
            double old = value;
            value = min(max(value, lo), hi);
            if (old != value)
            {
                changed = true;
                if (verbose)
                    cout << "[Pricing Config] " << label << " out of range. Clamped to " << value << "\n";
            }
        };

        clampVal(pricing.bikeBase, 0.0, 5000.0, "bike_base");
        clampVal(pricing.cngBase, 0.0, 5000.0, "cng_base");
        clampVal(pricing.uberXBase, 0.0, 5000.0, "uberx_base");
        clampVal(pricing.premiumBase, 0.0, 5000.0, "premium_base");

        clampVal(pricing.bikePerKm, 0.0, 500.0, "bike_per_km");
        clampVal(pricing.cngPerKm, 0.0, 500.0, "cng_per_km");
        clampVal(pricing.uberXPerKm, 0.0, 500.0, "uberx_per_km");
        clampVal(pricing.premiumPerKm, 0.0, 500.0, "premium_per_km");

        clampVal(pricing.trafficWeight, 0.0, 2.0, "traffic_weight");
        clampVal(pricing.weatherWeight, 0.0, 2.0, "weather_weight");

        clampVal(pricing.demandTier1Ratio, 0.05, 3.0, "demand_tier1_ratio");
        clampVal(pricing.demandTier2Ratio, 0.05, 3.0, "demand_tier2_ratio");
        clampVal(pricing.demandTier3Ratio, 0.05, 3.0, "demand_tier3_ratio");

        clampVal(pricing.demandTier1Surge, 0.0, 1.0, "demand_tier1_surge");
        clampVal(pricing.demandTier2Surge, 0.0, 1.0, "demand_tier2_surge");
        clampVal(pricing.demandTier3Surge, 0.0, 1.0, "demand_tier3_surge");

        clampVal(pricing.maxTotalSurge, 0.0, 3.0, "max_total_surge");

        if (!(pricing.demandTier1Ratio < pricing.demandTier2Ratio && pricing.demandTier2Ratio < pricing.demandTier3Ratio))
        {
            pricing.demandTier1Ratio = 0.3;
            pricing.demandTier2Ratio = 0.6;
            pricing.demandTier3Ratio = 1.0;
            changed = true;
            if (verbose)
                cout << "[Pricing Config] Invalid ratio order. Reset tiers to defaults: 0.3, 0.6, 1.0\n";
        }

        return changed;
    }

    void savePricingConfig()
    {
        sanitizePricingConfig(false);
        ofstream fout("pricing_config.csv");
        fout << "key,value\n";
        fout << "bike_base," << pricing.bikeBase << "\n";
        fout << "bike_per_km," << pricing.bikePerKm << "\n";
        fout << "cng_base," << pricing.cngBase << "\n";
        fout << "cng_per_km," << pricing.cngPerKm << "\n";
        fout << "uberx_base," << pricing.uberXBase << "\n";
        fout << "uberx_per_km," << pricing.uberXPerKm << "\n";
        fout << "premium_base," << pricing.premiumBase << "\n";
        fout << "premium_per_km," << pricing.premiumPerKm << "\n";
        fout << "traffic_weight," << pricing.trafficWeight << "\n";
        fout << "weather_weight," << pricing.weatherWeight << "\n";
        fout << "demand_tier1_ratio," << pricing.demandTier1Ratio << "\n";
        fout << "demand_tier1_surge," << pricing.demandTier1Surge << "\n";
        fout << "demand_tier2_ratio," << pricing.demandTier2Ratio << "\n";
        fout << "demand_tier2_surge," << pricing.demandTier2Surge << "\n";
        fout << "demand_tier3_ratio," << pricing.demandTier3Ratio << "\n";
        fout << "demand_tier3_surge," << pricing.demandTier3Surge << "\n";
        fout << "max_total_surge," << pricing.maxTotalSurge << "\n";
    }

    void loadPricingConfig()
    {
        ifstream fin("pricing_config.csv");
        if (!fin)
        {
            savePricingConfig();
            return;
        }

        string line;
        while (getline(fin, line))
        {
            if (line.empty()) continue;
            stringstream ss(line);
            string key, value;
            getline(ss, key, ',');
            getline(ss, value, ',');
            if (key == "key" || value.empty()) continue;

            double v = 0.0;
            try
            {
                v = stod(value);
            }
            catch (...)
            {
                continue;
            }

            if (key == "bike_base") pricing.bikeBase = v;
            else if (key == "bike_per_km") pricing.bikePerKm = v;
            else if (key == "cng_base") pricing.cngBase = v;
            else if (key == "cng_per_km") pricing.cngPerKm = v;
            else if (key == "uberx_base") pricing.uberXBase = v;
            else if (key == "uberx_per_km") pricing.uberXPerKm = v;
            else if (key == "premium_base") pricing.premiumBase = v;
            else if (key == "premium_per_km") pricing.premiumPerKm = v;
            else if (key == "traffic_weight") pricing.trafficWeight = v;
            else if (key == "weather_weight") pricing.weatherWeight = v;
            else if (key == "demand_tier1_ratio") pricing.demandTier1Ratio = v;
            else if (key == "demand_tier1_surge") pricing.demandTier1Surge = v;
            else if (key == "demand_tier2_ratio") pricing.demandTier2Ratio = v;
            else if (key == "demand_tier2_surge") pricing.demandTier2Surge = v;
            else if (key == "demand_tier3_ratio") pricing.demandTier3Ratio = v;
            else if (key == "demand_tier3_surge") pricing.demandTier3Surge = v;
            else if (key == "max_total_surge") pricing.maxTotalSurge = v;
        }

        if (sanitizePricingConfig(true))
            savePricingConfig();
    }

    bool geocodeLocationName(const string &query, double &lat, double &lon)
    {
        ostringstream cmd;
        cmd << "python geocode_lookup.py \"" << query << "\" > geocode_result.tmp";
        int result = system(cmd.str().c_str());
        if (result != 0)
            return false;

        ifstream fin("geocode_result.tmp");
        string line;
        bool ok = false;
        if (getline(fin, line))
        {
            stringstream ss(line);
            string latStr, lonStr;
            getline(ss, latStr, ',');
            getline(ss, lonStr, ',');
            if (!latStr.empty() && !lonStr.empty())
            {
                lat = stod(latStr);
                lon = stod(lonStr);
                ok = true;
            }
        }

        remove("geocode_result.tmp");
        return ok;
    }
    double calcDistance(const Location &a, const Location &b)
    {
        // very simple distance logic (beginner friendly)
        double dx = a.lat - b.lat;
        double dy = a.lon - b.lon;
        return sqrt(dx * dx + dy * dy) * 111.0;
    }

    int findLocationIndex(const string &name) {
        auto it = find_if(locations.begin(), locations.end(),
                      [&](const Location& loc){ return loc.name == name; });
        return (it != locations.end()) ? (it - locations.begin()) : -1;
    }

    int findNearestDriver(const Location &from, const string &vehicleType)
{
    int best = -1;
    double bestDist = 10000.0;

    for (size_t i = 0; i < drivers.size(); ++i)
    {
        if (drivers[i].getVehicleType() == vehicleType && drivers[i].isAvailable())
        {
            int locIdx = findLocationIndex(drivers[i].getLocation());

            if (locIdx >= 0)
            {
                double d = calcDistance(locations[static_cast<size_t>(locIdx)], from);

                if (d < bestDist)
                {
                    bestDist = d;
                    best = static_cast<int>(i);
                }
            }
        }
    }

    return best;
}

    int countAvailableDrivers(const string &vehicleType) const
    {
        
        return static_cast<int>(count_if(drivers.begin(), drivers.end(), [&](const Driver& d){
            return d.getVehicleType() == vehicleType && d.isAvailable();
        }));
    }

 void loadTraffic()
{
    ifstream fin("traffic.csv");
    if (!fin)
        return;

    string line;
    while (getline(fin, line))
    {
        if (line.empty()) continue;
        stringstream ss(line);
        string loc, yearStr, monthStr, dayStr, hourStr, factorStr;

        getline(ss, loc, ',');
        getline(ss, yearStr, ',');
        getline(ss, monthStr, ',');
        getline(ss, dayStr, ',');
        getline(ss, hourStr, ',');
        getline(ss, factorStr, ',');

        if (loc == "Location" || yearStr == "Year" || factorStr == "TrafficMultiplier")
            continue;

        trafficData.push_back(TrafficData(loc, stoi(yearStr), stoi(monthStr), stoi(dayStr), stoi(hourStr), stod(factorStr)));
    }
}

void loadWeather()
{
    ifstream fin("weather.csv");
    if (!fin)
        return;

    string line;

    while (getline(fin, line))
    {
        if (line.empty()) continue;
        stringstream ss(line);
        string loc, yearStr, monthStr, dayStr, hourStr, factorStr;

        getline(ss, loc, ',');
        getline(ss, yearStr, ',');
        getline(ss, monthStr, ',');
        getline(ss, dayStr, ',');
        getline(ss, hourStr, ',');
        getline(ss, factorStr, ',');

        if (loc == "Location" || yearStr == "Year" || factorStr == "WeatherMultiplier")
            continue;

        weatherData.push_back(WeatherData(loc, stoi(yearStr), stoi(monthStr), stoi(dayStr), stoi(hourStr), stod(factorStr)));
    }
}

    string describeTrafficFactor(double factor) const
    {
        if (factor >= 1.9)
            return "Heavy Traffic";
        if (factor >= 1.5)
            return "Medium Traffic";
        if (factor >= 1.2)
            return "Light-Medium Traffic";
        return "Free Flow";
    }

    string describeWeatherFactor(double factor) const
    {
        if (factor >= 1.8)
            return "Heavy showers / very bad weather";
        if (factor >= 1.5)
            return "Rainy";
        if (factor >= 1.3)
            return "Foggy or windy";
        return "Clear / normal";
    }

    bool refreshLiveDataForRequest(const Location &from)
    {
        ostringstream cmd;
        cmd << "python live_updater.py request \"" << from.name << "\" "
            << fixed << setprecision(6) << from.lat << " " << from.lon;

        int result = system(cmd.str().c_str());
        if (result != 0)
            return false;

        trafficData.clear();
        weatherData.clear();
        loadTraffic();
        loadWeather();
        return true;
    }


double getTrafficFactor(string locName)
{
    time_t currentTime = time(nullptr);
    tm *lt = localtime(&currentTime);
    
    int y = lt->tm_year + 1900;
    int m = lt->tm_mon + 1;
    int d = lt->tm_mday;
    int h = lt->tm_hour;

    for (const auto &traffic : trafficData)
    {
        if (traffic.location == locName && traffic.year == y && 
            traffic.month == m && traffic.day == d && traffic.hour == h)
        {
            return traffic.factor;
        }
    }

    return 1.0;
}
    
    double getWeatherFactor(string locName)
    {
        time_t currentTime = time(nullptr);
        tm *lt = localtime(&currentTime);
        
        int y = lt->tm_year + 1900;
        int m = lt->tm_mon + 1;
        int d = lt->tm_mday;
        int h = lt->tm_hour;

        for (const auto &wd : weatherData)
        {
            if (wd.location == locName && wd.year == y && 
                wd.month == m && wd.day == d && wd.hour == h)
            {
                return wd.factor;
            }
        }
        return 1.0;
    }

    double calculateDynamicFare(double distance, const string &vehicleType,
                                int availableDrivers, int activeUsers, string locName)
    {
        double baseCharge = getVehicleBase(vehicleType);
        double perKmRate = getVehiclePerKm(vehicleType);

        double baseFare = baseCharge + (distance * perKmRate);
        double trafficSurge = (getTrafficFactor(locName) - 1.0) * pricing.trafficWeight;
        double weatherSurge = (getWeatherFactor(locName) - 1.0) * pricing.weatherWeight;

        double demandSurge = 0.0;
        if (activeUsers > 0)
        {
            double ratio = (double)availableDrivers / (double)activeUsers;
            if (ratio < pricing.demandTier1Ratio)
            {
                demandSurge = pricing.demandTier1Surge;
            }
            else if (ratio < pricing.demandTier2Ratio)
            {
                demandSurge = pricing.demandTier2Surge;
            }
            else if (ratio < pricing.demandTier3Ratio)
            {
                demandSurge = pricing.demandTier3Surge;
            }
        }

        double totalSurge = trafficSurge + weatherSurge + demandSurge;
        if (totalSurge > pricing.maxTotalSurge)
            totalSurge = pricing.maxTotalSurge;
        if (totalSurge < 0.0)
            totalSurge = 0.0;

        double finalFare = baseFare * (1.0 + totalSurge);
        return round(finalFare / 5.0) * 5.0;
    }

    void loadLocations()
    {
        ifstream fin("location.csv");
        if (!fin)
            throw runtime_error("location.csv not found");

        string line;

        while (getline(fin, line))
        {
            stringstream ss(line);
            string n, la, lo;
            getline(ss, n, ',');
            getline(ss, la, ',');
            getline(ss, lo, ',');

            locations.push_back(Location(n, stod(la), stod(lo)));
        }
    }

    void loadUsers()
    {
        ifstream fin("users.txt");
        if (!fin)
            throw runtime_error("users.txt not found");

        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string u, p;
            double walletAmount = 1000.0;

            ss >> u >> p;
            if (ss >> walletAmount)
            {
                users.push_back(User(u, p, Money(walletAmount)));
            }
            else
            {
                users.push_back(User(u, p));
            }
        }
    }

    void saveUsers()
    {
        ofstream fout("users.txt");
        for (const auto &u : users)
        {
            fout << u.getUsername() << " " << u.getPassword() << " " << fixed << setprecision(2) << u.getWallet() << "\n";
        }
    }

    void saveLocations()
    {
        ofstream fout("location.csv");
        for (const auto &l : locations)
        {
            fout << l.name << "," << fixed << setprecision(3) << l.lat << "," << fixed << setprecision(3) << l.lon << "\n";
        }
    }

    void loadDrivers()
    {
        ifstream fin("drivers.csv");
        if (!fin)
            throw runtime_error("drivers.csv not found");

        string line;
        while (getline(fin, line))
        {
            stringstream ss(line);
            string id, pass, v, loc, availableStr, earnStr, tripStr, avgRatingStr, ratingCountStr;
            getline(ss, id, ',');
            getline(ss, pass, ',');
            getline(ss, v, ',');
            getline(ss, loc, ',');
            getline(ss, availableStr, ',');
            getline(ss, earnStr, ',');
            getline(ss, tripStr, ',');
            getline(ss, avgRatingStr, ',');
            getline(ss, ratingCountStr, ',');

            bool availFlag = (availableStr != "0");
            double totalEarnVal = stod(earnStr);
            int tripCnt = stoi(tripStr);
            double avgRatingVal = 0.0;
            int ratingCntVal = 0;
            if (!avgRatingStr.empty()) avgRatingVal = stod(avgRatingStr);
            if (!ratingCountStr.empty()) ratingCntVal = stoi(ratingCountStr);

            drivers.push_back(Driver(id, pass, v, loc, availFlag, totalEarnVal, tripCnt, avgRatingVal, ratingCntVal));
        }
    }

    void loadRides()
    {
        ifstream fin("rides.csv");
        if (!fin)
            return;//shurute file nao thakte pare tai return not throw

        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            Ride r;//ekta ride struct e sob data neya hobe
            getline(ss, r.timeStamp, ',');
            getline(ss, r.user, ',');
            getline(ss, r.driver, ',');
            getline(ss, r.from, ',');
            getline(ss, r.to, ',');
            string d1, d2;
            getline(ss, d1, ',');
            getline(ss, d2, ',');
            getline(ss, r.vehicle, ',');
            getline(ss, r.payment, ',');

            r.distance = stod(d1);
            r.fare = stod(d2);

            rides.push_back(r);
        }
    }

    void saveDrivers()
    {
        ofstream fout("drivers.csv");
        for (const auto &d : drivers)
        {
            fout << d.getUsername() << "," << d.getPassword() << "," << d.getVehicleType() << ","
                 << d.getLocation() << "," << (d.isAvailable() ? "1" : "0") << ","
                 << fixed << setprecision(2) << d.getTotalEarn() << "," << d.getTripCount() << ","
                 << fixed << setprecision(1) << d.getAvgRating() << "," << d.getRatingCount() << "\n";
        }
    }

    void saveTraffic()
    {
        ofstream fout("traffic.csv");
        for (const auto &t : trafficData)
        {
            fout << t.location << "," << t.year << "," << t.month << "," << t.day << "," << t.hour << "," << fixed << setprecision(1) << t.factor << "\n";
        }
    }

    void saveWeather()
    {
        ofstream fout("weather.csv");
        for (const auto &w : weatherData)
        {
            fout << w.location << "," << w.year << "," << w.month << "," << w.day << "," << w.hour << "," << fixed << setprecision(1) << w.factor << "\n";
        }
    }

    void appendRide(const Ride &r)
    {
        ofstream fout("rides.csv", ios::app);
        fout << r.timeStamp << "," << r.user << "," << r.driver << "," << r.from << "," << r.to
             << "," << fixed << setprecision(5) << r.distance << "," << r.fare << "," << r.vehicle
             << "," << r.payment << "\n";
    }

    void showLocations()
    {
        cout << "\nAvailable Locations:\n";
        for (size_t i = 0; i < locations.size(); ++i)
        {
            cout << "  " << i + 1 << ". " << locations[i].name << "\n";
        }
    }

    void showRideHistory(const string &name, const string &role)
    {
        system("cls");
        if (role == "User")
        {
            cout << "\n----- RIDE HISTORY -----\n";
            bool found = false;
            for (const auto &r : rides)
            {
                if (r.user == name)
                {
                    found = true;
                    cout << r.timeStamp << " | " << r.from << " -> " << r.to << " | " << r.vehicle
                         << " | " << r.fare << "\n";
                }
            }
            if (!found)
                cout << "No rides yet.\n";
        }
        else if (role == "Driver")
        {
            cout << "\n----- DRIVER RIDE HISTORY -----\n";
            bool found = false;
            for (const auto &r : rides)
            {
                if (r.driver == name)
                {
                    found = true;
                    cout << r.timeStamp << " | " << r.user << " | " << r.from << " -> " << r.to << " | "
                         << r.vehicle << " | " << r.fare << "\n";
                }
            }
            if (!found)
                cout << "No rides yet.\n";
        }
    }

    void adminListUsers()
    {
        system("cls");
        cout << "\n--- USERS LIST (username, password, wallet) ---\n";
        for (size_t i = 0; i < users.size(); ++i)
        {
            cout << "  " << i + 1 << ". " << users[i].getUsername() << ", " << users[i].getPassword() << ", " << fixed << setprecision(2) << users[i].getWallet() << "\n";
        }
        int sel = InputValidator<int>::getInput("Enter user serial to view/delete details (0 to go back): ");
        if (sel >= 1 && sel <= (int)users.size())
        {
            string uname = users[static_cast<size_t>(sel - 1)].getUsername();
            cout << "\nUser: " << uname << "\n";
            cout << "Wallet: " << users[static_cast<size_t>(sel - 1)].getWallet() << "\n";
            showRideHistory(uname, "User");

            int del = InputValidator<int>::getInput("Delete this user? (1=Yes, 0=No): ");
            if (del == 1)
            {
                users.erase(users.begin() + (sel - 1));
                saveUsers();
                cout << "User deleted successfully.\n";
            }
        }
    }

    void adminListDrivers()
    {
        system("cls");
        cout << "\n--- DRIVERS LIST (username,vehicle,location,available,totalEarn,tripCount,avgRating,ratingCount) ---\n";
        for (size_t i = 0; i < drivers.size(); ++i)
        {
            cout << "  " << i + 1 << ". " << drivers[i].getUsername() << ", " << drivers[i].getVehicleType() << ", " << drivers[i].getLocation() << ", "
                 << (drivers[i].isAvailable() ? "1" : "0") << ", " << fixed << setprecision(2) << drivers[i].getTotalEarn() << ", " << drivers[i].getTripCount() << ", "
                 << fixed << setprecision(1) << drivers[i].getAvgRating() << ", " << drivers[i].getRatingCount() << "\n";
        }
        int sel = InputValidator<int>::getInput("Enter driver serial to view/delete details (0 to go back): ");
        if (sel >= 1 && sel <= (int)drivers.size())
        {
            string dname = drivers[static_cast<size_t>(sel - 1)].getUsername();
            cout << "\nDriver: " << dname << "\n";
            cout << "Vehicle : " << drivers[static_cast<size_t>(sel - 1)].getVehicleType() << "\n";
            cout << "Location: " << drivers[static_cast<size_t>(sel - 1)].getLocation() << "\n";
            cout << "Rating  : " << fixed << setprecision(1) << drivers[static_cast<size_t>(sel - 1)].getAvgRating() << " (" << drivers[static_cast<size_t>(sel - 1)].getRatingCount() << " reviews)\n";
            cout << "Trips   : " << drivers[static_cast<size_t>(sel - 1)].getTripCount() << "\n";
            cout << "Earning : " << drivers[static_cast<size_t>(sel - 1)].getTotalEarn() << "\n";
            showRideHistory(dname, "Driver");

            int del = InputValidator<int>::getInput("Delete this driver? (1=Yes, 0=No): ");
            if (del == 1)
            {
                drivers.erase(drivers.begin() + (sel - 1));
                saveDrivers();
                cout << "Driver deleted successfully.\n";
            }
        }
    }

    void requestRide(User &u)
    {
        showLocations();

        int fromI = InputValidator<int>::getInput("Pick FROM location number: ");
        int toI = InputValidator<int>::getInput("Pick TO location number  : ");

        if (fromI < 1 || fromI > (int)locations.size() || toI < 1 || toI > (int)locations.size() ||
            fromI == toI)
        {
            cout << "Invalid location choice.\n";
            return;
        }

        cout << "\nVehicle Types:\n";
        cout << "1. Bike\n2. CNG\n3. UberX\n4. Premium\n";
        int v = InputValidator<int>::getInput("Choose vehicle: ");

        if (v < 1 || v > 4)
        {
            cout << "Invalid vehicle choice.\n";
            return;
        }

        string selectedVehicle;
        if (v == 1)
            selectedVehicle = "Bike";
        else if (v == 2)
            selectedVehicle = "CNG";
        else if (v == 3)
            selectedVehicle = "UberX";
        else if (v == 4)
            selectedVehicle = "Premium";

        if (selectedVehicle.empty())
        {
            cout << "Vehicle select failed.\n";
            return;
        }

        Location from = locations[static_cast<size_t>(fromI - 1)];
        Location to = locations[static_cast<size_t>(toI - 1)];

        if (refreshLiveDataForRequest(from))
        {
            double liveTraffic = getTrafficFactor(from.name);
            double liveWeather = getWeatherFactor(from.name);
            cout << "\nLive conditions for pickup location: " << from.name << "\n";
            cout << "Traffic factor : " << fixed << setprecision(1) << liveTraffic
                 << " (" << describeTrafficFactor(liveTraffic) << ")\n";
            cout << "Weather factor : " << fixed << setprecision(1) << liveWeather
                 << " (" << describeWeatherFactor(liveWeather) << ")\n";
        }
        else
        {
            cout << "\nWarning: Live API refresh failed. Using existing cached traffic/weather data.\n";
        }

        int dIdx = findNearestDriver(from, selectedVehicle);
        if (dIdx < 0)
        {
            cout << "No driver available for this vehicle right now.\n";
            return;
        }

        Driver &driver = drivers[static_cast<size_t>(dIdx)];
        driver.setAvailable(false);

        Vehicle *vehicle = nullptr;
        if (v == 1)
            vehicle = new Bike();
        else if (v == 2)
            vehicle = new CNG();
        else if (v == 3)
            vehicle = new UberX();
        else if (v == 4)
            vehicle = new Premium();

        if (!vehicle)
        {
            cout << "Vehicle select failed.\n";
            driver.setAvailable(true);
            return;
        }

        selectedVehicle = vehicle->name();

        int locIdx = findLocationIndex(driver.getLocation());
        double pickupDistance = 0;
        if (locIdx >= 0)
            pickupDistance = calcDistance(locations[static_cast<size_t>(locIdx)], from);

        double tripDistance = calcDistance(from, to);
        int activeUsers = (int)users.size();
        double speed = vehicle->speed();
        int availableDrivers = countAvailableDrivers(selectedVehicle);
        double fare = calculateDynamicFare(tripDistance, selectedVehicle, availableDrivers, activeUsers, from.name);
        int eta = (int)round(((pickupDistance + tripDistance) / speed) * 60.0);

        // static variable demo
        static int serial = 0;
        serial++;

        cout << "\nDriver assigned: " << driver.getUsername() << "\n";
        cout << "Vehicle       : " << selectedVehicle << "\n";
        cout << "Distance(km)  : " << fixed << setprecision(2) << tripDistance << "\n";
        cout << "ETA(min)      : " << eta << "\n";
        cout << "Fare(taka)    : " << fare << "\n";
        cout << "Ride Serial   : " << serial << "\n";

        if (!u.pay(fare))
        {
            cout << "Wallet balance not enough.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }

        saveUsers();
        cout << "Payment done from wallet. Remaining: " << u.getWallet() << "\n";

    int driverWaitTime = (int)max(1.0, round((pickupDistance / speed) * 60.0));
    int journeyTime = (int)max(1.0, round((tripDistance / speed) * 60.0));
        cout << "\nAll set! Press any key to start the ride tracking...";
        _getch();
        system("cls");
        simulateRideProgress(driverWaitTime, journeyTime);
        driver.addTrip(company.getDriverPart(fare));
        driver.setLocation(to.name);
        driver.setAvailable(true);

        company.addRideRevenue(fare);

        delete vehicle;

        Ride r;
        r.timeStamp = nowTime();
        r.user = u.getUsername();
        r.driver = driver.getUsername();
        r.from = from.name;
        r.to = to.name;
        r.distance = tripDistance;
        r.fare = fare;
        r.vehicle = selectedVehicle;
        r.payment = "Wallet";

        rides.push_back(r);
        appendRide(r);
        
        cout << "\n--- RATE YOUR RIDE ---\n";
        int rVal = InputValidator<int>::getInput("Please rate the driver (1-5): ");
        if (rVal < 1) rVal = 1;
        if (rVal > 5) rVal = 5;
        driver.addRating(rVal);
        cout << "Thank you for your rating!\n";

        // persist updated driver stats centrally
        saveDrivers();

        cout << "\nRide completed successfully. Press any key to return to menu...";
        _getch();
    }

    void userMenu(User &u)
    {
        while (true)
        {
            cout << "\n--- USER MENU (" << u.getUsername() << ") ---\n";
            cout << "1. Request Ride\n";
            cout << "2. Wallet\n";
            cout << "3. Ride History\n";
            cout << "4. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");

            if (ch == 1)
            {
                requestRide(u);
            }
            else if (ch == 2)
            {
                system("cls");
                cout << "--- WALLET ---\n";
                cout << "Current Balance: " << u.getWallet() << " taka\n";
                int add = InputValidator<int>::getInput("Add money (0 to skip): ");
                if (add > 0)
                {
                    u.addMoney(add);
                    saveUsers();
                }
            }
            else if (ch == 3)
            {
                showRideHistory(u.getUsername(), "User");
            }
            else if (ch == 4)
            {
                system("cls");
                break;
            }
            else
            {
                cout << "Invalid choice.\n";
            }
        }
    }

    void driverMenu(Driver &d)
    {
        while (true)
        {
            cout << "\n--- DRIVER MENU (" << d.getUsername() << ") ---\n";
            cout << "1. Ride History\n";
            cout << "2. Toggle Availability (Current: " << (d.isAvailable() ? "Available" : "Busy") << ")\n";
            cout << "3. Stats\n";
            cout << "4. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                showRideHistory(d.getUsername(), "Driver");
            }
            else if (ch == 2)
            {
                // toggle availability
                bool newState = !d.isAvailable();
                d.setAvailable(newState);
                cout << "Status updated: " << (d.isAvailable() ? "Available" : "Busy") << "\n";
                saveDrivers();
            }
            else if (ch == 3)
            {
                system("cls");
                cout << "--- DRIVER STATS ---\n";
                cout << "Vehicle  : " << d.getVehicleType() << "\n";
                cout << "Location : " << d.getLocation() << "\n";
                cout << "Rating   : " << fixed << setprecision(1) << d.getAvgRating() << " (" << d.getRatingCount() << " reviews)\n";
                cout << "Trips    : " << d.getTripCount() << "\n";
                cout << "Earning  : " << d.getTotalEarn() << "\n";
                cout << "\nPress any key to return to menu...";
                _getch();
            }
            else if (ch == 4)
            {
                system("cls");
                break;
            }
            else
            {
                cout << "Invalid choice.\n";
            }
        }
    }

    void adminShowSummary()
    {
        system("cls");
        cout << "\n========== ADMIN SUMMARY ==========" << "\n";
        cout << "Users         : " << users.size() << "\n";
        cout << "Drivers       : " << drivers.size() << "\n";
        cout << "Total rides   : " << rides.size() << "\n";
        cout << "Total revenue : " << company.getTotalRevenue() << "\n";
        cout << "\nPress any key to return to Admin Menu...";
        _getch();
    }

    void adminLocationEdit()
    {
        while (true)
        {
            cout << "\n--- ADMIN LOCATION MENU ---\n";
            cout << "1. View\n";
            cout << "2. Add (Manual Lat/Lon)\n";
            cout << "3. Add By Geocoding (Name -> Lat/Lon)\n";
            cout << "4. Delete Location\n";
            cout << "5. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                showLocations();
                cout << "\nPress any key to back...";
                _getch();
                system("cls");
            }
            else if (ch == 2)
            {
                string n;
                cin.ignore(1000, '\n');//get line use korle ager input er \n ta clear kore nite hoy
                cout << "Location name: ";
                getline(cin, n);

                if (n.empty())
                {
                    cout << "Invalid: Location name cannot be empty.\n";

                    continue;
                }

                double la = InputValidator<double>::getInput("Latitude : ");
                double lo = InputValidator<double>::getInput("Longitude: ");

                int dupIdx = findLocationIndexCaseInsensitive(n);
                if (dupIdx >= 0)
                {
                    cout << "A location named '" << locations[static_cast<size_t>(dupIdx)].name << "' already exists at "
                         << fixed << setprecision(6) << locations[static_cast<size_t>(dupIdx)].lat << ", "
                         << locations[static_cast<size_t>(dupIdx)].lon << "\n";
                    int keep = InputValidator<int>::getInput("Add duplicate anyway? (1=Yes, 0=No): ");
                    if (keep != 1)
                    {
                        cout << "Cancelled.\n";
                        continue;
                    }
                }

                locations.push_back(Location(n, la, lo));
                cout << "Location added successfully.\n";
                saveLocations();
                cout << "Location file saved.\n";
            }
            else if (ch == 3)
            {
                string n;
                cin.ignore(1000, '\n');
                cout << "Location name to geocode: ";
                getline(cin, n);

                if (n.empty())
                {
                    cout << "Invalid: Location name cannot be empty.\n";
                    continue;
                }

                double la = 0.0;
                double lo = 0.0;
                if (!geocodeLocationName(n, la, lo))
                {
                    cout << "Geocoding failed. Ensure internet is available and location name is valid.\n";
                    continue;
                }

                cout << "Matched coordinates: " << fixed << setprecision(6) << la << ", " << lo << "\n";

                int dupIdx = findLocationIndexCaseInsensitive(n);
                if (dupIdx >= 0)
                {
                    cout << "A location named '" << locations[static_cast<size_t>(dupIdx)].name << "' already exists at "
                         << fixed << setprecision(6) << locations[static_cast<size_t>(dupIdx)].lat << ", "
                         << locations[static_cast<size_t>(dupIdx)].lon << "\n";
                    int keep = InputValidator<int>::getInput("Add duplicate anyway? (1=Yes, 0=No): ");
                    if (keep != 1)
                    {
                        cout << "Cancelled.\n";
                        continue;
                    }
                }

                locations.push_back(Location(n, la, lo));
                saveLocations();
                cout << "Location added via geocoding and saved.\n";
            }
            else if (ch == 4)
            {
                if (locations.empty())
                {
                    cout << "No locations available to delete.\n";
                    continue;
                }

                showLocations();
                int idx = InputValidator<int>::getInput("Enter location serial to delete (0 to cancel): ");
                if (idx == 0)
                {
                    cout << "Delete cancelled.\n";
                    continue;
                }

                if (idx < 1 || idx > static_cast<int>(locations.size()))
                {
                    cout << "Invalid serial.\n";
                    continue;
                }

                const Location &target = locations[static_cast<size_t>(idx - 1)];
                for (const auto &d : drivers)
                {
                    if (d.getLocation() == target.name)
                    {
                        cout << "Cannot delete. Driver '" << d.getUsername() << "' is currently assigned to this location.\n";
                        cout << "Move/delete related drivers first, then retry.\n";
                        idx = -1;
                        break;
                    }
                }

                if (idx == -1)
                {
                    continue;
                }

                cout << "Delete location '" << target.name << "' (" << fixed << setprecision(6) << target.lat << ", " << target.lon << ") ?\n";
                int confirm = InputValidator<int>::getInput("Confirm delete (1=Yes, 0=No): ");
                if (confirm != 1)
                {
                    cout << "Delete cancelled.\n";
                    continue;
                }

                locations.erase(locations.begin() + (idx - 1));
                saveLocations();
                cout << "Location deleted and file updated.\n";
            }
            else if (ch == 5)
            {
                system("cls");
                break;
            }
        }
    }

    void adminPricingEdit()
    {
        while (true)
        {
            cout << "\n--- ADMIN PRICING EDITOR ---\n";
            cout << "1. View Config\n";
            cout << "2. Update Vehicle Base/Per-Km\n";
            cout << "3. Update Traffic/Weather Weights\n";
            cout << "4. Update Demand Tiers\n";
            cout << "5. Update Max Surge Cap\n";
            cout << "6. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                cout << "\nVehicle Pricing:\n";
                cout << "Bike    : base=" << pricing.bikeBase << ", perKm=" << pricing.bikePerKm << "\n";
                cout << "CNG     : base=" << pricing.cngBase << ", perKm=" << pricing.cngPerKm << "\n";
                cout << "UberX   : base=" << pricing.uberXBase << ", perKm=" << pricing.uberXPerKm << "\n";
                cout << "Premium : base=" << pricing.premiumBase << ", perKm=" << pricing.premiumPerKm << "\n";

                cout << "\nSurge Weights:\n";
                cout << "trafficWeight=" << pricing.trafficWeight << ", weatherWeight=" << pricing.weatherWeight << "\n";

                cout << "\nDemand Tiers:\n";
                cout << "tier1 if ratio < " << pricing.demandTier1Ratio << " => +" << pricing.demandTier1Surge << "\n";
                cout << "tier2 if ratio < " << pricing.demandTier2Ratio << " => +" << pricing.demandTier2Surge << "\n";
                cout << "tier3 if ratio < " << pricing.demandTier3Ratio << " => +" << pricing.demandTier3Surge << "\n";
                cout << "maxTotalSurge=" << pricing.maxTotalSurge << "\n";
                cout << "\nPress any key to back...";
                _getch();
                system("cls");
            }
            else if (ch == 2)
            {
                cout << "\nSelect Vehicle: 1.Bike 2.CNG 3.UberX 4.Premium\n";
                int v = InputValidator<int>::getInput("Vehicle choice: ");
                double base = InputValidator<double>::getInput("New base fare: ");
                double perKm = InputValidator<double>::getInput("New per-km fare: ");
                if (base < 0 || base > 5000 || perKm < 0 || perKm > 500)
                {
                    cout << "Invalid: base must be 0-5000 and per-km must be 0-500.\n";
                    continue;
                }

                if (v == 1) { pricing.bikeBase = base; pricing.bikePerKm = perKm; }
                else if (v == 2) { pricing.cngBase = base; pricing.cngPerKm = perKm; }
                else if (v == 3) { pricing.uberXBase = base; pricing.uberXPerKm = perKm; }
                else if (v == 4) { pricing.premiumBase = base; pricing.premiumPerKm = perKm; }
                else { cout << "Invalid vehicle choice.\n"; continue; }

                savePricingConfig();
                cout << "Vehicle pricing updated.\n";
            }
            else if (ch == 3)
            {
                double tw = InputValidator<double>::getInput("Traffic weight: ");
                double ww = InputValidator<double>::getInput("Weather weight: ");
                if (tw < 0 || tw > 2.0 || ww < 0 || ww > 2.0)
                {
                    cout << "Invalid: weights must be within 0.0 to 2.0.\n";
                    continue;
                }
                pricing.trafficWeight = tw;
                pricing.weatherWeight = ww;
                savePricingConfig();
                cout << "Surge weights updated.\n";
            }
            else if (ch == 4)
            {
                cout << "Enter tier ratio and surge values in increasing ratio order.\n";
                double r1 = InputValidator<double>::getInput("Tier1 ratio (<): ");
                double s1 = InputValidator<double>::getInput("Tier1 surge: ");
                double r2 = InputValidator<double>::getInput("Tier2 ratio (<): ");
                double s2 = InputValidator<double>::getInput("Tier2 surge: ");
                double r3 = InputValidator<double>::getInput("Tier3 ratio (<): ");
                double s3 = InputValidator<double>::getInput("Tier3 surge: ");

                bool ratiosOk = (r1 >= 0.05 && r1 <= 3.0) && (r2 >= 0.05 && r2 <= 3.0) && (r3 >= 0.05 && r3 <= 3.0) && (r1 < r2 && r2 < r3);
                bool surgeOk = (s1 >= 0.0 && s1 <= 1.0) && (s2 >= 0.0 && s2 <= 1.0) && (s3 >= 0.0 && s3 <= 1.0);
                if (!ratiosOk || !surgeOk)
                {
                    cout << "Invalid tier setup. Need 0.05<=ratio<=3.0, r1<r2<r3, and 0.0<=surge<=1.0.\n";
                    continue;
                }

                pricing.demandTier1Ratio = r1;
                pricing.demandTier1Surge = s1;
                pricing.demandTier2Ratio = r2;
                pricing.demandTier2Surge = s2;
                pricing.demandTier3Ratio = r3;
                pricing.demandTier3Surge = s3;
                savePricingConfig();
                cout << "Demand tiers updated.\n";
            }
            else if (ch == 5)
            {
                double cap = InputValidator<double>::getInput("Max total surge (additional): ");
                if (cap < 0 || cap > 3.0)
                {
                    cout << "Invalid cap. Must be between 0.0 and 3.0.\n";
                    continue;
                }
                pricing.maxTotalSurge = cap;
                savePricingConfig();
                cout << "Max surge cap updated.\n";
            }
            else if (ch == 6)
            {
                break;
            }
            else
            {
                cout << "Invalid choice.\n";
            }
        }
    }

    void adminTrafficEdit()
    {
        while (true)
        {
            cout << "\n--- ADMIN TRAFFIC EDITOR ---\n";
            cout << "1. View\n";
            cout << "2. Update\n";
            cout << "3. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                while (true)
                {
                    cout << "\nCurrent Traffic Data (location, YYYY-MM-DD, hour, factor):\n";
                    if (trafficData.empty())
                    {
                        cout << "No traffic data loaded.\n";
                    }
                    else
                    {
                        for (const auto &t : trafficData)
                        {
                            cout << t.location << ", " << t.year << "-" << setw(2) << setfill('0') << t.month << "-" << setw(2) << setfill('0') << t.day
                                 << ", " << t.hour << ":00, " << fixed << setprecision(1) << t.factor << "\n";
                        }
                        cout << setfill(' ');
                    }
                    cout << "\nPress any key to back...";
                    _getch();
                    system("cls");
                    break;
                }
            }
            else if (ch == 2)
            {
                int hour = InputValidator<int>::getInput("Hour (0-23): ");
                if (hour < 0 || hour > 23)
                {
                    cout << "Invalid hour format.\n";

                    continue;
                }

                double factor = InputValidator<double>::getInput("Traffic factor (1.0=low, 1.3=medium, 1.6=high): ");
                if (factor < 1.0)
                {
                    cout << "Invalid factor format (must be >= 1.0).\n";

                    continue;
                }

                string loc = InputValidator<string>::getInput("Location Name: ");
                time_t now = time(nullptr);
                tm *lt = localtime(&now);
                int y = lt->tm_year + 1900;
                int m = lt->tm_mon + 1;
                int d = lt->tm_mday;

                bool found = false;
                for (auto &t : trafficData)
                {
                    if (t.location == loc && t.hour == hour && t.year == y && t.month == m && t.day == d)
                    {
                        t.factor = factor;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    trafficData.push_back(TrafficData(loc, y, m, d, hour, factor));
                saveTraffic();
                cout << "Traffic data saved successfully.\n";
            }
            else if (ch == 3)
            {
                break;
            }
            else if (ch == 4)
            {
                cout << "Invalid choice.\n";
            }
        }
    }

    void adminWeatherEdit()
    {
        while (true)
        {
            cout << "\n--- ADMIN WEATHER EDITOR ---\n";
            cout << "1. View\n";
            cout << "2. Update\n";
            cout << "3. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                while (true)
                {
                    cout << "\nCurrent Weather Data (location, YYYY-MM-DD, hour, factor):\n";
                    if (weatherData.empty())
                    {
                        cout << "No weather data loaded.\n";
                    }
                    else
                    {
                        for (const auto &w : weatherData)
                        {
                            cout << w.location << ", " << w.year << "-" << setw(2) << setfill('0') << w.month << "-" << setw(2) << setfill('0') << w.day
                                 << ", " << w.hour << ":00, " << fixed << setprecision(1) << w.factor << "\n";
                        }
                        cout << setfill(' ');
                    }
                    cout << "\nPress any key to back...";
                    _getch();
                    system("cls");
                    break;
                }
            }
            else if (ch == 2)
            {
                int hour = InputValidator<int>::getInput("Hour (0-23): ");
                if (hour < 0 || hour > 23)
                {
                    cout << "Invalid hour format.\n";

                    continue;
                }

                double factor = InputValidator<double>::getInput("Weather factor (1.0=Sunny, 1.2=Cloudy, 1.5=Rainy, 1.8=Storm): ");
                if (factor < 1.0)
                {
                    cout << "Invalid factor format (must be >= 1.0).\n";

                    continue;
                }

                string loc = InputValidator<string>::getInput("Location Name: ");
                time_t now = time(nullptr);
                tm *lt = localtime(&now);
                int y = lt->tm_year + 1900;
                int m = lt->tm_mon + 1;
                int d = lt->tm_mday;

                bool found = false;
                for (auto &w : weatherData)
                {
                    if (w.location == loc && w.hour == hour && w.year == y && w.month == m && w.day == d)
                    {
                        w.factor = factor;
                        found = true;
                        break;
                    }
                }
                if (!found)
                    weatherData.push_back(WeatherData(loc, y, m, d, hour, factor));
                saveWeather();
                cout << "Weather data saved successfully.\n";
            }
            else if (ch == 3)
            {
                system("cls");
                break;
            }
            else if (ch == 4)
            {
                cout << "Invalid choice.\n";
            }
        }
    }

    void adminMenu()
    {
        while (true)
        {
            cout << "\n--- ADMIN MENU ---\n";
            cout << "1. Show Summary\n";
            cout << "2. Edit Locations\n";
            cout << "3. Edit Traffic\n";
            cout << "4. Edit Weather\n";
            cout << "5. List Users\n";
            cout << "6. List Drivers\n";
            cout << "7. Edit Pricing Logic\n";
            cout << "8. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                adminShowSummary();
            }
            else if (ch == 2)
            {
                adminLocationEdit();
            }
            else if (ch == 3)
            {
                adminTrafficEdit();
            }
            else if (ch == 4)
            {
                adminWeatherEdit();
            }
            else if (ch == 5)
            {
                adminListUsers();
            }
            else if (ch == 6)
            {
                adminListDrivers();
            }
            else if (ch == 7)
            {
                adminPricingEdit();
            }
            else if (ch == 8)
            {
                system("cls");
                break;
            }
            else
            {
                cout << "Invalid choice.\n";
            }
        }
    }

public:
    RideShareSystem()
    {
        loadLocations();
        loadPricingConfig();
        loadUsers();
        loadDrivers();
        loadRides();
        loadTraffic();
        loadWeather();
    }

    void saveAll()
    {
        saveUsers();
        saveDrivers();
        saveLocations();
        saveTraffic();
        saveWeather();
        savePricingConfig();
    }

    void showHeader()
    {

        cout << "\n===========================================\n";
        cout << "   DHAKA BASED RIDE SHARING SYSTEM \n";
        cout << "===========================================\n";
        cout << "Locations loaded: " << locations.size() << "\n";
        cout << "Users loaded    : " << users.size() << "\n";
        cout << "Drivers loaded  : " << drivers.size() << "\n";
    }

    bool loginUser(const string &u, const string &p)
    {
        for (auto &x : users)
        {
            if (x.login(u, p))
            {
                system("cls");
                cout << "Login success as User\n";
                userMenu(x);
                return true;
            }
        }
        return false;
    }

    bool loginDriver(const string &u, const string &p)
    {
        for (auto &x : drivers)
        {
            if (x.login(u, p))
            {
                system("cls");
                cout << "Login success as Driver\n";
                driverMenu(x);
                return true;
            }
        }
        return false;
    }

    bool loginAdmin(const string &u, const string &p)
    {
        if (admin.login(u, p))
        {
            system("cls");
            cout << "Login success as Admin\n";
            adminMenu();
            return true;
        }
        return false;
    }

    bool userExists(const string &uname) const
    {
        for (const auto &u : users)
        {
            if (u.getUsername() == uname)
                return true;
        }
        return false;
    }

    bool driverExists(const string &dname) const
    {
        for (const auto &d : drivers)
        {
            if (d.getUsername() == dname)
                return true;
        }
        return false;
    }

    void signupUser()
    {
        string uname, pass;
        cout << "\n--- USER SIGNUP ---\n";
        cout << "New username: ";
        cin >> uname;
        cout << "New password: ";
        cin >> pass;

        if (userExists(uname) || driverExists(uname) || uname == "admin")
        {
            cout << "Username already exists. Press any key to try again.\n";
            _getch();
            return;
        }

        users.push_back(User(uname, pass));
        saveUsers();
        cout << "User signup successful. Press any key to return to Main Menu...";
        _getch();
    }

    void signupDriver()
    {
        string dname, pass;
        cout << "\n--- DRIVER SIGNUP ---\n";
        cout << "New driver username: ";
        cin >> dname;
        cout << "New password: ";
        cin >> pass;

        if (driverExists(dname) || userExists(dname) || dname == "admin")
        {
            cout << "Username already exists. Press any key to try again.\n";
            _getch();
            return;
        }

        cout << "Vehicle Types:\n";
        cout << "1. Bike\n";
        cout << "2. CNG\n";
        cout << "3. UberX\n";
        cout << "4. Premium\n";
        int v = InputValidator<int>::getInput("Choose vehicle: ");

        Vehicle *vehicle = nullptr;
        
        if (v == 1) vehicle = new Bike();
        else if (v == 2) vehicle = new CNG();
        else if (v == 3) vehicle = new UberX();
        else if (v == 4) vehicle = new Premium();
        
        if (!vehicle)
        {
            cout << "Invalid vehicle. Signup failed.\n";
            return;
        }

        string vName = vehicle->name();
        delete vehicle;

        showLocations();
        int idx = InputValidator<int>::getInput("Choose current location number: ");
        if (idx < 1 || idx > (int)locations.size())
        {
            cout << "Invalid location. Signup failed.\n";
            return;
        }

        drivers.push_back(Driver(dname, pass, vName, locations[static_cast<size_t>(idx - 1)].name));
        saveDrivers();
        cout << "Driver signup successful. Press any key to return to Main Menu...";
        _getch();
    }
};

int main()
{
    try
    {
        RideShareSystem app;

        while (true)
        {
            system("cls");
            app.showHeader();
            cout << "\n========= MAIN MENU =========\n";
            cout << "1. User Login\n";
            cout << "2. Driver Login\n";
            cout << "3. User Signup\n";
            cout << "4. Driver Signup\n";
            cout << "5. Admin Login\n";
            cout << "6. Exit\n";

            int mainChoice = InputValidator<int>::getInput("Choice: ");

            if (mainChoice == 6)
            {
                app.saveAll();
                cout << "Goodbye!\n";
                system("cls");
                break;
                }

            if (mainChoice == 3)
            {
                app.signupUser();
                continue;
            }

            if (mainChoice == 4)
            {
                app.signupDriver();
                continue;
            }

            string u, p;
            cout << "Username: ";
            cin >> u;
            cout << "Password: ";
            cin >> p;

            if (mainChoice == 1)
            {
                if (!app.loginUser(u, p))
                {
                    cout << "Invalid user login. Press any key to try again...";
                    _getch();
                }
            }
            else if (mainChoice == 2)
            {
                if (!app.loginDriver(u, p))
                {
                    cout << "Invalid driver login. Press any key to try again...";
                    _getch();
                }
            }
            else if (mainChoice == 5)
            {
                if (!app.loginAdmin(u, p))
                {
                    cout << "Invalid admin login. Press any key to try again...";
                    _getch();
                }
            }
            else
            {
                cout << "Invalid main menu choice.\n";
            }
        }
    }
    catch (const exception &e)
    {
        cout << "Fatal error: " << e.what() << "\n";
    }

    return 0;
}