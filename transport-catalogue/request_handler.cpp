#include "request_handler.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>

namespace request_handler {
    using namespace std::literals;
    using namespace json;

    json::Node RequestHandler::BusRequest(const json::Dict& request) const {
        std::map<std::string, json::Node> temp;
        temp.emplace("request_id"s, json::Node{request.at("id"s).AsInt()});
        std::string_view name = request.at("name"s).AsString();
        if (db_.GetBus(name) -> Empty()){
            temp.emplace("error_message"s, json::Node{"not found"s});
        } else {
            const auto& bus = db_.GetBus(name);
            const double geo_dist = RootDistance(bus->GetStops());
            const int real_dist = RealDistance(db_, bus -> GetStops());
            std::unordered_set<Stop *> unique_stops{bus -> stops.begin(), bus -> stops.end()};

            temp.emplace("route_length"s, Node{real_dist});
            temp.emplace("stop_count"s, Node{static_cast<int>(bus -> stops.size())});
            temp.emplace("unique_stop_count"s, Node{static_cast<int>(unique_stops.size())});
            temp.emplace("curvature"s, Node{real_dist / geo_dist});
        }

        return std::move(json::Node{temp});
   }

   json::Node RequestHandler::StopRequest(const json::Dict& request) const {
        std::map<std::string, json::Node> temp;
        temp.emplace("request_id"s, json::Node{request.at("id"s).AsInt()});
        std::string_view name = request.at("name"s).AsString();
        if (db_.GetStop(name) -> Empty()){
            temp.emplace("error_message"s, json::Node{"not found"s});
        } else {
            const Stop* stop = db_.GetStop(name);
            Array buses;
            for (const auto bus : stop -> GetBuses()){
                buses.push_back(bus -> name);
            }

            std::sort(buses.begin(), buses.end(), [](const auto& lhs, const auto& rhs){
                return lhs.AsString() < rhs.AsString();
            });
            temp.emplace("buses"s, Node{buses});
        }

        return std::move(Node{temp});
    }

    json::Node RequestHandler::MapRequest(const json::Dict& request) const {
        json::Dict dict;
        dict.emplace("request_id"s, Node{request.at("id").AsInt()});
        std::ostringstream buffer;
        buffer.precision(6);
        MapRender(buffer);
        dict.emplace("map", Node{buffer.str()});
        return Node{dict};
    }

    void RequestHandler::ManageRequests(std::ostream& out, const Dict& requests) const {
        out << std::setprecision(6);
        Array temper;

        for (const auto& request : requests.at("stat_requests"s).AsArray()){
            const Dict& dict = request.AsMap();
            if (dict.at("type"s).AsString() == "Bus"s){
                temper.push_back(BusRequest(dict));
            } else if (dict.at("type"s).AsString() == "Stop"s) {
                temper.push_back(StopRequest(dict));
            } else if (dict.at("type"s).AsString() == "Map"s){
                temper.push_back(MapRequest(dict));
            }
        }

        PrintNode(Node{temper}, out);
   }


   void RequestHandler::MapRender(std::ostream& out) const {
        std::deque<Bus*> buses;
        for (const auto& bus : db_.GetAllBuses()){
            if (bus.stops.empty()){
                continue;
            }
            buses.push_back(const_cast<Bus*>(&bus));
        }
        std::sort(buses.begin(), buses.end(), [](const Bus* lhs, const Bus* rhs){
            return lhs -> name < rhs -> name;
        });

        std::deque<Stop*> stops;
        for (const auto& stop : db_.GetAllStops()){
            if (stop.buses.empty()){
                continue;
            }
            stops.push_back(const_cast<Stop*>(&stop));
        }

        std::sort(stops.begin(), stops.end(), [](const auto& lhs, const auto& rhs){
            return lhs -> name < rhs -> name;
        });

        render_.RenderMap(out, buses, stops);
   }
}