#pragma once

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "router.h"
#include "transport_router.h"


namespace request_handler {

    using namespace transport_directory;

    class RequestHandler final {
    private:
        const transport_directory::TransportCatalogue& db_;
        const map_render::RenderSVG& render_;
        const RouterHelper& helper_;
        const graph::Router<EdgeWeight> router_;
    public:
        explicit RequestHandler(
            const TransportCatalogue& db, const map_render::RenderSVG& render,
            const RouterHelper& helper
        )
            : db_(db)
            , render_(render)
            , helper_(helper)
            , router_(helper.GetGraph()){}

        const Stop* GetStopByName(std::string_view name) const;

        const Bus* GetBusByName(std::string_view name) const;

        const std::deque<Stop*>& GetBusStops(std::string_view name) const;

        const std::unordered_set<Bus*>& GetStopBuses(std::string_view name) const;

        int GetRealDistance(const std::deque<Stop*>& stops) const;

        void MapRender(std::ostream& out) const;

        std::optional<graph::Router<EdgeWeight>::RouteInfo> GetRoute(
            std::string_view from, std::string_view to
        ) const;

        const RouterHelper& GetHelper() const;
    };
}

