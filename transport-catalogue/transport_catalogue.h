#pragma once

#include "geo.h"

#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace transport_directory{

    struct Bus;

    struct Stop {

        explicit operator bool() const {
            return !name.empty();
        }

        bool operator!() const {
            return !operator bool();
        }
        std::string name;
        geo::Coordinates coordinates;
        std::unordered_set<Bus *> buses;
    };

    struct Bus {

        explicit operator bool() const {
            return !id.empty();
        }

        bool operator!() const {
            return !operator bool();
        }

        std::string id;
        std::list<Stop*> stops;
    };

    class TransportCatalogue {
    private:
        std::list<Stop> stops_;
        std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
        std::list<Bus> buses_;
        std::unordered_map<std::string_view, Bus*> busname_to_root_;
    public:
        void AddStop(std::string name, geo::Coordinates coordinates);

        void AddBus(std::string id, std::vector<std::string_view> stops);

        const Stop& GetStop(std::string_view name) const;

        const Bus& GetBus(std::string_view id) const;

    };

}