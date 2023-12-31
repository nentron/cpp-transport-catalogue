#include "request_handler.h"

#include <algorithm>
#include <iomanip>
#include <map>
#include <sstream>


namespace request_handler {
    using namespace std::literals;


    const Bus* RequestHandler::GetBusByName(std::string_view name) const {
        return db_.GetBus(name);
    }

    const Stop* RequestHandler::GetStopByName(std::string_view name) const {
        return db_.GetStop(name);
    }

    const std::list<Stop*>& RequestHandler::GetBusStops(std::string_view name) const {
        return db_.GetBus(name) -> GetStops();
    }

    const std::unordered_set<Bus*>& RequestHandler::GetStopBuses(std::string_view name) const {
        return db_.GetStop(name) -> GetBuses();
    }

    int RequestHandler::GetRealDistance(const std::list<Stop*>& stops) const {
        return RealDistance(db_, stops);
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