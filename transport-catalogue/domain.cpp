#include "domain.h"

namespace domain {
    bool Stop::Empty() const {
            return name.empty();
        }

    const auto& Stop::GetBuses() const {
        return buses;
    }

    bool Bus::Empty() const {
        return name.empty();
    }

    const auto& Bus::GetStops() const {
        return stops;
    }
}