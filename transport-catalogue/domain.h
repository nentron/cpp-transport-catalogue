#pragma once

#include "geo.h"

#include <list>
#include <string>
#include <unordered_set>


namespace domain {
    struct Bus;

    struct Stop {
        bool Empty() const;

        const std::unordered_set<Bus *>& GetBuses() const;

        std::string name;
        geo::Coordinates coordinates;
        std::unordered_set<Bus *> buses;
    };

    struct Bus {
        bool Empty() const;
        const std::list<Stop*>& GetStops() const;

        Bus() = default;
        Bus(const std::string& busname)
            : name(busname){}

        Bus(const std::string& busname, bool is_roundtrip)
            : name(busname)
            , is_roundtrip_(is_roundtrip){}

        std::string name;
        std::list<Stop*> stops;
        bool is_roundtrip_;
    };

}
