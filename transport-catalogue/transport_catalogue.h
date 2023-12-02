#pragma once

#include "geo.h"

#include <functional>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace transport_directory{

    struct Bus;

    struct Stop {
        bool empty() const {
            return name.empty();
        }
        std::string name;
        geo::Coordinates coordinates;
        std::unordered_set<Bus *> buses;
    };

    struct Bus {
        bool empty() const {
            return name.empty();
        }
        std::string name;
        std::list<Stop*> stops;
    };


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
    public:

        void AddStop(const std::string& name, geo::Coordinates coordinates);

        void AddBus(const std::string& name,
                    const std::vector<std::string_view>& string_stops);

        void AddRealDistance(std::string_view name,
            std::list<std::pair<std::string_view, int>> stop_dist);

        const Stop& GetStop(std::string_view name) const;

        const Bus& GetBus(std::string_view id) const;

        int GetDistance(Stop* const from, Stop* const to_stop) const;
    };

}