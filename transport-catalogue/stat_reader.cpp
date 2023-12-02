#include "geo.h"
#include "stat_reader.h"
#include "input_reader.h"

#include <algorithm>
#include <deque>
#include <iostream>
#include <iomanip>
#include <unordered_set>

namespace transport_directory::stat_reader{

    using namespace std::literals;

    std::pair<std::string_view, std::string_view> ParseCommand(std::string_view str){
        auto trimed_str = input_reader::Trim(str);
        auto mid_space = trimed_str.find_first_of(' ');

        return {trimed_str.substr(0, mid_space), input_reader::Trim(trimed_str.substr(mid_space))};
    }


    double RootDistance(const std::list<Stop *>& stops)
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
        const std::list<Stop *>& stops)
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

    void PrintBusStat(const TransportCatalogue& transport_catalogue,
                      std::string_view name, std::ostream& output) {
        if (transport_catalogue.GetBus(name).empty()){
                output << "Bus"s << ' ' << name << ": not found"s << std::endl;
        } else {
            const auto& bus = transport_catalogue.GetBus(name);
            const double geo_dist = RootDistance(bus.stops);
            const int real_dist = RealDistance(transport_catalogue, bus.stops);
            std::unordered_set<Stop *> unique_stops{bus.stops.begin(), bus.stops.end()};
            output << std::setprecision(6);
            output << "Bus "s << bus.name << ": "s << bus.stops.size() << " stops on route, ";
            output << unique_stops.size() << " unique stops, "s << real_dist;
            output << " route length, "s << (real_dist / geo_dist)
                << " curvature"s << std::endl;
        }
    }


    void PrintStopStat(const TransportCatalogue& transport_catalogue,
                      std::string_view name, std::ostream& output) {

        if (transport_catalogue.GetStop(name).empty()){
                output << "Stop"s << ' ' << name << ": not found"s << std::endl;
        } else {
            const Stop& stop = transport_catalogue.GetStop(name);

            if (stop.buses.empty()){
                output << "Stop"s << ' ' << name << ": no buses"s << std::endl;
            } else {
                std::deque<Bus*> buses{stop.buses.begin(), stop.buses.end()};
                std::sort(buses.begin(), buses.end(), [](const Bus* lhs, const Bus* rhs){
                    return lhs -> name < rhs -> name;
                });
                output << "Stop"s << ' ' << name << ": buses "s;
                bool is_next = false;
                for (const Bus* bus : buses){
                    if (is_next){
                        output << ' ';
                    }
                    output << bus -> name;
                    is_next = true;
                }
                output << std::endl;
            }
        }
    }


    void ParseAndPrintStat(const TransportCatalogue& transport_catalogue, std::string_view request,
                           std::ostream& output) {
        const auto& [command, name] = ParseCommand(request);
        if (command == "Bus"s){
            PrintBusStat(transport_catalogue, name, output);
        } else if (command == "Stop"s){
            PrintStopStat(transport_catalogue, name, output);
        }
    }

}