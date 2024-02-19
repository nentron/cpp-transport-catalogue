#pragma once

#include "domain.h"
#include "geo.h"

#include <deque>
#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace transport_directory{

    using namespace domain;

    struct HashPairOfStops{
        size_t operator()(
            const std::pair<Stop *, Stop*>& stops) const {
            return hasher(stops.first) + 7 * hasher(stops.second);
        }

        std::hash<const void*> hasher;
    };

    class TransportCatalogue {
    private:
        std::list<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::list<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_root_;

        std::unordered_map<
            std::pair<Stop *, Stop *>,
            int, HashPairOfStops> real_distance_;

        void AddBusBy(const std::string& name, Bus&& bus, const std::vector<std::string_view>& string_stops);
    public:

        const std::list<Bus>& GetAllBuses() const;
        const std::list<Stop>& GetAllStops() const;

        void AddStop(const std::string& name, geo::Coordinates coordinates);

        void AddBus(const std::string& name,
                    const std::vector<std::string_view>& string_stops);

        void AddBus(const std::string& name, bool is_roundtrip,
                    const std::vector<std::string_view>& string_stops);
        void AddRealDistance(std::string_view from_stopname,
                             int distance,
                             std::string_view to_stopname);

        const Stop* GetStop(std::string_view name) const;

        const Bus* GetBus(std::string_view id) const;

        int GetDistance(Stop* const from, Stop* const to_stop) const;
    };

    double RootDistance(const std::deque<Stop *>& stops);

    int RealDistance(const TransportCatalogue& transport_catalogue,
        const std::deque<Stop *>& stops);
}