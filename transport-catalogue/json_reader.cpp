#include "json_reader.h"

#include <sstream>


namespace json_reader {

    using namespace std::literals;
    using namespace json;


    const json::Document& Reader::GetDocument() const {
        return doc_;
    }


    void JSONReader::Read(std::istream& input) {
        doc_ = std::move(json::Load(input));
    }


    void LoadStops(TransportCatalogue& db, const json::Array& data){
        for (const auto& request : data){
            if (request.AsDict().at("type"s).AsString() == "Stop"s){
                const auto& request_dict = request.AsDict();
                db.AddStop(
                    request_dict.at("name"s).AsString(),
                    std::move(geo::Coordinates{
                        request_dict.at("latitude"s).AsDouble(),
                        request_dict.at("longitude"s).AsDouble()
                    })
                );
            }
        }
    }


    void LoadBuses(TransportCatalogue& db, const json::Array& data){
        for (const auto& request : data){
            const auto& dict = request.AsDict();
            if (dict.at("type"s).AsString() == "Stop"s){
                for (const auto& [name, dist] : dict.at("road_distances"s).AsDict()){
                    db.AddRealDistance(
                        dict.at("name"s).AsString(),
                        dist.AsInt(),
                        name
                    );
                }
            } else if (dict.at("type"s).AsString() == "Bus"s){
                std::vector<std::string_view> stops;
                const json::Array& stops_source = dict.at("stops"s).AsArray();
                for (const auto& stop : stops_source){
                    stops.push_back(stop.AsString());
                }
                bool is_roundtrip = dict.at("is_roundtrip"s).AsBool();
                if (!is_roundtrip){
                    for (auto iter = stops_source.crbegin(); iter != stops_source.crend(); ++iter){
                        if (iter == stops_source.crbegin()){
                            continue;
                        }
                        stops.push_back((*iter).AsString());
                    }
                }
                db.AddBus(
                    dict.at("name"s).AsString(),
                    is_roundtrip,
                    stops
                );
            }
        }
    }


    TransportCatalogue JSONReader::GetDB() const {
        TransportCatalogue db;

        const json::Dict& dict = doc_.GetRoot().AsDict();
        if (dict.count("base_requests"s) != 0){
            LoadStops(db, dict.at("base_requests"s).AsArray());
            LoadBuses(db, dict.at("base_requests"s).AsArray());
        } else {
            throw std::invalid_argument("No base requests"s);
        }

        return db;
    }

    svg::Color GetColor(const json::Node& node){
        if (node.IsString()){
            return node.AsString();
        } else {
            const json::Array& arr = node.AsArray();
            if (arr.size() == 3){
                return std::move(svg::Rgb(
                    static_cast<uint8_t>(arr.at(0).AsInt()),
                    static_cast<uint8_t>(arr.at(1).AsInt()),
                    static_cast<uint8_t>(arr.at(2).AsInt())
                ));
            } else {
                return std::move(svg::Rgba(
                    static_cast<uint8_t>(arr.at(0).AsInt()),
                    static_cast<uint8_t>(arr.at(1).AsInt()),
                    static_cast<uint8_t>(arr.at(2).AsInt()),
                    arr.at(3).AsDouble()
                ));
            }
        }
    }

    map_render::RenderSettings JSONReader::GetRenderSettings() const {
        map_render::RenderSettings settings;

        const json::Dict& dict = doc_.GetRoot().AsDict().at("render_settings").AsDict();

        settings.width_ = dict.at("width"s).AsDouble();
        settings.height_ = dict.at("height"s).AsDouble();

        settings.padding_ = dict.at("padding"s).AsDouble();

        settings.line_width_ = dict.at("line_width"s).AsDouble();
        settings.stop_radius_ = dict.at("stop_radius"s).AsDouble();

        settings.bus_label_font_size_ = dict.at("bus_label_font_size"s).AsInt();
        settings.bus_label_offset_ = svg::Point{
            dict.at("bus_label_offset"s).AsArray().at(0).AsDouble(),
            dict.at("bus_label_offset"s).AsArray().at(1).AsDouble()
        };

        settings.stop_label_font_size_ = dict.at("stop_label_font_size"s).AsInt();
        settings.stop_label_offset_ = svg::Point{
            dict.at("stop_label_offset"s).AsArray().at(0).AsDouble(),
            dict.at("stop_label_offset"s).AsArray().at(1).AsDouble()
        };

        settings.underlayer_color_ = std::move(GetColor(dict.at("underlayer_color"s)));
        settings.underlayer_width_ = dict.at("underlayer_width"s).AsDouble();

        for (const auto& color : dict.at("color_palette"s).AsArray()){
            settings.color_palette_.push_back(std::move(GetColor(color)));
        }

        return settings;
    }


    void JSONReader::BusRequest(
        json::Builder& builder,
        const json::Dict& request,
        const request_handler::RequestHandler& handler) const {
        
        builder.StartDict();
        builder.Key("request_id"s).Value(request.at("id"s).AsInt()); 
        std::string_view name = request.at("name"s).AsString();
        if (handler.GetBusByName(name) -> Empty()){
            builder.Key("error_message"s).Value("not found"s);
        } else {
            const auto& bus = handler.GetBusByName(name);
            const double geo_dist = RootDistance(bus -> GetStops());
            const int real_dist = handler.GetRealDistance(bus -> GetStops());
            std::unordered_set<Stop *> unique_stops{bus -> stops.begin(), bus -> stops.end()};

            builder.Key("route_length"s).Value(real_dist);
            builder.Key("stop_count"s).Value(static_cast<int>(bus -> stops.size()));
            builder.Key("unique_stop_count"s).Value(static_cast<int>(unique_stops.size()));
            builder.Key("curvature"s).Value(real_dist / geo_dist);
        }
        builder.EndDict();
   }

   void JSONReader::StopRequest(
        json::Builder& builder,
        const json::Dict& request,
        const request_handler::RequestHandler& handler) const
    {
        builder.StartDict();
        builder.Key("request_id"s).Value(request.at("id"s).AsInt());
        std::string_view name = request.at("name"s).AsString();
        if (handler.GetStopByName(name) -> Empty()){
            builder.Key("error_message"s).Value("not found"s);
        } else {
            const Stop* stop = handler.GetStopByName(name);
            Array buses;
            for (const auto bus : stop -> GetBuses()){
                buses.push_back(bus -> name);
            }

            std::sort(buses.begin(), buses.end(), [](const auto& lhs, const auto& rhs){
                return lhs.AsString() < rhs.AsString();
            });

            builder.Key("buses"s).Value(buses);
        }
        builder.EndDict();
    }

    void JSONReader::MapRequest(
        json::Builder& builder,
        const json::Dict& request,
        const request_handler::RequestHandler& handler) const
    {
        builder.StartDict();
        builder.Key("request_id"s).Value(request.at("id"s).AsInt());
        std::ostringstream buffer;
        buffer.precision(6);
        handler.MapRender(buffer);
        builder.Key("map"s).Value(buffer.str());
        builder.EndDict();
    }

    void JSONReader::RouteRequest(
            json::Builder& builder,
            const json::Dict& request,
            const request_handler::RequestHandler& handler
        ) const
    {
        builder.StartDict();
        builder.Key("request_id"s).Value(request.at("id"s).AsInt());
        const auto route = handler.GetRoute(
            request.at("from"s).AsString(),
            request.at("to"s).AsString()
        );

        if (!route){
            builder.Key("error_message"s).Value("not found"s);
        } else {
            builder.Key("total_time"s).Value(route -> weight.time_);
            builder.Key("items"s).StartArray();
            for (auto edge_id : route -> edges){
                const auto& edge = handler.GetHelper().GetEdge(edge_id).weight;
                builder.StartDict();
                builder.Key("type"s);
                if (edge.action_ == RoutesType::BUS){
                    builder.Value("Bus"s);
                    builder.Key("bus"s).Value(std::string(edge.name_));
                    builder.Key("span_count"s).Value(edge.span_counter);
                } else {
                    builder.Value("Wait"s);
                    builder.Key("stop_name"s).Value(std::string(edge.name_));
                }

                builder.Key("time"s).Value(edge.time_);
                builder.EndDict();
            }
            builder.EndArray();
        }

        builder.EndDict();
    }

    void JSONReader::ManageRequests(std::ostream& out, const request_handler::RequestHandler& handler) const {

        json::Builder builder = json::Builder{};
        builder.StartArray();

        for (const auto& request : doc_.GetRoot().AsDict().at("stat_requests"s).AsArray()){
            const Dict& dict = request.AsDict();
            if (dict.at("type"s).AsString() == "Bus"s){
                BusRequest(builder, dict, handler);
            } else if (dict.at("type"s).AsString() == "Stop"s) {
                StopRequest(builder, dict, handler);
            } else if (dict.at("type"s).AsString() == "Map"s){
                MapRequest(builder, dict, handler);
            } else if (dict.at("type"s).AsString() == "Route"s){
                RouteRequest(builder, dict, handler);
            }
        }
        builder.EndArray();
        Print(Document{builder.Build()}, out);
   }

   RoutingSettings JSONReader::GetRoutingSettings() const {
        RoutingSettings settings;
        const auto& dict_settings = doc_.GetRoot().AsDict().at("routing_settings"s).AsDict();
        settings.bus_wait_time = dict_settings.at("bus_wait_time"s).AsInt();
        settings.bus_velocity = dict_settings.at("bus_velocity"s).AsInt();
        return settings;
   }
}