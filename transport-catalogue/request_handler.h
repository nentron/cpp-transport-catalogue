#pragma once

#include "json_reader.h"
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

        json::Node BusRequest(const json::Dict& request) const;

        json::Node StopRequest(const json::Dict& request) const;

        json::Node MapRequest(const json::Dict& request) const;

        void ManageRequests(std::ostream& out, const json::Dict& requests) const;

        void MapRender(std::ostream& out) const;

    };
}

