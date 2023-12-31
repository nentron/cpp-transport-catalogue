#include "json.h"
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
            if (request.AsMap().at("type"s).AsString() == "Stop"s){
                const auto& request_dict = request.AsMap();
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
            const auto& dict = request.AsMap();
            if (dict.at("type"s).AsString() == "Stop"s){
                for (const auto& [name, dist] : dict.at("road_distances"s).AsMap()){
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

        const json::Dict& dict = doc_.GetRoot().AsMap();
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

        const json::Dict& dict = doc_.GetRoot().AsMap().at("render_settings").AsMap();

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


    json::Node JSONReader::BusRequest(
        const json::Dict& request,
        const request_handler::RequestHandler& handler) const {

        std::map<std::string, json::Node> temp;
        temp.emplace("request_id"s, json::Node{request.at("id"s).AsInt()});
        std::string_view name = request.at("name"s).AsString();
        if (handler.GetBusByName(name) -> Empty()){
            temp.emplace("error_message"s, json::Node{"not found"s});
        } else {
            const auto& bus = handler.GetBusByName(name);
            const double geo_dist = RootDistance(bus -> GetStops());
            const int real_dist = handler.GetRealDistance(bus -> GetStops());
            std::unordered_set<Stop *> unique_stops{bus -> stops.begin(), bus -> stops.end()};

            temp.emplace("route_length"s, Node{real_dist});
            temp.emplace("stop_count"s, Node{static_cast<int>(bus -> stops.size())});
            temp.emplace("unique_stop_count"s, Node{static_cast<int>(unique_stops.size())});
            temp.emplace("curvature"s, Node{real_dist / geo_dist});
        }

        return std::move(json::Node{temp});
   }

   json::Node JSONReader::StopRequest(
        const json::Dict& request,
        const request_handler::RequestHandler& handler) const
    {
        std::map<std::string, json::Node> temp;
        temp.emplace("request_id"s, json::Node{request.at("id"s).AsInt()});
        std::string_view name = request.at("name"s).AsString();
        if (handler.GetStopByName(name) -> Empty()){
            temp.emplace("error_message"s, json::Node{"not found"s});
        } else {
            const Stop* stop = handler.GetStopByName(name);
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

    json::Node JSONReader::MapRequest(
        const json::Dict& request,
        const request_handler::RequestHandler& handler) const
    {
        json::Dict dict;
        dict.emplace("request_id"s, Node{request.at("id").AsInt()});
        std::ostringstream buffer;
        buffer.precision(6);
        handler.MapRender(buffer);
        dict.emplace("map", Node{buffer.str()});
        return Node{dict};
    }


    void JSONReader::ManageRequests(std::ostream& out, const request_handler::RequestHandler& handler) const {
        Array temper;

        for (const auto& request : doc_.GetRoot().AsMap().at("stat_requests"s).AsArray()){
            const Dict& dict = request.AsMap();
            if (dict.at("type"s).AsString() == "Bus"s){
                temper.push_back(BusRequest(dict, handler));
            } else if (dict.at("type"s).AsString() == "Stop"s) {
                temper.push_back(StopRequest(dict, handler));
            } else if (dict.at("type"s).AsString() == "Map"s){
                temper.push_back(MapRequest(dict, handler));
            }
        }

        PrintNode(Node{temper}, out);
   }
}