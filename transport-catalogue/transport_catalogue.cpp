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

    void TransportCatalogue::AddRealDistance(std::string_view name,
            std::list<std::pair<std::string_view, int>> stop_dist){
            Stop* first = stopname_to_stop_.at(name);
            for (auto [to_stopname, dist] : stop_dist){
                Stop* second = stopname_to_stop_.at(to_stopname);
                real_distance_.insert({std::make_pair(first, second), dist});
            }
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

    int TransportCatalogue::GetDistance(Stop* const from, Stop* const to_stop) const {
        if (real_distance_.count(std::make_pair(from, to_stop)) == 0){
            return real_distance_.at({to_stop, from});
        }
        return real_distance_.at({from, to_stop});
    }

}