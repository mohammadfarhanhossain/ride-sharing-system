#include <algorithm>
#include <cmath>
#include <cctype>
#include <conio.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "dsa.hpp"

using namespace std;
using namespace dsa;
struct Location
{
    string name;
    double lat;
    double lon;

    Location(string n = "", double la = 0, double lo = 0)
        : name(n), lat(la), lon(lo) {}
};

double companyRevenue = 0.0;
const double COMPANY_COMMISSION = 0.20;

void addCompanyRevenue(double fare)
{
    companyRevenue += fare * COMPANY_COMMISSION;
}


double driverShare(double fare)
{
    return fare * (1.0 - COMPANY_COMMISSION);
}

class Person
{
protected:
    string username;
    string password;

public:
    Person(string u = "", string p = "") : username(u), password(p) {}
    string getUsername() const { return username; }
    string getPassword() const { return password; }
    bool login(const string &u, const string &p) const { return username == u && password == p; }
};

class User : public Person
{
    double wallet;
    int rideCount;

public:
    User(string u = "", string p = "", double w = 1000.0, int rides = 0)
        : Person(u, p), wallet(w), rideCount(rides) {}

    double getWallet() const { return wallet; }
    int getRideCount() const { return rideCount; }
    void addMoney(double amount) { wallet += amount; }
    void recordRide() { ++rideCount; }
    bool pay(double fare) { if (wallet < fare) return false; wallet -= fare; return true; }
};

class Driver : public Person
{
    string vehicleType;
    string location;
    bool available;
    int tripCount;
    double totalEarn;
    double avgRating;
    int ratingCount;

public:
    Driver(string u = "", string p = "", string v = "", string l = "")
        : Person(u, p), vehicleType(v), location(l), available(true), tripCount(0), totalEarn(0), avgRating(0), ratingCount(0) {}

    Driver(string u, string p, string v, string l, bool avail, double earn, int trips, double rating, int rc)
        : Person(u, p), vehicleType(v), location(l), available(avail), tripCount(trips), totalEarn(earn), avgRating(rating), ratingCount(rc) {}

    string getVehicleType() const { return vehicleType; }
    string getLocation() const { return location; }
    bool isAvailable() const { return available; }
    int getTripCount() const { return tripCount; }
    double getTotalEarn() const { return totalEarn; }
    double getAvgRating() const { return avgRating; }
    int getRatingCount() const { return ratingCount; }
    void setLocation(const string &l) { location = l; }
    void setAvailable(bool value) { available = value; }
    void addTrip(double earning)
    {
        ++tripCount;
        totalEarn += earning;
    }
    void addRating(int rating)
    {
        double totalPoints = (avgRating * ratingCount) + rating;
        ++ratingCount;
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
    virtual double baseFare() const = 0;
    virtual double perKm() const = 0;
    virtual double speed() const = 0;
};

class Bike : public Vehicle { public: string name() const { return "Bike"; } double baseFare() const { return 81; } double perKm() const { return 7.8; } double speed() const { return 24; } };
class CNG : public Vehicle { public: string name() const { return "CNG"; } double baseFare() const { return 125; } double perKm() const { return 28.4; } double speed() const { return 20; } };
class UberX : public Vehicle { public: string name() const { return "UberX"; } double baseFare() const { return 331; } double perKm() const { return 22.6; } double speed() const { return 27; } };
class Intercity : public Vehicle { public: string name() const { return "Intercity"; } double baseFare() const { return 127.16; } double perKm() const { return 30.70; } double speed() const { return 33; } };

void showVehicleMenu()
{
    cout << "\n1. Bike\n2. CNG\n3. UberX\n4. Intercity\n";
}

unique_ptr<Vehicle> vehicleFromChoice(int choice)
{
    if (choice == 1) return make_unique<Bike>();
    if (choice == 2) return make_unique<CNG>();
    if (choice == 3) return make_unique<UberX>();
    if (choice == 4) return make_unique<Intercity>();
    return nullptr;
}

struct Ride
{
    string timeStamp;
    string user;
    string driver;
    string from;
    string to;
    double distance = 0;
    double fare = 0;
    string vehicle;
    string payment;
    string status = "Completed";
    int requestHour = 0;
};

struct PricingConfig
{
    double trafficWeight = 0.4;
    double weatherWeight = 0.5;
    double maxTotalSurge = 1.0;
    bool dynamicPricingEnabled = true;
};

struct HourlyProfile
{
    int hour = 0;
    string condition;
    double multiplier = 1.0;
};

int getInt(const string &prompt)
{
    int value;
    cout << prompt;
    if (!(cin >> value))
    {
        cin.clear();
        cin.ignore(1000, '\n');
        return -1;
    }
    cin.ignore(1000, '\n');
    return value;
}

string getString(const string &prompt)
{
    string value;
    cout << prompt;
    getline(cin >> ws, value);
    return value;
}

class RideShareSystem
{
    struct DriverCandidate
    {
        size_t index;
        double distance;
        double rating;
        int trips;
    };

    struct DriverCandidateCompare
    {
        bool operator()(const DriverCandidate &a, const DriverCandidate &b) const
        {
            if (a.distance != b.distance) return a.distance > b.distance;
            if (a.rating != b.rating) return a.rating < b.rating;
            if (a.trips != b.trips)
                return a.trips > b.trips;
            return a.index > b.index;
        }
    };

    using RideStack = dsa::Stack<Ride>;
    using DriverHeap = dsa::PriorityQueue<DriverCandidate, DriverCandidateCompare>;

    dsa::Vector<Location> locations;
    dsa::Vector<User> users;
    dsa::Vector<Driver> drivers;
    dsa::Vector<Ride> rides;
    dsa::Vector<HourlyProfile> trafficData;
    dsa::Vector<HourlyProfile> weatherData;
    dsa::HashMap<string, size_t> userIndexByName;
    dsa::HashMap<string, size_t> driverIndexByName;
    dsa::HashMap<string, dsa::Vector<size_t>> driverBucketsByVehicle;
    dsa::BST<string, size_t> locationBST;
    dsa::BST<int, size_t> userIdBST;
    RideStack rideActivityStack;
    dsa::Vector<dsa::Vector<pair<size_t, double>>> routeGraph;
    Admin admin;
    PricingConfig pricing;

    string lowerCopy(string text) const
    {
        for (char &c : text)
            c = tolower(c);
        return text;
    }

    string normalizeKey(string text) const
    {
        text = lowerCopy(text);
        string word;
        string out;
        stringstream input(text);
        while (input >> word)
        {
            if (!out.empty()) out += ' ';
            out += word;
        }
        return out;
    }

    vector<string> splitCsv(const string &line) const
    {
        vector<string> parts;
        string item;
        stringstream ss(line);
        while (getline(ss, item, ',')) parts.push_back(item);
        return parts;
    }

    double approxDistanceKm(double lat1, double lon1, double lat2, double lon2) const
    {
        double dx = lat1 - lat2;
        double dy = lon1 - lon2;
        return sqrt(dx * dx + dy * dy) * 111.0;
    }

    int currentHour() const
    {
        time_t now = time(nullptr);
        return localtime(&now)->tm_hour;
    }

    double profileFactor(const dsa::Vector<HourlyProfile> &profiles, int hour) const
    {
        if (hour >= 0 && hour < static_cast<int>(profiles.size()))
            return profiles[hour].multiplier;
        return 1.0;
    }

    string profileCondition(const dsa::Vector<HourlyProfile> &profiles, int hour) const
    {
        if (hour >= 0 && hour < static_cast<int>(profiles.size()))
            return profiles[hour].condition;
        return "Normal";
    }

    bool isValidLocIdx(int idx) const { return idx >= 1 && idx <= static_cast<int>(locations.size()); }
    bool isValidLocPair(int from, int to) const { return isValidLocIdx(from) && isValidLocIdx(to) && from != to; }
    bool usernameTaken(const string &name) const { return userExists(name) || driverExists(name) || name == "admin"; }

    vector<size_t> reconstructPath(const vector<int> &parent, size_t goal) const
    {
        vector<size_t> path;
        for (int cur = goal; cur != -1; cur = parent[cur])
            path.push_back(cur);
        reverse(path.begin(), path.end());
        return path;
    }

    void addRouteEdge(size_t i, size_t j, double d)
    {
        auto addIfMissing = [&](size_t from, size_t to)
        {
            auto &adj = routeGraph[from];
            if (find_if(adj.begin(), adj.end(), [to](const pair<size_t, double> &p)
                        { return p.first == to; }) == adj.end())
                adj.push_back({to, d});
        };
        addIfMissing(i, j);
        addIfMissing(j, i);
    }

    void rebuildIndexes()
    {
        userIndexByName.clear();
        driverIndexByName.clear();
        driverBucketsByVehicle.clear();
        userIdBST.clear();
        for (size_t i = 0; i < users.size(); ++i)
        {
            userIndexByName[users[i].getUsername()] = i;
            userIdBST.insert(101 + i, i);
        }
        for (size_t i = 0; i < drivers.size(); ++i)
        {
            driverIndexByName[drivers[i].getUsername()] = i;
            driverBucketsByVehicle[drivers[i].getVehicleType()].push_back(i);
        }
    }

    void sortLocationsByName()
    {
        dsa::mergeSort(locations, [&](const Location &a, const Location &b)
                    {
            string ak = normalizeKey(a.name);
            string bk = normalizeKey(b.name);
            return ak == bk ? a.name < b.name : ak < bk; });
        locationBST.clear();
        for (size_t i = 0; i < locations.size(); ++i)
            locationBST.insert(normalizeKey(locations[i].name), i);
    }

    void rebuildRouteGraph()
    {
        routeGraph.assign(locations.size(), {});
        for (size_t i = 0; i < locations.size(); ++i)
        {
            dsa::Vector<pair<double, size_t>> ranked;
            for (size_t j = 0; j < locations.size(); ++j)
            {
                if (i != j)
                    ranked.push_back({calcDistance(locations[i], locations[j]), j});
            }
            dsa::mergeSort(ranked, [](const auto &a, const auto &b)
                 {
                if (a.first != b.first) return a.first < b.first;
                return a.second < b.second; });
            size_t limit = min<size_t>(4, ranked.size());
            for (size_t k = 0; k < limit; ++k)
                addRouteEdge(i, ranked[k].second, ranked[k].first);
        }
    }

    double demandSurgeForLocation(int availableDrivers, int activeUsers) const
    {
        double ratio = activeUsers > 0 ? availableDrivers / activeUsers : 0.0;
        if (ratio < 0.30)
            return 0.20;
        if (ratio < 0.60)
            return 0.15;
        if (ratio < 1.00)
            return 0.05;
        return 0.0;
    }

    double calcDistance(const Location &a, const Location &b) const
    {
        return approxDistanceKm(a.lat, a.lon, b.lat, b.lon);
    }

    int findLocationIndex(const string &name) const
    {
        const size_t *val = locationBST.find(normalizeKey(name));
        return val ? *val : -1;
    }

    dsa::Vector<size_t> bfsPath(size_t start, size_t goal) const
    {
        return dsa::bfsPath(routeGraph, locations.size(), start, goal);
    }

    bool dfsReachable(size_t current, size_t goal) const
    {
        return dsa::dfsReachable(routeGraph, locations.size(), current, goal);
    }

    dsa::Vector<size_t> dijkstraPath(size_t start, size_t goal, double &distanceKm) const
    {
        int hour = currentHour();
        double trafficMul = profileFactor(trafficData, hour);
        double weatherMul = profileFactor(weatherData, hour);
        auto path = dsa::dijkstraPath(routeGraph, locations.size(), start, goal, trafficMul, weatherMul, distanceKm);
        double totalPhysDist = 0.0;
        for (size_t k = 0; k + 1 < path.size(); ++k)
            totalPhysDist += calcDistance(locations[path[k]], locations[path[k + 1]]);
        distanceKm = totalPhysDist > 0 ? totalPhysDist : calcDistance(locations[start], locations[goal]);
        return path;
    }

    dsa::Vector<size_t> dfsAlternativePath(size_t start, size_t goal) const
    {
        return dsa::dfsPath(routeGraph, locations.size(), start, goal);
    }

    double dijkstraDistance(size_t start, size_t goal) const
    {
        double distanceKm = 0.0;
        if (dijkstraPath(start, goal, distanceKm).empty())
            return numeric_limits<double>::infinity();
        return distanceKm;
    }

    double shortestRouteDistance(const Location &from, const Location &to) const
    {
        int fromIdx = findLocationIndex(from.name);
        int toIdx = findLocationIndex(to.name);
        if (fromIdx < 0 || toIdx < 0)
            return calcDistance(from, to);
        double dist = dijkstraDistance(fromIdx, toIdx);
        return isfinite(dist) ? dist : calcDistance(from, to);
    }

    void loadLocations()
    {
        locations.clear();
        ifstream fin("location.csv");
        if (!fin)
            throw runtime_error("location.csv not found");
        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;
            vector<string> cols = splitCsv(line);
            if (cols.size() < 3)
                continue;
            try
            {
                locations.push_back(Location(cols[0], stod(cols[1]), stod(cols[2])));
            }
            catch (...)
            {
            }
        }
        sortLocationsByName();
        rebuildRouteGraph();
    }

    void saveLocations()
    {
        sortLocationsByName();
        ofstream fout("location.csv");
        for (const auto &l : locations)
        {
            fout << l.name << ',' << l.lat << ',' << l.lon << '\n';
        }
    }

    void loadUsers()
    {
        users.clear();
        ifstream fin("users.csv");
        if (!fin)
            return;
        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;
            vector<string> cols = splitCsv(line);
            if (cols.size() < 3)
                continue;
            try
            {
                double wallet = cols.size() > 2 && !cols[2].empty() ? stod(cols[2]) : 1000.0;
                int rideCount = cols.size() > 3 && !cols[3].empty() ? stoi(cols[3]) : 0;
                User u(cols[0], cols[1], wallet, rideCount);
                users.push_back(u);
            }
            catch (...)
            {
            }
        }
        rebuildIndexes();
    }

    void saveUsers()
    {
        ofstream fout("users.csv");
        for (const auto &u : users)
        {
            fout << u.getUsername() << ',' << u.getPassword() << ',' << u.getWallet() << ',';
            fout << u.getRideCount() << '\n';
        }
    }

    void loadDrivers()
    {
        drivers.clear();
        ifstream fin("drivers.csv");
        if (!fin)
            throw runtime_error("drivers.csv not found");
        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;
            vector<string> cols = splitCsv(line);
            if (cols.size() < 9)
                continue;
            try
            {
                drivers.push_back(Driver(cols[0], cols[1], cols[2], cols[3], cols[4] != "0",
                                         cols[5].empty() ? 0.0 : stod(cols[5]), cols[6].empty() ? 0 : stoi(cols[6]),
                                         cols[7].empty() ? 0.0 : stod(cols[7]), cols[8].empty() ? 0 : stoi(cols[8])));
            }
            catch (...)
            {
            }
        }
        rebuildIndexes();
    }

    void saveDrivers()
    {
        ofstream fout("drivers.csv");
        for (const auto &d : drivers)
        {
            fout << d.getUsername() << ',' << d.getPassword() << ',' << d.getVehicleType() << ',' << d.getLocation() << ','
                 << (d.isAvailable() ? '1' : '0') << ',' << d.getTotalEarn() << ','
                 << d.getTripCount() << ',' << d.getAvgRating() << ',' << d.getRatingCount() << '\n';
        }
    }

    void loadProfiles(const string &fileName, dsa::Vector<HourlyProfile> &profiles)
    {
        profiles.clear();
        ifstream fin(fileName);
        if (!fin)
            return;
        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;
            vector<string> cols = splitCsv(line);
            try
            {
                int hour = stoi(cols[0]);
                if (hour < 0 || hour > 23 || cols.size() < 3)
                    continue;
                profiles.push_back({hour, cols[1], stod(cols[2])});
            }
            catch (...)
            {
            }
        }
    }

    void saveProfiles(const string &fileName, const dsa::Vector<HourlyProfile> &profiles)
    {
        ofstream fout(fileName);
        for (const auto &p : profiles)
        {
            fout << p.hour << ',' << p.condition << ',' << p.multiplier << '\n';
        }
    }

    void loadHourlyData(dsa::Vector<HourlyProfile> &data, const string &file)
    {
        loadProfiles(file, data);
    }

    void loadTraffic() { loadHourlyData(trafficData, "traffic.csv"); }
    void loadWeather() { loadHourlyData(weatherData, "weather.csv"); }
    void saveTraffic() { saveProfiles("traffic.csv", trafficData); }
    void saveWeather() { saveProfiles("weather.csv", weatherData); }

    void loadRides()
    {
        rides.clear();
        rideActivityStack.clear();
        ifstream fin("rides.csv");
        if (!fin)
            return;
        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;
            vector<string> cols = splitCsv(line);
            if (cols.size() < 9)
                continue;
            try
            {
                Ride r;
                r.timeStamp = cols[0];
                r.user = cols[1];
                r.driver = cols[2];
                r.from = cols[3];
                r.to = cols[4];
                r.distance = stod(cols[5]);
                r.fare = stod(cols[6]);
                r.vehicle = cols[7];
                r.payment = cols[8];
                if (cols.size() > 9)
                    r.status = cols[9];
                if (cols.size() > 10 && !cols[10].empty())
                    r.requestHour = stoi(cols[10]);
                rides.push_back(r);
                rideActivityStack.push(r);
            }
            catch (...)
            {
            }
        }
    }

    void saveRides()
    {
        ofstream fout("rides.csv", ios::trunc);
        for (const auto &r : rides)
        {
            fout << r.timeStamp << ',' << r.user << ',' << r.driver << ',' << r.from << ',' << r.to << ','
                 << r.distance << ',' << r.fare << ',' << r.vehicle << ',' << r.payment << ','
                 << r.status << ',' << r.requestHour
                 << ',' << r.requestHour << '\n';
        }
    }

    string nowTime() const
    {
        time_t current = time(nullptr);
        tm *lt = localtime(&current);
        char buffer[32];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", lt);
        return buffer;
    }

    int countAvailableDrivers(const string &vehicleType) const
    {
        return static_cast<int>(count_if(drivers.begin(), drivers.end(), [&](const Driver &d)
                                         { return d.getVehicleType() == vehicleType && d.isAvailable(); }));
    }

    double computeFare(const Location &from, const Location &to, const string &vehicleType, int availableDrivers, int activeUsers, int requestHour) const
    {
        double distance = shortestRouteDistance(from, to);
        double base = 0.0;
        double perKm = 0.0;
        if (vehicleType == "Bike") { base = 81; perKm = 7.8; }
        else if (vehicleType == "CNG") { base = 125; perKm = 28.4; }
        else if (vehicleType == "UberX") { base = 331; perKm = 22.6; }
        else if (vehicleType == "Intercity") { base = 127.16; perKm = 30.70; }
        int hour = max(0, min(23, requestHour));
        double traffic = profileFactor(trafficData, hour);
        double weather = profileFactor(weatherData, hour);
        double trafficSurge = max(0.0, traffic - 1.0) * pricing.trafficWeight;
        double weatherSurge = max(0.0, weather - 1.0) * pricing.weatherWeight;
        double demandSurge = demandSurgeForLocation(availableDrivers, activeUsers);
        double total = trafficSurge + weatherSurge + demandSurge;
        if (!pricing.dynamicPricingEnabled)
            total = 0.0;
        total = min(max(total, 0.0), pricing.maxTotalSurge);
        return round((base + distance * perKm) * (1.0 + total) * 100.0) / 100.0;
    }

    void printShortestPath(const dsa::Vector<size_t> &path) const
    {
        cout << "Shortest Path: ";
        for (size_t i = 0; i < path.size(); ++i)
        {
            cout << locations[path[i]].name;
            if (i + 1 < path.size()) cout << " -> ";
        }
        cout << '\n';
    }

    void showRouteSummary(const string &fromName, const string &toName) const
    {
        int fromIdx = findLocationIndex(fromName), toIdx = findLocationIndex(toName);
        if (fromIdx < 0 || toIdx < 0) { cout << "Route summary unavailable.\n"; return; }
        double distance = 0.0;
        dsa::Vector<size_t> path = dijkstraPath(fromIdx, toIdx, distance);
        if (path.empty()) distance = calcDistance(locations[fromIdx], locations[toIdx]);
        cout << "\n--- ROUTE SUMMARY ---\nFrom: " << locations[fromIdx].name
             << "\nTo  : " << locations[toIdx].name << "\nDijkstra km  : " << distance << '\n';
        printShortestPath(path);
        dsa::Vector<size_t> alternative = dfsAlternativePath(fromIdx, toIdx);
        if (!alternative.empty() && alternative.size() != path.size())
        {
            cout << "Alternative Route (DFS): ";
            printShortestPath(alternative);
        }
    }

    bool pickLocationPair(int &fromI, int &toI, const string &fromPrompt, const string &toPrompt) const
    {
        showLocations();
        fromI = getInt(fromPrompt);
        toI = getInt(toPrompt);
        if (!isValidLocPair(fromI, toI))
        {
            cout << "Invalid location choice.\n";
            return false;
        }
        return true;
    }

    void interactiveRouteSummary() const
    {
        int fromI = 0, toI = 0;
        if (!pickLocationPair(fromI, toI, "Pick START location number: ", "Pick END location number: "))
            return;
        showRouteSummary(locations[fromI - 1].name, locations[toI - 1].name);
        
    }

    void showRideHistory(const string &name, bool forUser)
    {
        loadRides();
        system("cls");
        cout << (forUser ? "\n----- YOUR TRIPS -----\n" : "\n----- DRIVER RIDE HISTORY -----\n");
        bool found = false;
        for (const auto &r : rides)
        {
            bool match = forUser ? (r.user == name) : (r.driver == name && r.status == "Completed");
            if (!match)
                continue;
            found = true;
            cout << r.timeStamp << " | ";
            if (!forUser)
                cout << r.user << " | ";
            cout << r.from << " -> " << r.to << " | " << r.vehicle << " | " << r.fare;
            if (forUser)
                cout << " | " << r.status;
            cout << '\n';
        }
        if (!found)
            cout << (forUser ? "No trips yet.\n" : "No rides yet.\n");
    }

    vector<size_t> rankDriversForRequest(const Location &from, const string &vehicleType)
    {
        dsa::Vector<size_t> *bucket = driverBucketsByVehicle.find(vehicleType);
        if (!bucket)
            return {};
        int fromIndex = findLocationIndex(from.name);
        dsa::Vector<bool> visited(locations.size(), false);
        dsa::Vector<int> level(locations.size(), -1);
        dsa::LocationQueue nearby;
        if (fromIndex >= 0) { nearby.push(fromIndex); visited[fromIndex] = true; level[fromIndex] = 0; }
        while (!nearby.empty())
        {
            size_t node = nearby.front(); nearby.pop();
            if (level[node] == 2) continue;
            for (const auto &edge : routeGraph[node])
                if (!visited[edge.first]) { visited[edge.first] = true; level[edge.first] = level[node] + 1; nearby.push(edge.first); }
        }
        DriverHeap pq;
        for (size_t idx : *bucket)
        {
            if (!drivers[idx].isAvailable())
                continue;
            int locIdx = findLocationIndex(drivers[idx].getLocation());
            if (locIdx < 0 || !visited[locIdx]) continue;
            double distance = locIdx >= 0 ? shortestRouteDistance(locations[locIdx], from) : numeric_limits<double>::infinity();
            pq.push({idx, distance, drivers[idx].getAvgRating(), drivers[idx].getTripCount()});
        }
        vector<size_t> ordered;
        while (!pq.empty())
        {
            ordered.push_back(pq.top().index);
            pq.pop();
        }
        return ordered;
    }

    void requestRide(User &u)
    {
        int fromI = 0, toI = 0;
        if (!pickLocationPair(fromI, toI, "Pick FROM location number: ", "Pick TO location number: "))
            return;

        showVehicleMenu();
        int vehicleChoice = getInt("Choose vehicle: ");
        unique_ptr<Vehicle> vehicle = vehicleFromChoice(vehicleChoice);
        if (!vehicle)
        {
            cout << "Invalid vehicle choice.\n";
            return;
        }

        Location from = locations[fromI - 1];
        Location to = locations[toI - 1];
        

        vector<size_t> orderedDrivers = rankDriversForRequest(from, vehicle->name());
        if (orderedDrivers.empty())
        {
            cout << "No driver available for this vehicle right now.\n";
            return;
        }

        int requestHour = currentHour();
        double tripDistance = 0.0;
        dsa::Vector<size_t> tripPath = dijkstraPath(fromI - 1, toI - 1, tripDistance);
        if (tripPath.empty()) tripDistance = calcDistance(from, to);
        int availableDrivers = countAvailableDrivers(vehicle->name());
        int activeUsers = static_cast<int>(users.size());
        double fare = computeFare(from, to, vehicle->name(), availableDrivers, activeUsers, requestHour);

        double vehicleSpeed = vehicle->speed();
        double trafficMul = profileFactor(trafficData, requestHour);
        double weatherMul = profileFactor(weatherData, requestHour);
        double estTimeMins = (tripDistance / vehicleSpeed) * 60.0 * trafficMul * weatherMul;

        cout << "\n[Route analysis]\nFrom: " << from.name << "\nTo  : " << to.name
             << "\nDijkstra km  : " << tripDistance << '\n';
        printShortestPath(tripPath);
        dsa::Vector<size_t> alternativePath = dfsAlternativePath(fromI - 1, toI - 1);
        if (!alternativePath.empty() && alternativePath.size() != tripPath.size())
        {
            cout << "Alternative Route (DFS): ";
            printShortestPath(alternativePath);
        }
        cout << "Vehicle: " << vehicle->name() << '\n';
        cout << "Trip   : " << tripDistance << " km\n";
        cout << "Est. Time: " << estTimeMins << " mins\n";
        cout << "Time   : " << requestHour << ":00\n";
        cout << "Traffic: " << profileCondition(trafficData, requestHour) << " x" << profileFactor(trafficData, requestHour) << '\n';
        cout << "Weather: " << profileCondition(weatherData, requestHour) << " x" << profileFactor(weatherData, requestHour) << '\n';
        cout << "Fare   : " << fare << " taka\n";

        cout << "\nPayment: Wallet\n";

        Ride r;
        r.timeStamp = nowTime();
        r.user = u.getUsername();
        r.from = from.name;
        r.to = to.name;
        r.distance = tripDistance;
        r.fare = fare;
        r.vehicle = vehicle->name();
        r.payment = "Wallet";
        r.status = "Started";
        r.requestHour = requestHour;
        rides.push_back(r);
        saveRides();
        rideActivityStack.push(r);

        cout << "\nRide request placed! You have 5 seconds to cancel (Press 'c' to cancel):\n";
        bool canceled = false;
        for (int sec = 5; sec >= 1; --sec)
        {
            cout << "\r[Cancel Window] Time remaining: " << sec << "s (Press 'c' to cancel)... " << flush;
            int elapsedMs = 0;
            while (elapsedMs < 1000)
            {
                if (_kbhit())
                {
                    char ch = _getch();
                    if (ch == 'c' || ch == 'C')
                    {
                        canceled = true;
                        break;
                    }
                }
                Sleep(50);
                elapsedMs += 50;
            }
            if (canceled)
                break;
        }
        if (canceled)
        {
            rides.back().status = "Canceled";
            saveRides();
            cout << "\n\nRide request canceled by user.\n";
        }
        else
        {
            cout << "\n\nBest available driver assigned automatically. Starting ride...\n";
            size_t rideIndex = rides.size() - 1;
            Driver &bestDriver = drivers[orderedDrivers[0]];
            if (!completeRide(rideIndex, bestDriver))
                cout << "Ride could not be started because wallet payment failed.\n";
        }
        
    }

    bool completeRide(size_t rideIndex, Driver &driver)
    {
        if (rideIndex >= rides.size())
            return false;
        Ride &ride = rides[rideIndex];
        if (ride.status != "Started") return false;

        loadUsers();
        size_t *userIndex = userIndexByName.find(ride.user);
        if (!userIndex)
        {
            ride.status = "Canceled";
            saveRides();
            return false;
        }

        User &user = users[*userIndex];
        double fare = ride.fare;
        
        bool paymentOk = true;
        if (ride.payment == "Wallet")
            paymentOk = user.pay(fare);
        else if (ride.payment == "Card")
            paymentOk = user.pay(fare);
        if (!paymentOk)
        {
            saveUsers();
            saveRides();
            return false;
        }

        user.recordRide();
        saveUsers();

        string driverStartLocation = driver.getLocation();
        driver.addTrip(driverShare(fare));
        driver.setAvailable(true);
        driver.addRating(4);
        addCompanyRevenue(fare);

        ride.driver = driver.getUsername();
        ride.fare = fare;
        ride.status = "Completed";
        saveDrivers();
        saveRides();

        int driverLocIdx = findLocationIndex(driverStartLocation);
        int fromIdx = findLocationIndex(ride.from);
        int toIdx = findLocationIndex(ride.to);
        double speed = 25.0;
        if (ride.vehicle == "Bike") speed = 24;
        else if (ride.vehicle == "CNG") speed = 20;
        else if (ride.vehicle == "UberX") speed = 27;
        else if (ride.vehicle == "Intercity") speed = 33;
        double trafficMul = profileFactor(trafficData, ride.requestHour);
        double weatherMul = profileFactor(weatherData, ride.requestHour);

        cout << "\n================ RIDE DETAILS & ETA ================\n";
        if (driverLocIdx >= 0 && fromIdx >= 0)
        {
            if (!dfsReachable(driverLocIdx, fromIdx))
            {
                cout << "Pickup route is unreachable. Ride canceled.\n";
                return false;
            }
            double pickupKm = 0.0;
            dsa::Vector<size_t> pickupPath = dijkstraPath(driverLocIdx, fromIdx, pickupKm);
            double pickupMins = (pickupKm / speed) * 60.0 * trafficMul * weatherMul;
            cout << "Driver Starting Node: " << locations[driverLocIdx].name << '\n';
            cout << "Driver Pickup ETA   : " << pickupMins << " mins (" << pickupKm << " km)\n";
        }

        double estMins = (ride.distance / speed) * 60.0 * trafficMul * weatherMul;
        cout << "Trip Distance       : " << ride.distance << " km\n";
        cout << "Trip ETA            : " << estMins << " mins\n";

        auto simulatePath = [&](const dsa::Vector<size_t> &path, const string &phase, double minutes)
        {
            if (path.empty())
                return;
            int remaining = max(1, static_cast<int>(ceil(minutes)));
            cout << phase << " route: ";
            for (size_t k = 0; k < path.size(); ++k)
                cout << locations[path[k]].name << (k + 1 < path.size() ? " -> " : "\n");
            for (size_t k = 0; k < path.size(); ++k)
            {
                int step = k;
                int eta = remaining - step;
                if (eta < 0) eta = 0;
                cout << "\r[" << phase << "] Current location: " << locations[path[k]].name
                     << " | ETA: " << eta << " seconds        " << flush;
                if (k + 1 < path.size())
                    Sleep(1000);
            }
            cout << '\n';
        };

        if (driverLocIdx >= 0 && fromIdx >= 0)
        {
            double pickupKm = 0.0;
            dsa::Vector<size_t> pickupPath = dijkstraPath(driverLocIdx, fromIdx, pickupKm);
            if (pickupPath.empty())
            {
                pickupPath = dsa::dfsPath(routeGraph, locations.size(), driverLocIdx, fromIdx);
                pickupKm = calcDistance(locations[driverLocIdx], locations[fromIdx]);
                cout << "Dijkstra pickup route unavailable; using DFS fallback.\n";
            }
            double pickupMins = (pickupKm / speed) * 60.0 * trafficMul * weatherMul;
            simulatePath(pickupPath, "Pickup", pickupMins);
        }
        if (fromIdx >= 0 && toIdx >= 0)
        {
            double tripKm = 0.0;
            dsa::Vector<size_t> tripPath = dijkstraPath(fromIdx, toIdx, tripKm);
            if (tripPath.empty())
            {
                tripPath = dsa::dfsPath(routeGraph, locations.size(), fromIdx, toIdx);
                cout << "Dijkstra trip route unavailable; using DFS fallback.\n";
            }
            simulatePath(tripPath, "Trip", estMins);
        }
        driver.setLocation(ride.to);
        cout << "\nTrip reached destination successfully!\n";
        cout << "====================================================\n";

        return true;
    }

    void showLocations() const
    {
        cout << "\nAvailable Locations:\n";
        for (size_t i = 0; i < locations.size(); ++i)
            cout << "  " << i + 1 << ". " << locations[i].name << '\n';
    }

    void showDriverLeaderboard() const
    {
        struct LeaderEntry
        {
            size_t idx;
            double rating;
            double earnings;
            int trips;
        };
        struct LeaderCompare
        {
            bool operator()(const LeaderEntry &a, const LeaderEntry &b) const
            {
                if (a.rating != b.rating)
                    return a.rating < b.rating;
                if (a.earnings != b.earnings)
                    return a.earnings < b.earnings;
                return a.trips < b.trips;
            }
        };
        dsa::Vector<LeaderEntry> ranked;
        for (size_t i = 0; i < drivers.size(); ++i)
            ranked.push_back({i, drivers[i].getAvgRating(), drivers[i].getTotalEarn(), drivers[i].getTripCount()});
        dsa::mergeSort(ranked, LeaderCompare());

        cout << "\n=================== DRIVER LEADERBOARD ===================\n";
        cout << "Rank" << "Driver" << "Vehicle" << "Rating" << "Trips" << "Earnings\n";
        cout << "----------------------------------------------------------\n";
        int rank = 1;
        while (rank <= 5 && rank <= static_cast<int>(ranked.size()))
        {
            LeaderEntry e = ranked[rank - 1];
            const Driver &d = drivers[e.idx];
            cout << rank++
                 << d.getUsername()
                 << d.getVehicleType()
                 << d.getAvgRating()
                 << d.getTripCount()
                 << d.getTotalEarn() << " taka\n";
        }
        cout << "==========================================================\n";
    }

    void adminSearchUserByID() const
    {
        cout << "\n--- BST USER SEARCH BY ID ---\n";
        int searchId = getInt("Enter User ID (e.g., 101, 102...): ");
        const size_t *val = userIdBST.find(searchId);
        if (!val)
        {
            cout << "User ID " << searchId << " not found in BST.\n";
            return;
        }
        const User &u = users[*val];
        cout << "\n[User Record Found via BST]\n";
        cout << "User ID       : " << searchId << '\n';
        cout << "Username      : " << u.getUsername() << '\n';
        cout << "Wallet Balance: " << u.getWallet() << " taka\n";
        cout << "Ride Count    : " << u.getRideCount() << '\n';
    }

    void adminDashboard() const
    {
        cout << "\n========================= SYSTEM DASHBOARD =========================\n";
        cout << " Total Passengers: " << users.size()
             << " Total Drivers: " << drivers.size() << '\n';
        cout << " Total Rides: " << rides.size()
             << " Company Revenue: " << companyRevenue << " taka\n";
        cout << "====================================================================\n";
    }

    void userMenu(User &u)
    {
        while (true)
        {
            cout << "\n--- USER MENU (" << u.getUsername() << ") ---\n";
            cout << "1. Request a Ride\n2. Your trips\n3. Route Summary\n4. Driver Leaderboard\n5. Logout\n";
            int choice = getInt("Choice: ");
            if (choice == 1)
                requestRide(u);
            else if (choice == 2)
                showRideHistory(u.getUsername(), true);
            else if (choice == 3)
                interactiveRouteSummary();
            else if (choice == 4)
                showDriverLeaderboard();
            else if (choice == 5)
                break;
        }
    }

    void driverMenu(Driver &d)
    {
        while (true)
        {
            cout << "\n--- DRIVER MENU (" << d.getUsername() << ") ---\n";
            cout << "1. View Ride History\n2. Toggle Availability\n3. View Stats\n4. Route Summary\n5. Leaderboard\n6. Logout\n";
            int choice = getInt("Choice: ");
            if (choice == 1)
                showRideHistory(d.getUsername(), false);
            else if (choice == 2)
            {
                d.setAvailable(!d.isAvailable());
                saveDrivers();
            }
            else if (choice == 3)
            {
                cout << "Vehicle: " << d.getVehicleType() << '\n'
                     << "Location: " << d.getLocation() << '\n'
                     << "Rating: " << d.getAvgRating() << '\n'
                     << "Trips: " << d.getTripCount() << '\n'
                     << "Earning: " << d.getTotalEarn() << '\n';
            }
            else if (choice == 4)
                interactiveRouteSummary();
            else if (choice == 5)
                showDriverLeaderboard();
            else if (choice == 6)
                break;
        }
    }

public:
    bool userExists(const string &name) const { return userIndexByName.contains(name); }
    bool driverExists(const string &name) const { return driverIndexByName.contains(name); }

    bool loginUser(const string &u, const string &p)
    {
        size_t *index = userIndexByName.find(u);
        if (!index || !users[*index].login(u, p)) return false;
        userMenu(users[*index]);
        return true;
    }

    bool loginDriver(const string &u, const string &p)
    {
        size_t *index = driverIndexByName.find(u);
        if (!index || !drivers[*index].login(u, p)) return false;
        driverMenu(drivers[*index]);
        return true;
    }

    bool loginAdmin(const string &u, const string &p)
    {
        if (admin.login(u, p))
        {
            adminMenu();
            return true;
        }
        return false;
    }

    void signupUser()
    {
        string u = getString("New username: ");
        string p = getString("New password: ");
        if (usernameTaken(u))
        {
            cout << "Username already exists.\n";
            return;
        }
        users.push_back(User(u, p));
        rebuildIndexes();
        saveUsers();
    }

    void signupDriver()
    {
        string u = getString("New driver username: ");
        string p = getString("New password: ");
        if (usernameTaken(u))
        {
            cout << "Username already exists.\n";
            return;
        }

        showVehicleMenu();
        int choice = getInt("Vehicle: ");
        unique_ptr<Vehicle> vehicle = vehicleFromChoice(choice);
        if (!vehicle)
            return;

        showLocations();
        int idx = getInt("Choose current location number: ");
        if (!isValidLocIdx(idx))
            return;
        drivers.push_back(Driver(u, p, vehicle->name(), locations[idx - 1].name));
        rebuildIndexes();
        saveDrivers();
    }

    void adminShowSummary() const
    {
        adminDashboard();

    }

    void adminListUsers() const
    {
        for (size_t i = 0; i < users.size(); ++i)
            cout << i + 1 << ". " << users[i].getUsername() << " | rides: " << users[i].getRideCount() << '\n';
    }

    void adminListDrivers() const
    {
        for (size_t i = 0; i < drivers.size(); ++i)
            cout << i + 1 << ". " << drivers[i].getUsername() << " | " << drivers[i].getVehicleType() << " | " << drivers[i].getLocation() << " | rating: " << drivers[i].getAvgRating() << '\n';
    }

    void adminRecentRides() const
    {
        dsa::Vector<Ride> recent = rideActivityStack.newestFirst();
        cout << "\n--- RECENT RIDES (STACK: newest first) ---\n";
        for (size_t i = 0; i < recent.size(); ++i)
            cout << i + 1 << ". " << recent[i].timeStamp << " | User: " << recent[i].user
                 << " | " << recent[i].from << " -> " << recent[i].to
                 << " | Driver: " << recent[i].driver << '\n';
        if (recent.empty()) cout << "No rides yet.\n";
    }

public:
    RideShareSystem()
    {
        loadLocations();
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
        saveRides();
    }

    void adminMenu()
    {
        while (true)
        {
            cout << "\n--- ADMIN MENU ---\n1. Dashboard & Summary\n2. Users List\n3. Drivers List\n4. Search User by ID (BST)\n5. Recent Rides (Stack)\n6. Driver Leaderboard\n7. Route Summary\n8. Back\n";
            int choice = getInt("Choice: ");
            if (choice == 1)
                adminShowSummary();
            else if (choice == 2)
                adminListUsers();
            else if (choice == 3)
                adminListDrivers();
            else if (choice == 4)
                adminSearchUserByID();
            else if (choice == 5)
                adminRecentRides();
            else if (choice == 6)
                showDriverLeaderboard();
            else if (choice == 7)
                interactiveRouteSummary();
            else if (choice == 8)
                break;
        }
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
            cout << "DHAKA BASED RIDE SHARING SYSTEM\n";
            cout << "1. User Login\n2. Driver Login\n3. User Signup\n4. Driver Signup\n5. Admin Login\n6. Exit\n";
            int choice = getInt("Choice: ");
            switch (choice)
            {
            case 1:
            case 2:
            case 5:
            {
                string u = getString("Username: ");
                string p = getString("Password: ");
                bool ok = choice == 1 ? app.loginUser(u, p) : (choice == 2 ? app.loginDriver(u, p) : app.loginAdmin(u, p));
                if (!ok) cout << "Invalid login.\n";
                break;
            }
            case 3: app.signupUser(); break;
            case 4: app.signupDriver(); break;
            case 6: app.saveAll(); return 0;
            default: cout << "Invalid choice.\n";
            }
        }
    }
    catch (const exception &e)
    {
        cout << "Fatal error: " << e.what() << '\n';
    }
    return 0;
}

