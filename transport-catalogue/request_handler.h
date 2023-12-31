#pragma once


#include "transport_catalogue.h"
#include "map_renderer.h"


namespace request_handler {

    using namespace transport_directory;

    class RequestHandler final {
    private:
        const transport_directory::TransportCatalogue db_;
        const map_render::RenderSVG render_;
    public:
        explicit RequestHandler(TransportCatalogue&& db, map_render::RenderSVG&& render)
            : db_(std::move(db))
            , render_(std::move(render)){}

        const Stop* GetStopByName(std::string_view name) const;

        const Bus* GetBusByName(std::string_view name) const;

        const std::list<Stop*>& GetBusStops(std::string_view name) const;

        const std::unordered_set<Bus*>& GetStopBuses(std::string_view name) const;

        int GetRealDistance(const std::list<Stop*>& stops) const;

        void MapRender(std::ostream& out) const;

    };
}

