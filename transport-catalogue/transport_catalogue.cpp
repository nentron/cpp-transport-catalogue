#include "transport_catalogue.h"

namespace transport_directory{

        double RootDistance(const std::deque<Stop *>& stops)
    {
        double result = 0.0;
        Stop* previos_stop = nullptr;

        for (const auto& stop : stops){
            if (previos_stop == nullptr){
                previos_stop = stop;
                continue;
            }
            result += ComputeDistance(previos_stop -> coordinates, stop -> coordinates);
            previos_stop = stop;
        }

        return result;
    }

    int RealDistance(const TransportCatalogue& transport_catalogue,
        const std::deque<Stop *>& stops)
    {
        int result = 0;
        Stop* previos_stop = nullptr;

        for (const auto& stop : stops){
            if (previos_stop == nullptr){
                previos_stop = stop;
                continue;
            }
            result += transport_catalogue.GetDistance(
                previos_stop, stop
            );
            previos_stop = stop;
        }

        return result;
    }

    void TransportCatalogue::AddStop(const std::string& name, geo::Coordinates coordinates){
        Stop stop;
        stop.name = name;
        stop.coordinates = std::move(coordinates);
        stops_.push_back(std::move(stop));
        auto stop_pointer = &stops_.back();
        stopname_to_stop_[name] = std::move(stop_pointer);
    }

    void TransportCatalogue::AddBusBy(
        const std::string& name, Bus&& bus,
        const std::vector<std::string_view>& string_stops){
        buses_.push_back(bus);
        auto bus_pointer = &buses_.back();

        for (const auto& s : string_stops){
            Stop *stop = stopname_to_stop_.at(s);
            bus_pointer -> stops.push_back(stop);
            stop -> buses.insert(bus_pointer);
        }

        busname_to_root_[name] = std::move(bus_pointer);
    }

    void TransportCatalogue::AddBus(
        const std::string& name, const std::vector<std::string_view>& string_stops){

        AddBusBy(name, Bus{name}, string_stops);
    }

    void TransportCatalogue::AddBus(
        const std::string& name, bool is_roundtrip,
        const std::vector<std::string_view>& string_stops
    ){
        AddBusBy(name, Bus{name, is_roundtrip}, string_stops);
    }

    void TransportCatalogue::AddRealDistance(std::string_view from_stopname,
                                             int distance,
                                             std::string_view to_stopname)
    {
        Stop* first = stopname_to_stop_.at(from_stopname);
        Stop* second = stopname_to_stop_.at(to_stopname);
        real_distance_.insert({std::make_pair(first, second), distance});
    }

    const Stop* TransportCatalogue::GetStop(std::string_view name) const {
        static const Stop& empty_stop = Stop{};

        if (stopname_to_stop_.count(name) == 0){
            return &empty_stop;
        }

        return stopname_to_stop_.at(name);
    }

    const Bus* TransportCatalogue::GetBus(std::string_view name) const {
        static const Bus& empty_bus = Bus{};

        if (busname_to_root_.count(name) == 0){
            return &empty_bus;
        }

        return busname_to_root_.at(name);
    }

    int TransportCatalogue::GetDistance(Stop* const from, Stop* const to_stop) const {
        if (real_distance_.count(std::make_pair(from, to_stop)) != 0){
            return real_distance_.at({from, to_stop});
        }
        return real_distance_.at({to_stop, from});
    }

    const std::list<Bus>& TransportCatalogue::GetAllBuses() const {
        return buses_;
    }

    const std::list<Stop>& TransportCatalogue::GetAllStops() const {
        return stops_;
    }
}