#pragma once

#include "json.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "transport_catalogue.h"


namespace json_reader{
    using namespace transport_directory;
    class Reader {
    public:
        virtual ~Reader() = default;
        virtual void Read(std::istream& input) = 0;
        virtual TransportCatalogue GetDB() const = 0;
        const json::Document& GetDocument() const;
        virtual map_render::RenderSettings GetRenderSettings() const = 0;
    protected:
        json::Document doc_;
    };


    class JSONReader : public Reader{
    public:
        JSONReader() = default;
        void Read(std::istream& input) override;

        TransportCatalogue GetDB() const override;
        map_render::RenderSettings GetRenderSettings() const override;

        json::Node BusRequest(const json::Dict& request, const request_handler::RequestHandler& handler) const;

        json::Node StopRequest(const json::Dict& request, const request_handler::RequestHandler& handler) const;

        json::Node MapRequest(const json::Dict& request, const request_handler::RequestHandler& handler) const;

        void ManageRequests(std::ostream& out, const request_handler::RequestHandler& handler) const;
    };
}