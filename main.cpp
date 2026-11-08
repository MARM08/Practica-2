/* =========================================================================
   Miguel Angel Restrepo Martinez
   =========================================================================

   PRACTICE - COMPLETED CODE
   Integrated Itinerary and Fare Engine - Medellin Metro System
   Programming Languages and Paradigms (ST0244) - Universidad EAFIT

   Build: g++ -std=c++17 -Wall -Wextra -o practice main.cpp
   ========================================================================= */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>
#include <numeric>
#include <map>
#include <functional>
#include <utility>

std::string formatCOP(long long value) {
    std::string digits = std::to_string(value);
    std::string out;
    int count = 0;
    for (auto it = digits.rbegin(); it != digits.rend(); ++it) {
        out.push_back(*it);
        if (++count % 3 == 0 && it + 1 != digits.rend()) {
            out.push_back('.');
        }
    }
    std::reverse(out.begin(), out.end());
    return "$ " + out;
}

class TransportMode {
public:
    TransportMode(std::string name, int stations)
        : name_(std::move(name)), stations_(stations) {}
    virtual ~TransportMode() = default;

    const std::string& name() const { return name_; }
    int stations() const { return stations_; }
    virtual long long baseFare() const = 0;
    virtual std::string serviceType() const = 0;
    virtual bool allowsStudentFlatFare() const { return true; }
    virtual bool allowsIntegration() const { return true; }
    virtual bool opensIntegrationWindow() const { return true; }
    std::string describe() const {
        return serviceType() + " " + name_ + " (" + std::to_string(stations_) +
               " stations) - full fare " + formatCOP(baseFare());
    }

private:
    std::string name_;
    int stations_;
};

class MetroLine : public TransportMode {
public:
    using TransportMode::TransportMode;
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Metro line"; }
};

class TramLine : public TransportMode {
public:
    using TransportMode::TransportMode;
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Tram line"; }
};

class CableLine : public TransportMode {
public:
    using TransportMode::TransportMode;
    long long baseFare() const override { return 3820; }
    std::string serviceType() const override { return "Cable line"; }
};

class TouristCable : public CableLine {
public:
    TouristCable(std::string name, int stations, long long surcharge)
        : CableLine(std::move(name), stations), surcharge_(surcharge) {}
    long long baseFare() const override { return CableLine::baseFare() + surcharge_; }
    std::string serviceType() const override { return "Tourist cable"; }
    bool allowsStudentFlatFare() const override { return false; }
    bool allowsIntegration() const override { return false; }
    bool opensIntegrationWindow() const override { return false; }

private:
    long long surcharge_;
};

class FeederRoute : public TransportMode {
public:
    using TransportMode::TransportMode;
    long long baseFare() const override { return 0; }
    std::string serviceType() const override { return "Feeder route"; }
    bool opensIntegrationWindow() const override { return false; }
};

class Passenger {
public:
    virtual ~Passenger() = default;
    virtual std::string profile() const = 0;
    long long fareOn(const TransportMode& mode) const {
        return std::max(0LL, computeFare(mode));
    }

protected:
    virtual long long computeFare(const TransportMode& mode) const {
        return mode.baseFare();
    }
};

class FrequentPassenger : public Passenger {
public:
    explicit FrequentPassenger(std::string name) : name_(std::move(name)) {}
    std::string profile() const override { return "Civica frequent"; }

private:
    std::string name_;
};

class StudentPassenger : public Passenger {
public:
    explicit StudentPassenger(std::string name) : name_(std::move(name)) {}
    std::string profile() const override { return "Municipal student"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return mode.allowsStudentFlatFare() ? 1600 : Passenger::computeFare(mode);
    }

private:
    std::string name_;
};

class SeniorPassenger : public Passenger {
public:
    explicit SeniorPassenger(std::string name) : name_(std::move(name)) {}
    std::string profile() const override { return "Senior citizen"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 490;
    }

private:
    std::string name_;
};

class DisabilityPassenger : public Passenger {
public:
    explicit DisabilityPassenger(std::string name) : name_(std::move(name)) {}
    std::string profile() const override { return "Person with disability"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) - 1100;
    }

private:
    std::string name_;
};

class BearerPassenger : public Passenger {
public:
    explicit BearerPassenger(std::string name) : name_(std::move(name)) {}
    std::string profile() const override { return "Bearer (occasional)"; }

protected:
    long long computeFare(const TransportMode& mode) const override {
        return Passenger::computeFare(mode) + 580;
    }

private:
    std::string name_;
};

struct Leg {
    int minute;
    const TransportMode* mode;
};

struct ChargedLeg {
    int minute;
    std::string modeName;
    std::string serviceType;
    long long charge;
    bool integrated;
};

class Network {
public:
    TransportMode& add(std::unique_ptr<TransportMode> mode) {
        modes_.push_back(std::move(mode));
        return *modes_.back();
    }
    const std::vector<std::unique_ptr<TransportMode>>& modes() const { return modes_; }
    void printInventory() const {
        std::for_each(modes_.begin(), modes_.end(), [](const auto& mode) {
            std::cout << "  - " << mode->describe() << '\n';
        });
    }

private:
    std::vector<std::unique_ptr<TransportMode>> modes_;
};

std::vector<ChargedLeg> chargeItinerary(std::vector<Leg> legs,
                                        const Passenger& passenger) {
    std::sort(legs.begin(), legs.end(), [](const Leg& left, const Leg& right) {
        return left.minute < right.minute;
    });

    std::vector<ChargedLeg> charged;
    bool windowOpen = false;
    int windowStart = 0;
    int freeTransfers = 0;

    for (const Leg& leg : legs) {
        const TransportMode& mode = *leg.mode;
        const bool inWindow = windowOpen && leg.minute - windowStart < 90;
        const bool isFreeTransfer = mode.allowsIntegration() &&
                                   mode.opensIntegrationWindow() &&
                                   inWindow && freeTransfers < 2;
        long long amount = 0;

        if (isFreeTransfer) {
            ++freeTransfers;
        } else {
            amount = passenger.fareOn(mode);
            if (mode.allowsIntegration() && mode.opensIntegrationWindow()) {
                windowOpen = true;
                windowStart = leg.minute;
                freeTransfers = 0;
            }
        }

        charged.push_back({leg.minute, mode.name(), mode.serviceType(), amount,
                           isFreeTransfer});
    }
    return charged;
}

long long totalPaid(const std::vector<ChargedLeg>& legs) {
    return std::accumulate(legs.begin(), legs.end(), 0LL,
        [](long long total, const ChargedLeg& leg) { return total + leg.charge; });
}

long long freeTransferCount(const std::vector<ChargedLeg>& legs) {
    return static_cast<long long>(std::count_if(legs.begin(), legs.end(),
        [](const ChargedLeg& leg) { return leg.integrated; }));
}

std::map<std::string, long long> paidByServiceType(const std::vector<ChargedLeg>& legs) {
    std::map<std::string, long long> totals;
    std::for_each(legs.begin(), legs.end(), [&totals](const ChargedLeg& leg) {
        totals[leg.serviceType] += leg.charge;
    });
    return totals;
}

std::vector<std::string> namesMatching(
    const Network& network,
    const std::function<bool(const TransportMode&)>& policy) {
    std::vector<std::string> names;
    std::for_each(network.modes().begin(), network.modes().end(),
        [&names, &policy](const auto& mode) {
            if (policy(*mode)) names.push_back(mode->name());
        });
    return names;
}

std::string joined(const std::vector<std::string>& strings) {
    return std::accumulate(strings.begin(), strings.end(), std::string(),
        [](const std::string& result, const std::string& value) {
            return result.empty() ? value : result + ", " + value;
        });
}

int main() {
    std::cout << "============================================================\n";
    std::cout << "   INTEGRATED ITINERARY AND FARE ENGINE - Valle de Aburra\n";
    std::cout << "============================================================\n\n";

    Network network;
    TransportMode& lineA  = network.add(std::make_unique<MetroLine>("A", 21));
    TransportMode& lineB  = network.add(std::make_unique<MetroLine>("B", 7));
    TransportMode& tramTA = network.add(std::make_unique<TramLine>("T-A Ayacucho", 9));
    TransportMode& cableK = network.add(std::make_unique<CableLine>("K Santo Domingo", 4));
    TransportMode& arvi   = network.add(std::make_unique<TouristCable>("L Arvi", 2, 9180));
    TransportMode& feeder = network.add(std::make_unique<FeederRoute>("130 Robledo", 18));

    std::cout << ">> 1. NETWORK INVENTORY (polymorphic describe)\n";
    network.printInventory();
    std::cout << '\n';

    const std::vector<Leg> itinerary = {
        {45, &cableK}, {0, &feeder}, {12, &lineA}, {150, &lineB},
        {70, &tramTA}, {200, &arvi}
    };

    const FrequentPassenger frequent("Diego");
    const StudentPassenger student("Valentina");
    const SeniorPassenger senior("Dona Amparo");

    std::cout << ">> 2. FULL FARE PER PROFILE ON METRO LINE A\n";
    std::cout << "  " << frequent.profile() << " pays " << formatCOP(frequent.fareOn(lineA)) << '\n';
    std::cout << "  " << student.profile() << " pays " << formatCOP(student.fareOn(lineA)) << '\n';
    std::cout << "  " << senior.profile() << " pays " << formatCOP(senior.fareOn(lineA)) << '\n';
    std::cout << "  " << student.profile() << " on the tourist cable pays "
              << formatCOP(student.fareOn(arvi)) << " (no flat fare there)\n\n";

    const std::vector<ChargedLeg> charged = chargeItinerary(itinerary, frequent);
    std::cout << ">> 3. CHARGED ITINERARY (frequent profile)\n";
    std::for_each(charged.begin(), charged.end(), [](const ChargedLeg& c) {
        std::cout << "  min " << c.minute << " | " << c.serviceType << " " << c.modeName
                  << " | " << formatCOP(c.charge)
                  << (c.integrated ? "  [free transfer]" : "") << '\n';
    });
    std::cout << '\n';

    std::cout << ">> 4. REPORTS BUILT WITH STL ALGORITHMS\n";
    std::cout << "  accumulate: total paid -> " << formatCOP(totalPaid(charged)) << '\n';
    std::cout << "  count_if: free transfers -> " << freeTransferCount(charged) << '\n';
    const auto byType = paidByServiceType(charged);
    std::for_each(byType.begin(), byType.end(),
        [](const std::pair<const std::string, long long>& entry) {
            std::cout << "  map: paid on " << entry.first << " -> "
                      << formatCOP(entry.second) << '\n';
        });
    const auto firstPaid = std::find_if(charged.begin(), charged.end(),
        [](const ChargedLeg& c) { return c.charge > 0; });
    std::cout << "  find_if: first leg that actually paid -> "
              << (firstPaid == charged.end() ? std::string("none") : firstPaid->modeName) << '\n';
    const std::function<bool(const TransportMode&)> bigModes =
        [](const TransportMode& mode) { return mode.stations() > 8; };
    std::cout << "  injected policy: modes with more than 8 stations -> "
              << joined(namesMatching(network, bigModes)) << '\n';
    std::cout << "  total for the student on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, student))) << '\n';
    std::cout << "  total for the senior on the same itinerary -> "
              << formatCOP(totalPaid(chargeItinerary(itinerary, senior))) << '\n';

    std::cout << "\n============================================================\n";
    std::cout << "  End of demonstration.\n";
    std::cout << "============================================================\n";
    return 0;
}
