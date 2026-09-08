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
#include <functional>
#include <memory>
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
    double getCommissionRate() const { return commissionRate; }
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
    Money cardBalance;
    vector<string> favoriteLocations;

public:
    User(string u, string p, Money w = Money(1000), Money card = Money(1500))
        : Person(u, p), wallet(w), cardBalance(card) {}


    Money getWallet() const { return wallet; }
    Money getCardBalance() const { return cardBalance; }

    vector<string> getFavoriteLocations() const { return favoriteLocations; }
    
    void addFavorite(const string &locName) {
        for (const auto &f : favoriteLocations) {
            if (f == locName) return;
        }
        if (favoriteLocations.size() < 5) {
            favoriteLocations.push_back(locName);
        }
    }
    
    void removeFavorite(const string &locName) {
        favoriteLocations.erase(
            remove(favoriteLocations.begin(), favoriteLocations.end(), locName),
            favoriteLocations.end()
        );
    }
    
    void setFavorites(const vector<string> &favs) { favoriteLocations = favs; }

    // function overloading
    void addMoney(int x) { wallet = wallet + Money((double)x); }
    void addMoney(double x) { wallet = wallet + Money(x); }
    void addCardMoney(double x) { cardBalance = cardBalance + Money(x); }

    bool pay(double fare)
    {
        if (wallet.get() < fare)
            return false;
        wallet = wallet - Money(fare);
        return true;
    }

    bool payCard(double fare)
    {
        if (cardBalance.get() < fare)
            return false;
        cardBalance = cardBalance - Money(fare);
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

class Intercity : public Vehicle
{
public:
    string name() const override { return "Intercity"; }
    double factor() const override { return 30.7; }
    double speed() const override { return 33; }
};

enum class RideStatus
{
    Requesting,
    Matched,
    DriverEnRoute,
    OnTrip,
    Completed,
    Cancelled
};

inline string rideStatusLabel(RideStatus s)
{
    switch (s)
    {
    case RideStatus::Requesting:
        return "Requesting";
    case RideStatus::Matched:
        return "Matched";
    case RideStatus::DriverEnRoute:
        return "DriverEnRoute";
    case RideStatus::OnTrip:
        return "OnTrip";
    case RideStatus::Completed:
        return "Completed";
    case RideStatus::Cancelled:
        return "Cancelled";
    default:
        return "Unknown";
    }
}

struct TripQuote
{
    double baseCharge = 0;
    double perKmRate = 0;
    double distance = 0;
    double baseFare = 0;
    double trafficFactor = 1;
    double weatherFactor = 1;
    double trafficSurge = 0;
    double weatherSurge = 0;
    double demandSurge = 0;
    double totalSurge = 0;
    double supplyRatio = 0;
    double finalFare = 0;
    double promoSavings = 0;
    string promoCode;
};

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
    double intercityBase = 127.1599332;
    double intercityPerKm = 30.7022556;

    double trafficWeight = 0.4;
    double weatherWeight = 0.5;

    double demandTier1Ratio = 0.3;
    double demandTier1Surge = 0.20;
    double demandTier2Ratio = 0.6;
    double demandTier2Surge = 0.15;
    double demandTier3Ratio = 1.0;
    double demandTier3Surge = 0.05;

    double maxTotalSurge = 1.0;
    bool dynamicPricingEnabled = true;
};

class IFareCalculator
{
public:
    virtual ~IFareCalculator() = default;
    virtual TripQuote compute(double distance, const string &vehicleType, int availableDrivers,
                              int activeUsers, const string &locName) = 0;
};

class DynamicFareCalculator : public IFareCalculator
{
    PricingConfig *pricing;
    function<double(const string &)> trafficFn;
    function<double(const string &)> weatherFn;
    function<double(const string &)> baseFn;
    function<double(const string &)> perKmFn;

public:
    DynamicFareCalculator(PricingConfig *p, function<double(const string &)> tf,
                          function<double(const string &)> wf, function<double(const string &)> bf,
                          function<double(const string &)> pf)
        : pricing(p), trafficFn(move(tf)), weatherFn(move(wf)), baseFn(move(bf)), perKmFn(move(pf))
    {
    }

    TripQuote compute(double distance, const string &vehicleType, int availableDrivers,
                      int activeUsers, const string &locName) override
    {
        TripQuote q;
        q.distance = distance;
        q.baseCharge = baseFn(vehicleType);
        q.perKmRate = perKmFn(vehicleType);
        q.baseFare = q.baseCharge + (distance * q.perKmRate);
        q.trafficFactor = trafficFn(locName);
        q.weatherFactor = weatherFn(locName);
        q.trafficSurge = (q.trafficFactor - 1.0) * pricing->trafficWeight;
        q.weatherSurge = (q.weatherFactor - 1.0) * pricing->weatherWeight;

        q.demandSurge = 0.0;
        if (activeUsers > 0)
        {
            q.supplyRatio = (double)availableDrivers / (double)activeUsers;
            if (q.supplyRatio < pricing->demandTier1Ratio)
                q.demandSurge = pricing->demandTier1Surge;
            else if (q.supplyRatio < pricing->demandTier2Ratio)
                q.demandSurge = pricing->demandTier2Surge;
            else if (q.supplyRatio < pricing->demandTier3Ratio)
                q.demandSurge = pricing->demandTier3Surge;
        }

        q.totalSurge = q.trafficSurge + q.weatherSurge + q.demandSurge;
        if (q.totalSurge > pricing->maxTotalSurge)
            q.totalSurge = pricing->maxTotalSurge;
        if (q.totalSurge < 0.0)
            q.totalSurge = 0.0;
        if (!pricing->dynamicPricingEnabled)
            q.totalSurge = 0.0;

        q.finalFare = round(q.baseFare * (1.0 + q.totalSurge) * 100.0) / 100.0;
        return q;
    }
};

class PromoFareDecorator : public IFareCalculator
{
    unique_ptr<IFareCalculator> inner;
    double discountFraction;
    string codeLabel;

public:
    PromoFareDecorator(unique_ptr<IFareCalculator> in, double discountFrac, const string &code)
        : inner(move(in)), discountFraction(discountFrac), codeLabel(code)
    {
    }

    TripQuote compute(double distance, const string &vehicleType, int availableDrivers,
                      int activeUsers, const string &locName) override
    {
        TripQuote q = inner->compute(distance, vehicleType, availableDrivers, activeUsers, locName);
        if (discountFraction > 0.0 && !codeLabel.empty())
        {
            q.promoSavings = round(q.finalFare * discountFraction * 100.0) / 100.0;
            q.finalFare = round((q.finalFare - q.promoSavings) * 100.0) / 100.0;
            q.promoCode = codeLabel;
        }
        return q;
    }
};

class CancellationPolicy
{
    double feeAfterConfirm;

public:
    explicit CancellationPolicy(double fee = 75.0) : feeAfterConfirm(fee) {}
    double cancellationFee(bool afterConfirm) const { return afterConfirm ? feeAfterConfirm : 0.0; }
};

class IPaymentMethod
{
public:
    virtual ~IPaymentMethod() = default;
    virtual string label() const = 0;
    virtual bool charge(User &user, double amount) = 0;
};

class WalletPaymentMethod : public IPaymentMethod
{
public:
    string label() const override { return "Wallet"; }
    bool charge(User &u, double amount) override { return u.pay(amount); }
};

class CashPaymentMethod : public IPaymentMethod
{
public:
    string label() const override { return "Cash"; }
    bool charge(User &, double) override { return true; }
};

class CardPaymentMethod : public IPaymentMethod
{
public:
    string label() const override { return "Card"; }
    bool charge(User &u, double amount) override { return u.payCard(amount); }
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
    string status;
    double estimateFare;
    double surgeFraction;
    double promoDiscount;
    string promoCode;

    Ride()
        : distance(0), fare(0), payment("Wallet"), status("Completed"), estimateFare(0), surgeFraction(0),
          promoDiscount(0)
    {
    }
};

class ReceiptFormatter
{
public:
    static void printTripReceipt(const TripQuote &est, const Ride &trip, const Company &co,
                                 const string &paymentLabel)
    {
        cout << "\n========== TRIP RECEIPT ==========\n";
        cout << fixed << setprecision(2);
        cout << "Trip time      : " << trip.timeStamp << "\n";
        cout << "Route          : " << trip.from << " -> " << trip.to << "\n";
        cout << "Distance       : " << trip.distance << " km\n";
        cout << "Vehicle        : " << trip.vehicle << "\n";
        cout << "Trip status    : " << trip.status << "\n";
        cout << "Payment        : " << paymentLabel << "\n\n";

        cout << "Fare breakdown:\n";
        cout << "  Upfront estimate : " << est.finalFare + est.promoSavings << " taka\n";
        if (est.promoSavings > 0.0)
            cout << "  Promo (" << est.promoCode << ")   : -" << est.promoSavings << " taka\n";
        cout << "  Subtotal (quote): " << est.finalFare << " taka\n";
        cout << "  Surge total      : +" << (est.totalSurge * 100.0) << "%\n";
        double platFee = trip.fare * co.getCommissionRate();
        cout << "  Platform fee     : " << platFee << " taka\n";
        cout << "  Driver payout    : " << co.getDriverPart(trip.fare) << " taka\n";
        cout << "  Amount charged   : " << trip.fare << " taka\n";
        cout << "==================================\n";
    }
};

template <typename T>
class InputValidator {
public:
    static T getInput(const string &prompt) {
        T x;
        while (true) {
            cout << prompt;
            if (cin >> x) {
                cin.ignore(1000, '\n');
                return x;
            }
            cin.clear();
            cin.ignore(1000, '\n');
            cout << "Invalid input. Try again.\n";
        }
    }
};

// Specialization for string to handle spaces
template <>
class InputValidator<string> {
public:
    static string getInput(const string &prompt) {
        string x;
        while (true) {
            cout << prompt;
            // Skip leading whitespace (like newlines from previous cin >> calls)
            getline(cin >> ws, x);
            if (!x.empty())
                return x;
            cout << "Input cannot be empty. Try again.\n";
        }
    }
};

class RideShareSystem
{
private:
    struct GeocodeCandidate
    {
        string display;
        double lat;
        double lon;

        GeocodeCandidate(const string &d, double la, double lo)
            : display(d), lat(la), lon(lo) {}
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
    CancellationPolicy cancelPolicy;

    // Simulation and Map utilities
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
        cout << "    LIVE TRIP (Uber-style phases)    \n";
        cout << "====================================\n\n";

        cout << "[Phase: " << rideStatusLabel(RideStatus::DriverEnRoute) << "] Driver en route...\n";
        for (int t = driverWaitTime; t > 0; t--)
        {
            cout << "\r[ETA pickup] " << t << " mins   " << flush;
            Sleep(1000);
        }
        cout << "\r[Phase: " << rideStatusLabel(RideStatus::OnTrip) << "] Driver arrived — trip starting.        \n\n";

        cout << "[Phase: " << rideStatusLabel(RideStatus::OnTrip) << "] En route to destination...\n";
        for (int t = journeyTime; t > 0; t--)
        {
            cout << "\r[ETA dropoff] " << t << " mins remaining...  " << flush;
            Sleep(1000);
        }

        cout << "\r[Phase: " << rideStatusLabel(RideStatus::Completed) << "] Trip completed.\n";
        cout << "====================================\n\n";
    }

    double getVehicleBase(const string &vehicleType) const
    {
        if (vehicleType == "Bike") return pricing.bikeBase;
        if (vehicleType == "CNG") return pricing.cngBase;
        if (vehicleType == "UberX") return pricing.uberXBase;
        if (vehicleType == "Premium") return pricing.premiumBase;
        if (vehicleType == "Intercity") return pricing.intercityBase;
        return 0.0;
    }

    double getVehiclePerKm(const string &vehicleType) const
    {
        if (vehicleType == "Bike") return pricing.bikePerKm;
        if (vehicleType == "CNG") return pricing.cngPerKm;
        if (vehicleType == "UberX") return pricing.uberXPerKm;
        if (vehicleType == "Premium") return pricing.premiumPerKm;
        if (vehicleType == "Intercity") return pricing.intercityPerKm;
        return 0.0;
    }

    string lowerCopy(string s) const
    {
        transform(s.begin(), s.end(), s.begin(), [](unsigned char c) {
            return static_cast<char>(tolower(c));
        });
        return s;
    }

    string normalizeKey(string s) const
    {
        s = lowerCopy(s);

        size_t start = 0;
        while (start < s.size() && isspace(static_cast<unsigned char>(s[start])))
            ++start;

        size_t end = s.size();
        while (end > start && isspace(static_cast<unsigned char>(s[end - 1])))
            --end;

        string trimmed = s.substr(start, end - start);
        string out;
        out.reserve(trimmed.size());
        bool prevSpace = false;
        for (char c : trimmed)
        {
            bool nowSpace = isspace(static_cast<unsigned char>(c)) != 0;
            if (nowSpace)
            {
                if (!prevSpace)
                    out.push_back(' ');
            }
            else
            {
                out.push_back(c);
            }
            prevSpace = nowSpace;
        }
        return out;
    }

    void sortLocationsByName()
    {
        stable_sort(locations.begin(), locations.end(), [&](const Location &a, const Location &b) {
            string ak = normalizeKey(a.name);
            string bk = normalizeKey(b.name);
            if (ak != bk)
                return ak < bk;
            return a.name < b.name;
        });
    }

    double approxDistanceKm(double lat1, double lon1, double lat2, double lon2) const
    {
        double dx = lat1 - lat2;
        double dy = lon1 - lon2;
        return sqrt(dx * dx + dy * dy) * 111.0;
    }

    int findLocationIndexCaseInsensitive(const string &name) const
    {
        string target = normalizeKey(name);
        for (size_t i = 0; i < locations.size(); ++i)
        {
            if (normalizeKey(locations[i].name) == target)
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
        clampVal(pricing.intercityBase, 0.0, 5000.0, "intercity_base");
        clampVal(pricing.intercityPerKm, 0.0, 500.0, "intercity_per_km");

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
        fout << "intercity_base," << pricing.intercityBase << "\n";
        fout << "intercity_per_km," << pricing.intercityPerKm << "\n";
        fout << "traffic_weight," << pricing.trafficWeight << "\n";
        fout << "weather_weight," << pricing.weatherWeight << "\n";
        fout << "demand_tier1_ratio," << pricing.demandTier1Ratio << "\n";
        fout << "demand_tier1_surge," << pricing.demandTier1Surge << "\n";
        fout << "demand_tier2_ratio," << pricing.demandTier2Ratio << "\n";
        fout << "demand_tier2_surge," << pricing.demandTier2Surge << "\n";
        fout << "demand_tier3_ratio," << pricing.demandTier3Ratio << "\n";
        fout << "demand_tier3_surge," << pricing.demandTier3Surge << "\n";
        fout << "max_total_surge," << pricing.maxTotalSurge << "\n";
        fout << "dynamic_pricing_enabled," << (pricing.dynamicPricingEnabled ? "1" : "0") << "\n";
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
            else if (key == "intercity_base") pricing.intercityBase = v;
            else if (key == "intercity_per_km") pricing.intercityPerKm = v;
            else if (key == "traffic_weight") pricing.trafficWeight = v;
            else if (key == "weather_weight") pricing.weatherWeight = v;
            else if (key == "demand_tier1_ratio") pricing.demandTier1Ratio = v;
            else if (key == "demand_tier1_surge") pricing.demandTier1Surge = v;
            else if (key == "demand_tier2_ratio") pricing.demandTier2Ratio = v;
            else if (key == "demand_tier2_surge") pricing.demandTier2Surge = v;
            else if (key == "demand_tier3_ratio") pricing.demandTier3Ratio = v;
            else if (key == "demand_tier3_surge") pricing.demandTier3Surge = v;
            else if (key == "max_total_surge") pricing.maxTotalSurge = v;
            else if (key == "dynamic_pricing_enabled") pricing.dynamicPricingEnabled = (v != 0);
        }

        if (sanitizePricingConfig(true))
            savePricingConfig();
    }

    bool geocodeLocationCandidates(const string &query, vector<GeocodeCandidate> &out)
    {
        ostringstream cmd;
        cmd << "python geocode_lookup.py \"" << query << "\" > geocode_result.tmp";
        int result = system(cmd.str().c_str());
        if (result != 0)
            return false;

        ifstream fin("geocode_result.tmp");
        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string display, latStr, lonStr;
            getline(ss, display, '\t');
            getline(ss, latStr, '\t');
            getline(ss, lonStr, '\t');
            if (!display.empty() && !latStr.empty() && !lonStr.empty())
            {
                try
                {
                    out.push_back(GeocodeCandidate(display, stod(latStr), stod(lonStr)));
                }
                catch (...)
                {
                    continue;
                }
            }
        }

        remove("geocode_result.tmp");
        return !out.empty();
    }
    double calcDistance(const Location &a, const Location &b)
    {
        // very simple distance logic (beginner friendly)
        return approxDistanceKm(a.lat, a.lon, b.lat, b.lon);
    }

    int findLocationIndex(const string &name) {
        auto it = find_if(locations.begin(), locations.end(),
                      [&](const Location& loc){ return normalizeKey(loc.name) == normalizeKey(name); });
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
        vector<string> columns;
        string col;
        while (getline(ss, col, ',')) {
            columns.push_back(col);
        }

        if (columns.size() < 6) continue;

        string loc = columns[0];
        string yearStr = columns[1];
        string monthStr = columns[2];
        string dayStr = columns[3];
        string hourStr = columns[4];
        string factorStr = columns.back();

        if (loc == "Location" || yearStr == "Year" || factorStr == "TrafficMultiplier" || factorStr == "WeatherMultiplier")
            continue;

        try {
            trafficData.push_back(TrafficData(loc, stoi(yearStr), stoi(monthStr), stoi(dayStr), stoi(hourStr), stod(factorStr)));
        } catch (...) {
            continue;
        }
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
        vector<string> columns;
        string col;
        while (getline(ss, col, ',')) {
            columns.push_back(col);
        }

        if (columns.size() < 6) continue;
        
        loc = columns[0];
        yearStr = columns[1];
        monthStr = columns[2];
        dayStr = columns[3];
        hourStr = columns[4];
        // If 10 columns (live_updater full mode), the multiplier is the last one (index 9)
        // If 6 columns (sync mode or generator), the multiplier is the last one (index 5)
        factorStr = columns.back();

        if (loc == "Location" || yearStr == "Year" || factorStr == "WeatherMultiplier" || factorStr == "TrafficMultiplier")
            continue;

        try {
            weatherData.push_back(WeatherData(loc, stoi(yearStr), stoi(monthStr), stoi(dayStr), stoi(hourStr), stod(factorStr)));
        } catch (...) {
            continue;
        }
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
            if (normalizeKey(traffic.location) == normalizeKey(locName) && traffic.year == y && 
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
            if (normalizeKey(wd.location) == normalizeKey(locName) && wd.year == y && 
                wd.month == m && wd.day == d && wd.hour == h)
            {
                return wd.factor;
            }
        }
        return 1.0;
    }

    unique_ptr<IFareCalculator> makeCoreFareCalculator()
    {
        return make_unique<DynamicFareCalculator>(
            &pricing,
            [this](const string &s) { return getTrafficFactor(s); },
            [this](const string &s) { return getWeatherFactor(s); },
            [this](const string &v) { return getVehicleBase(v); },
            [this](const string &v) { return getVehiclePerKm(v); });
    }

    TripQuote computeTripQuote(double distance, const string &vehicleType, int availableDrivers,
                               int activeUsers, const string &locName, const string &promoEntry)
    {
        string trimmed = promoEntry;
        while (!trimmed.empty() && isspace(static_cast<unsigned char>(trimmed.front())))
            trimmed.erase(trimmed.begin());
        while (!trimmed.empty() && isspace(static_cast<unsigned char>(trimmed.back())))
            trimmed.pop_back();

        double promoFrac = 0;
        string promoLabel;
        if (trimmed == "SAVE10")
        {
            promoFrac = 0.10;
            promoLabel = "SAVE10";
        }
        else if (trimmed == "UBER5")
        {
            promoFrac = 0.05;
            promoLabel = "UBER5";
        }

        unique_ptr<IFareCalculator> engine = makeCoreFareCalculator();
        if (promoFrac > 0.0)
            engine = make_unique<PromoFareDecorator>(move(engine), promoFrac, promoLabel);
        return engine->compute(distance, vehicleType, availableDrivers, activeUsers, locName);
    }

    void printFareEstimateBreakdown(const TripQuote &q)
    {
        cout << "\n========= FARE ESTIMATE BREAKDOWN =========\n";
        cout << fixed << setprecision(2);
        cout << "Base fare             : " << q.baseCharge << " taka\n";
        cout << "Distance (" << q.distance << " km @ " << q.perKmRate << "/km) : " << (q.distance * q.perKmRate)
             << " taka\n";
        cout << "Subtotal              : " << q.baseFare << " taka\n\n";
        cout << "Surges:\n";
        cout << "  Traffic (" << q.trafficFactor << "x, weight " << pricing.trafficWeight << ") : +"
             << (q.trafficSurge * 100.0) << "%\n";
        cout << "  Weather (" << q.weatherFactor << "x, weight " << pricing.weatherWeight << ") : +"
             << (q.weatherSurge * 100.0) << "%\n";
        cout << "  Demand (supply ratio " << setprecision(3) << q.supplyRatio << ") : +" << setprecision(2)
             << (q.demandSurge * 100.0) << "%\n";
        cout << "  Total surge           : +" << (q.totalSurge * 100.0) << "%\n\n";
        if (q.promoSavings > 0.0)
            cout << "Promo (" << q.promoCode << ") savings : -" << q.promoSavings << " taka\n";
        cout << "TOTAL ESTIMATED FARE  : " << q.finalFare << " taka\n";
        cout << "==========================================\n";
    }

    double calculateDynamicFare(double distance, const string &vehicleType,
                                int availableDrivers, int activeUsers, string locName)
    {
        return computeTripQuote(distance, vehicleType, availableDrivers, activeUsers, locName, "").finalFare;
    }

    void showFareEstimate(double distance, const string &vehicleType,
                          int availableDrivers, int activeUsers, string locName)
    {
        TripQuote q = computeTripQuote(distance, vehicleType, availableDrivers, activeUsers, locName, "");
        printFareEstimateBreakdown(q);
    }

    void loadUsers()
    {
        ifstream fin("users.csv");
        if (!fin)
        {
            // Try legacy users.txt if csv doesn't exist
            ifstream legacy("users.txt");
            if (!legacy) return;
            
            string line;
            while (getline(legacy, line))
            {
                if (line.empty()) continue;
                stringstream ss(line);
                string u, p;
                double walletAmount = 1000.0;
                ss >> u >> p >> walletAmount;
                User newUser(u, p, Money(walletAmount));
                
                string favs;
                if (getline(ss, favs)) {
                    stringstream fss(favs);
                    string f;
                    while (getline(fss, f, ':')) {
                        size_t start = f.find_first_not_of(" ");
                        if (start != string::npos) {
                            f = f.substr(start);
                            newUser.addFavorite(f);
                        }
                    }
                }
                users.push_back(newUser);
            }
            legacy.close();
            saveUsers(); // Migrate to CSV immediately
            return;
        }

        string line;
        while (getline(fin, line))
        {
            if (line.empty()) continue;

            stringstream ss(line);
            string u, p, walletStr, favStr;
            
            getline(ss, u, ',');
            getline(ss, p, ',');
            getline(ss, walletStr, ',');
            getline(ss, favStr, ',');

            double walletAmount = 1000.0;
            try {
                if (!walletStr.empty()) walletAmount = stod(walletStr);
            } catch (...) {}

            string cardStr;
            getline(ss, cardStr, ',');
            double cardAmount = 1500.0;
            try {
                if (!cardStr.empty())
                    cardAmount = stod(cardStr);
            } catch (...) {}

            User newUser(u, p, Money(walletAmount), Money(cardAmount));
            if (!favStr.empty())
            {
                stringstream favSS(favStr);
                string fav;
                while (getline(favSS, fav, ':'))
                {
                    if (!fav.empty()) newUser.addFavorite(fav);
                }
            }
            users.push_back(newUser);
        }
    }

    void saveUsers()
    {
        ofstream fout("users.csv");
        for (const auto &u : users)
        {
            fout << u.getUsername() << "," << u.getPassword() << "," << fixed << setprecision(2) << u.getWallet().get() << ",";
            
            vector<string> favs = u.getFavoriteLocations();
            for (size_t i = 0; i < favs.size(); ++i)
            {
                fout << favs[i];
                if (i < favs.size() - 1)
                    fout << ":";
            }
            fout << "," << fixed << setprecision(2) << u.getCardBalance().get() << "\n";
        }
    }

    void loadLocations()
    {
        ifstream fin("location.csv");
        if (!fin) {
            perror("Error opening location.csv");
            throw runtime_error("location.csv not found");
        }

        string line;
        while (getline(fin, line))
        {
            if (line.empty())
                continue;

            stringstream ss(line);
            string n, la, lo;
            getline(ss, n, ',');
            getline(ss, la, ',');
            getline(ss, lo, ',');

            try
            {
                locations.push_back(Location(n, stod(la), stod(lo)));
            }
            catch (...)
            {
                continue;
            }
        }
        sortLocationsByName();
    }

    void saveLocations()
    {
        sortLocationsByName();
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
            if (line.empty()) continue;
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
            double totalEarnVal = 0.0;
            int tripCnt = 0;
            double avgRatingVal = 0.0;
            int ratingCntVal = 0;
            
            try {
                if (!earnStr.empty()) totalEarnVal = stod(earnStr);
                if (!tripStr.empty()) tripCnt = stoi(tripStr);
                if (!avgRatingStr.empty()) avgRatingVal = stod(avgRatingStr);
                if (!ratingCountStr.empty()) ratingCntVal = stoi(ratingCountStr);
            } catch (...) {}

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
            Ride r;
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

            try {
                if (!d1.empty()) r.distance = stod(d1);
                if (!d2.empty()) r.fare = stod(d2);
            } catch (...) {
                continue;
            }

            string opt;
            if (getline(ss, opt, ','))
                r.status = opt;
            if (getline(ss, opt, ','))
            {
                try
                {
                    if (!opt.empty())
                        r.estimateFare = stod(opt);
                } catch (...) {}
            }
            if (getline(ss, opt, ','))
            {
                try
                {
                    if (!opt.empty())
                        r.surgeFraction = stod(opt);
                } catch (...) {}
            }
            if (getline(ss, opt, ','))
            {
                try
                {
                    if (!opt.empty())
                        r.promoDiscount = stod(opt);
                } catch (...) {}
            }
            if (getline(ss, opt, ','))
                r.promoCode = opt;

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
             << "," << r.payment << "," << r.status << "," << fixed << setprecision(2) << r.estimateFare << ","
             << fixed << setprecision(4) << r.surgeFraction << "," << fixed << setprecision(2) << r.promoDiscount
             << "," << r.promoCode << "\n";
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
            cout << "\n----- YOUR TRIPS (history) -----\n";
            bool found = false;
            for (const auto &r : rides)
            {
                if (r.user == name)
                {
                    found = true;
                    cout << r.timeStamp << " | " << r.status << " | " << r.from << " -> " << r.to << " | "
                         << r.vehicle << " | " << r.payment << " | est " << r.estimateFare << " | paid "
                         << r.fare << "\n";
                }
            }
            if (!found)
                cout << "No trips yet.\n";
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
            cout << "Current values for driver: " << dname << "\n";
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
        int fromI = 0;
        int toI = 0;
        
        cout << "\n--- SELECT PICKUP LOCATION ---\n";
        
        vector<string> favs = u.getFavoriteLocations();
        if (!favs.empty())
        {
            cout << "Your Favorite Locations:\n";
            for (size_t i = 0; i < favs.size(); ++i)
            {
                cout << "  (" << (i + 1) << ") " << favs[i] << "\n";
            }
            int fav = InputValidator<int>::getInput("Select favorite (or 0 to browse all): ");
            if (fav >= 1 && fav <= (int)favs.size())
            {
                string fromI_str = favs[static_cast<size_t>(fav - 1)];
                int idx = findLocationIndexCaseInsensitive(fromI_str);
                if (idx >= 0)
                {
                    fromI = idx + 1;
                }
                else
                {
                    cout << "Favorite location not found. Browsing all locations.\n";
                    showLocations();
                    fromI = InputValidator<int>::getInput("Pick FROM location number: ");
                }
            }
            else if (fav != 0)
            {
                cout << "Invalid choice.\n";
                return;
            }
            else
            {
                showLocations();
                fromI = InputValidator<int>::getInput("Pick FROM location number: ");
            }
        }
        else
        {
            showLocations();
            fromI = InputValidator<int>::getInput("Pick FROM location number: ");
        }

        toI = InputValidator<int>::getInput("Pick TO location number  : ");

        if (fromI < 1 || fromI > (int)locations.size() || toI < 1 || toI > (int)locations.size() ||
            fromI == toI)
        {
            cout << "Invalid location choice.\n";
            return;
        }

        cout << "\nVehicle Types:\n";
        cout << "1. Bike\n2. CNG\n3. UberX\n4. Premium\n5. Intercity\n";
        int v = InputValidator<int>::getInput("Choose vehicle: ");

        if (v < 1 || v > 5)
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
        else if (v == 5)
            selectedVehicle = "Intercity";

        if (selectedVehicle.empty())
        {
            cout << "Vehicle select failed.\n";
            return;
        }

        Location from = locations[static_cast<size_t>(fromI - 1)];
        Location to = locations[static_cast<size_t>(toI - 1)];

        // Note: API refresh is now done at program startup to save time here.
        // We use the cached traffic/weather data loaded at startup.

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
        else if (v == 5)
            vehicle = new Intercity();

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

        cout << "\n[Phase: " << rideStatusLabel(RideStatus::Matched) << "] Driver matched.\n";

        cout << "Promo codes (optional): SAVE10 / UBER5\n";
        cout << "Promo (or leave blank): ";
        string promoRaw;
        getline(cin >> ws, promoRaw);

        TripQuote quote =
            computeTripQuote(tripDistance, selectedVehicle, availableDrivers, activeUsers, from.name, promoRaw);
        printFareEstimateBreakdown(quote);
        double fareAmount = quote.finalFare;

        cout << "\nPayment method:\n";
        cout << "1. Wallet\n";
        cout << "2. Cash\n";
        cout << "3. Card (simulated balance)\n";
        int payChoice = InputValidator<int>::getInput("Choose payment: ");

        WalletPaymentMethod walletPay;
        CashPaymentMethod cashPay;
        CardPaymentMethod cardPay;
        IPaymentMethod *paymentMethod = nullptr;
        if (payChoice == 1)
            paymentMethod = &walletPay;
        else if (payChoice == 2)
            paymentMethod = &cashPay;
        else if (payChoice == 3)
            paymentMethod = &cardPay;
        else
        {
            cout << "Invalid payment selection.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }

        if (payChoice == 1 && u.getWallet().get() < fareAmount)
        {
            cout << "\nInsufficient wallet balance.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }
        if (payChoice == 3 && u.getCardBalance().get() < fareAmount)
        {
            cout << "\nInsufficient card balance.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }

        int confirm = InputValidator<int>::getInput("Confirm trip booking? (1=Yes, 0=No): ");
        if (confirm != 1)
        {
            cout << "Trip cancelled.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }

        int beginChoice =
            InputValidator<int>::getInput("1. Start trip   2. Cancel trip (fee applies to wallet)\nChoice: ");
        if (beginChoice == 2)
        {
            double cfee = cancelPolicy.cancellationFee(true);
            WalletPaymentMethod feeMethod;
            cout << fixed << setprecision(2) << "Cancellation fee: " << cfee << " taka (wallet).\n";
            if (!feeMethod.charge(u, cfee))
                cout << "Insufficient wallet for cancellation fee.\n";
            else
                saveUsers();
            driver.setAvailable(true);
            delete vehicle;
            cout << "Trip cancelled.\n";
            return;
        }
        if (beginChoice != 1)
        {
            cout << "Invalid choice. Restoring driver availability.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }

        static int serial = 0;
        serial++;

        int eta = (int)round(((pickupDistance + tripDistance) / speed) * 60.0);

        cout << "\n=== TRIP CONFIRMED ===\n";
        cout << "[Phase: " << rideStatusLabel(RideStatus::Matched) << "]\n";
        cout << "Driver : " << driver.getUsername() << " (" << fixed << setprecision(2) << pickupDistance
             << " km away)\n";
        cout << "Vehicle: " << selectedVehicle << "\n";
        cout << "Trip   : " << fixed << setprecision(2) << tripDistance << " km\n";
        cout << "ETA    : " << eta << " mins\n";
        cout << "Fare   : " << fareAmount << " taka via " << paymentMethod->label() << "\n";
        cout << "Trip # : " << serial << "\n";

        int driverWaitTime = (int)max(1.0, round((pickupDistance / speed) * 60.0));
        int journeyTime = (int)max(1.0, round((tripDistance / speed) * 60.0));
        cout << "\nPress any key to start live tracking...";
        _getch();
        system("cls");
        simulateRideProgress(driverWaitTime, journeyTime);

        if (!paymentMethod->charge(u, fareAmount))
        {
            cout << "Payment failed after trip completion.\n";
            driver.setAvailable(true);
            delete vehicle;
            return;
        }
        saveUsers();

        cout << "Payment captured: " << fareAmount << " taka (" << paymentMethod->label() << ").\n";

        driver.addTrip(company.getDriverPart(fareAmount));
        driver.setLocation(to.name);
        driver.setAvailable(true);

        company.addRideRevenue(fareAmount);
        delete vehicle;

        Ride r;
        r.timeStamp = nowTime();
        r.user = u.getUsername();
        r.driver = driver.getUsername();
        r.from = from.name;
        r.to = to.name;
        r.distance = tripDistance;
        r.fare = fareAmount;
        r.vehicle = selectedVehicle;
        r.payment = paymentMethod->label();
        r.status = rideStatusLabel(RideStatus::Completed);
        r.estimateFare = quote.finalFare + quote.promoSavings;
        r.surgeFraction = quote.totalSurge;
        r.promoDiscount = quote.promoSavings;
        r.promoCode = quote.promoCode;

        rides.push_back(r);
        appendRide(r);

        ReceiptFormatter::printTripReceipt(quote, r, company, paymentMethod->label());

        cout << "\n--- RATE YOUR TRIP (required) ---\n";
        int rVal = 0;
        while (rVal < 1 || rVal > 5)
        {
            rVal = InputValidator<int>::getInput("Rate your driver (1-5): ");
            if (rVal < 1 || rVal > 5)
                cout << "Enter a whole number from 1 to 5.\n";
        }
        driver.addRating(rVal);
        cout << "Thanks for your feedback.\n";

        int viewMap = InputValidator<int>::getInput("View ride route on map? (1=Yes, 0=No): ");
        if (viewMap == 1)
            viewRideMap(from.name, to.name);

        saveDrivers();
        cout << "\nTrip finished. Press any key to return to menu...";
        _getch();
    }

    void userMenu(User &u)
    {
        while (true)
        {
            cout << "\n--- USER MENU (" << u.getUsername() << ") ---\n";
            cout << "1. Request a Ride\n";
            cout << "2. Top up Wallet / Card\n";
            cout << "3. Your trips (history)\n";
            cout << "4. Add/View Favorite Locations\n";
            cout << "5. View Map\n";
            cout << "6. Back to Main Menu\n";

            int ch = InputValidator<int>::getInput("Choice: ");

            if (ch == 1)
            {
                requestRide(u);
            }
            else if (ch == 2)
            {
                system("cls");
                cout << "--- WALLET / CARD ---\n";
                cout << "Wallet balance: " << u.getWallet() << " taka\n";
                cout << "Card balance  : " << u.getCardBalance() << " taka\n";
                int add = InputValidator<int>::getInput("Add money to wallet (0 to skip): ");
                if (add > 0)
                {
                    u.addMoney(add);
                    saveUsers();
                }
                int addCard = InputValidator<int>::getInput("Add money to card (0 to skip): ");
                if (addCard > 0)
                {
                    u.addCardMoney(addCard);
                    saveUsers();
                }
            }
            else if (ch == 3)
            {
                showRideHistory(u.getUsername(), "User");
            }
            else if (ch == 4)
            {
                manageFavoriteLocations(u);
            }
            else if (ch == 5)
            {
                viewSystemMap();
            }
            else if (ch == 6)
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

    void manageFavoriteLocations(User &u)
    {
        while (true)
        {
            system("cls");
            cout << "\n--- MANAGE FAVORITE LOCATIONS ---\n";
            vector<string> favs = u.getFavoriteLocations();
            
            if (favs.empty())
            {
                cout << "No favorite locations saved yet. (Max 5)\n\n";
            }
            else
            {
                cout << "Your Favorites:\n";
                for (size_t i = 0; i < favs.size(); ++i)
                {
                    cout << "  " << (i + 1) << ". " << favs[i] << "\n";
                }
                cout << "\n";
            }

            cout << "1. Add Favorite\n";
            if (!favs.empty())
                cout << "2. Remove Favorite\n";
            cout << "3. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");

            if (ch == 1)
            {
                if (favs.size() >= 5)
                {
                    cout << "Maximum 5 favorites allowed. Remove one first.\n";
                    cout << "Press any key to continue...";
                    _getch();
                    continue;
                }

                showLocations();
                int idx = InputValidator<int>::getInput("Select location number to add as favorite: ");
                if (idx >= 1 && idx <= (int)locations.size())
                {
                    string locName = locations[static_cast<size_t>(idx - 1)].name;
                    u.addFavorite(locName);
                    saveUsers();
                    cout << "Added '" << locName << "' as favorite.\n";
                    cout << "Press any key to continue...";
                    _getch();
                }
                else
                {
                    cout << "Invalid location.\n";
                    cout << "Press any key to continue...";
                    _getch();
                }
            }
            else if (ch == 2 && !favs.empty())
            {
                int idx = InputValidator<int>::getInput("Select favorite number to remove: ");
                if (idx >= 1 && idx <= (int)favs.size())
                {
                    string toRemove = favs[static_cast<size_t>(idx - 1)];
                    u.removeFavorite(toRemove);
                    saveUsers();
                    cout << "Removed '" << toRemove << "' from favorites.\n";
                    cout << "Press any key to continue...";
                    _getch();
                }
                else
                {
                    cout << "Invalid selection.\n";
                    cout << "Press any key to continue...";
                    _getch();
                }
            }
            else if (ch == 3)
            {
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
            cout << "1. View Ride History\n";
            cout << "2. Toggle Availability (Current: " << (d.isAvailable() ? "Available" : "Busy") << ")\n";
            cout << "3. View Driver Stats\n";
            cout << "4. View Map\n";
            cout << "5. Back to Main Menu\n";

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
                viewSystemMap();
            }
            else if (ch == 5)
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
    void viewSystemMap()
    {
        system("cls");
        cout << "Generating system map...\n";
        system("python map_generator.py system");
        cout << "System map opened in browser.\n";
        cout << "Press any key to continue...";
        _getch();
    }

    void viewRideMap(string fromLoc, string toLoc)
    {
        system("cls");
        cout << "Generating ride map with real road routing...\n";
        string cmd = "python map_generator.py ride \"" + fromLoc + "\" \"" + toLoc + "\"";
        int result = system(cmd.c_str());
        if (result == 0)
            cout << "Ride map opened in browser.\n";
        else
            cout << "Could not generate map.\n";
        cout << "Press any key to continue...";
        _getch();
    }

    void viewHeatmap()
    {
        system("cls");
        cout << "Generating Demand Heatmap...\n";
        system("python map_generator.py heatmap");
        cout << "Heatmap opened in browser.\n";
        cout << "Press any key to continue...";
        _getch();
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

                vector<GeocodeCandidate> candidates;
                if (!geocodeLocationCandidates(n, candidates))
                {
                    cout << "Geocoding failed. Ensure internet is available and location name is valid.\n";
                    continue;
                }

                double refLat = candidates[0].lat;
                double refLon = candidates[0].lon;
                stable_sort(candidates.begin(), candidates.end(), [&](const GeocodeCandidate &a, const GeocodeCandidate &b) {
                    double da = approxDistanceKm(refLat, refLon, a.lat, a.lon);
                    double db = approxDistanceKm(refLat, refLon, b.lat, b.lon);
                    if (da != db)
                        return da < db;
                    return normalizeKey(a.display) < normalizeKey(b.display);
                });

                cout << "Top 10 geocoding matches (closest to farthest):\n";
                for (size_t i = 0; i < candidates.size(); ++i)
                {
                    double dkm = approxDistanceKm(refLat, refLon, candidates[i].lat, candidates[i].lon);
                    cout << "  " << i + 1 << ". " << candidates[i].display << " ("
                         << fixed << setprecision(6) << candidates[i].lat << ", " << candidates[i].lon
                         << ") ~" << setprecision(2) << dkm << " km\n";
                }

                int pick = InputValidator<int>::getInput("Select match serial (0 to cancel): ");
                if (pick == 0)
                {
                    cout << "Cancelled.\n";
                    continue;
                }

                if (pick < 1 || pick > static_cast<int>(candidates.size()))
                {
                    cout << "Invalid selection.\n";
                    continue;
                }

                const GeocodeCandidate &chosen = candidates[static_cast<size_t>(pick - 1)];
                string finalName = chosen.display;
                double la = chosen.lat;
                double lo = chosen.lon;
                cout << "Matched coordinates: " << fixed << setprecision(6) << la << ", " << lo << "\n";

                int dupIdx = findLocationIndexCaseInsensitive(finalName);
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

                locations.push_back(Location(finalName, la, lo));
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
                    if (normalizeKey(d.getLocation()) == normalizeKey(target.name))
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
            cout << "6. Toggle Dynamic Pricing\n";
            cout << "7. Back\n";

            int ch = InputValidator<int>::getInput("Choice: ");
            if (ch == 1)
            {
                cout << "\nVehicle Pricing:\n";
                cout << "Bike    : base=" << pricing.bikeBase << ", perKm=" << pricing.bikePerKm << "\n";
                cout << "CNG     : base=" << pricing.cngBase << ", perKm=" << pricing.cngPerKm << "\n";
                cout << "UberX   : base=" << pricing.uberXBase << ", perKm=" << pricing.uberXPerKm << "\n";
                cout << "Premium : base=" << pricing.premiumBase << ", perKm=" << pricing.premiumPerKm << "\n";
                cout << "Intercity: base=" << pricing.intercityBase << ", perKm=" << pricing.intercityPerKm << "\n";

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
                cout << "\nSelect Vehicle: 1.Bike 2.CNG 3.UberX 4.Premium 5.Intercity\n";
                int v = InputValidator<int>::getInput("Vehicle choice: ");

                string keyBase, keyPerKm;
                double curBase = 0.0, curPerKm = 0.0;
                if (v == 1) { keyBase = "bike_base"; keyPerKm = "bike_per_km"; curBase = pricing.bikeBase; curPerKm = pricing.bikePerKm; }
                else if (v == 2) { keyBase = "cng_base"; keyPerKm = "cng_per_km"; curBase = pricing.cngBase; curPerKm = pricing.cngPerKm; }
                else if (v == 3) { keyBase = "uberx_base"; keyPerKm = "uberx_per_km"; curBase = pricing.uberXBase; curPerKm = pricing.uberXPerKm; }
                else if (v == 4) { keyBase = "premium_base"; keyPerKm = "premium_per_km"; curBase = pricing.premiumBase; curPerKm = pricing.premiumPerKm; }
                else if (v == 5) { keyBase = "intercity_base"; keyPerKm = "intercity_per_km"; curBase = pricing.intercityBase; curPerKm = pricing.intercityPerKm; }
                else { cout << "Invalid vehicle choice.\n"; continue; }

                cout << "Current values - (" << keyBase << ") = " << curBase << ", (" << keyPerKm << ") = " << curPerKm << "\n";
                double base = InputValidator<double>::getInput((string("New base fare (key: ") + keyBase + "): "));
                double perKm = InputValidator<double>::getInput((string("New per-km fare (key: ") + keyPerKm + "): "));
                if (base < 0 || base > 5000 || perKm < 0 || perKm > 500)
                {
                    cout << "Invalid: base must be 0-5000 and per-km must be 0-500.\n";
                    continue;
                }

                if (v == 1) { pricing.bikeBase = base; pricing.bikePerKm = perKm; }
                else if (v == 2) { pricing.cngBase = base; pricing.cngPerKm = perKm; }
                else if (v == 3) { pricing.uberXBase = base; pricing.uberXPerKm = perKm; }
                else if (v == 4) { pricing.premiumBase = base; pricing.premiumPerKm = perKm; }
                else if (v == 5) { pricing.intercityBase = base; pricing.intercityPerKm = perKm; }

                savePricingConfig();
                cout << "Vehicle pricing updated.\n";
            }
            else if (ch == 3)
            {
                cout << "Current values - (traffic_weight) = " << pricing.trafficWeight << ", (weather_weight) = " << pricing.weatherWeight << "\n";
                double tw = InputValidator<double>::getInput("Traffic weight (key: traffic_weight): ");
                double ww = InputValidator<double>::getInput("Weather weight (key: weather_weight): ");
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
                cout << "Current Demand Tiers:\n";
                cout << "  (demand_tier1_ratio) = " << pricing.demandTier1Ratio << ", (demand_tier1_surge) = " << pricing.demandTier1Surge << "\n";
                cout << "  (demand_tier2_ratio) = " << pricing.demandTier2Ratio << ", (demand_tier2_surge) = " << pricing.demandTier2Surge << "\n";
                cout << "  (demand_tier3_ratio) = " << pricing.demandTier3Ratio << ", (demand_tier3_surge) = " << pricing.demandTier3Surge << "\n";

                cout << "Enter tier ratio and surge values in increasing ratio order.\n";
                double r1 = InputValidator<double>::getInput("Tier1 ratio (key: demand_tier1_ratio): ");
                double s1 = InputValidator<double>::getInput("Tier1 surge (key: demand_tier1_surge): ");
                double r2 = InputValidator<double>::getInput("Tier2 ratio (key: demand_tier2_ratio): ");
                double s2 = InputValidator<double>::getInput("Tier2 surge (key: demand_tier2_surge): ");
                double r3 = InputValidator<double>::getInput("Tier3 ratio (key: demand_tier3_ratio): ");
                double s3 = InputValidator<double>::getInput("Tier3 surge (key: demand_tier3_surge): ");

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
                cout << "Current value - (max_total_surge) = " << pricing.maxTotalSurge << "\n";
                double cap = InputValidator<double>::getInput("Max total surge (key: max_total_surge): ");
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
                pricing.dynamicPricingEnabled = !pricing.dynamicPricingEnabled;
                savePricingConfig();
                cout << "Dynamic Pricing is now " << (pricing.dynamicPricingEnabled ? "ENABLED" : "DISABLED") << ".\n";
            }
            else if (ch == 7)
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
                    if (normalizeKey(t.location) == normalizeKey(loc) && t.hour == hour && t.year == y && t.month == m && t.day == d)
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
            else
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
                    if (normalizeKey(w.location) == normalizeKey(loc) && w.hour == hour && w.year == y && w.month == m && w.day == d)
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
            else
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
            cout << "8. View Demand Heatmap\n";
            cout << "9. Back\n";

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
                viewHeatmap();
            }
            else if (ch == 9)
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
        // SYNC LIVE DATA AT STARTUP (OPTIONAL)
        system("cls");
        cout << "===========================================\n";
        cout << "   DHAKA BASED RIDE SHARING SYSTEM \n";
        cout << "===========================================\n\n";
        cout << "Would you like to sync live traffic & weather data? (Slow)\n";
        cout << "1. Yes (Sync now)\n";
        cout << "2. No  (Use cached data)\n";
        int syncChoice = InputValidator<int>::getInput("Choice: ");
        
        if (syncChoice == 1) {
            cout << "Syncing... Please wait (this may take a few minutes)\n";
            system("python live_updater.py sync");
        }
        
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
        cout << "5. Intercity\n";
        int v = InputValidator<int>::getInput("Choose vehicle: ");

        Vehicle *vehicle = nullptr;
        
        if (v == 1) vehicle = new Bike();
        else if (v == 2) vehicle = new CNG();
        else if (v == 3) vehicle = new UberX();
        else if (v == 4) vehicle = new Premium();
        else if (v == 5) vehicle = new Intercity();
        
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
            cout << "6. View System Map\n";
            cout << "7. Exit\n";

            int mainChoice = InputValidator<int>::getInput("Choice: ");

            if (mainChoice == 7)
            {
                app.saveAll();
                cout << "Goodbye!\n";
                system("cls");
                break;
            }

            if (mainChoice == 6)
            {
                app.viewSystemMap();
                continue;
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
