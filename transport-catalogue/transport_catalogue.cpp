#include "transport_catalogue.h"

namespace transport_directory{
    void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates){
        Stop stop;
        stop.name = name;
        stop.coordinates = std::move(coordinates);
        stops_.push_back(std::move(stop));
        auto* stop_pointer = &stops_.back();
        stopname_to_stop_[stop_pointer -> name] = stop_pointer;
    }

    void TransportCatalogue::AddBus(
        const std::string& name, const std::vector<std::string_view>& string_stops){

        Bus bus;
        buses_.push_back(std::move(bus));
        Bus* bus_pointer = &buses_.back();
        bus_pointer -> name = name;

        for (const auto& s : string_stops){
            Stop *stop = stopname_to_stop_.at(s);
            bus_pointer -> stops.push_back(stop);
            stop -> buses.insert(bus_pointer);
        }

        busname_to_root_[bus_pointer -> name] = bus_pointer;
    }

    const Stop& TransportCatalogue::GetStop(std::string_view name) const {
        static const Stop& empty_stop = Stop{};

        if (stopname_to_stop_.count(name) == 0){
            return empty_stop;
        }

        return *stopname_to_stop_.at(name);
    }

    const Bus& TransportCatalogue::GetBus(std::string_view id) const {
        static const Bus& empty_bus = Bus{};

        if (busname_to_root_.count(id) == 0){
            return empty_bus;
        }

        return *busname_to_root_.at(id);
    }

}