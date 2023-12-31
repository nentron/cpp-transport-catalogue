#include "domain.h"

namespace domain {
    bool Stop::Empty() const {
            return name.empty();
        }

    const std::unordered_set<Bus*>& Stop::GetBuses() const {
        return buses;
    }

    bool Bus::Empty() const {
        return name.empty();
    }

    const std::list<Stop*>& Bus::GetStops() const {
        return stops;
    }
}