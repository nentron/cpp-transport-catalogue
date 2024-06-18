#include "map_renderer.h"

namespace map_render {
    using namespace std::literals;

    inline const double EPSILON = 1e-6;
    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    class SphereProjector {
    public:
        template <typename PointInputIt>
        SphereProjector(PointInputIt points_begin, PointInputIt points_end,
                        double max_width, double max_height, double padding)
            : padding_(padding)
        {
            if (points_begin == points_end) {
                return;
            }

            const auto [left_it, right_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs -> coordinates.lng < rhs -> coordinates.lng; });
            min_lon_ = (*left_it) -> coordinates.lng;
            const double max_lon = (*right_it) -> coordinates.lng;

            const auto [bottom_it, top_it] = std::minmax_element(
                points_begin, points_end,
                [](auto lhs, auto rhs) { return lhs -> coordinates.lat < rhs -> coordinates.lat; });
            const double min_lat = (*bottom_it) ->coordinates.lat;
            max_lat_ = (*top_it) -> coordinates.lat;

            std::optional<double> width_zoom;
            if (!IsZero(max_lon - min_lon_)) {
                width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
            }

            std::optional<double> height_zoom;
            if (!IsZero(max_lat_ - min_lat)) {
                height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
            }

            if (width_zoom && height_zoom) {
                zoom_coeff_ = std::min(*width_zoom, *height_zoom);
            } else if (width_zoom) {
                zoom_coeff_ = *width_zoom;
            } else if (height_zoom) {
                zoom_coeff_ = *height_zoom;
            }
        }

        svg::Point operator()(geo::Coordinates coords) const {
            return {
                (coords.lng - min_lon_) * zoom_coeff_ + padding_,
                (max_lat_ - coords.lat) * zoom_coeff_ + padding_
            };
        }

    private:
        double padding_;
        double min_lon_ = 0;
        double max_lat_ = 0;
        double zoom_coeff_ = 0;
    };

    void RenderSVG::AddLines(
        svg::Document& doc, const std::deque<domain::Bus*>& buses,
        const SphereProjector& projector
    ) const {
        for (size_t i = 0; i < buses.size(); ++i){
            svg::Polyline lines;
            lines.SetFillColor(svg::NoneColor).SetStrokeWidth(settings_.line_width_)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND).SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);

            lines.SetStrokeColor(settings_.color_palette_[i % settings_.color_palette_.size()]);
            for (const auto stop : buses.at(i) -> stops){
                    auto point = projector(stop -> coordinates);
                    lines.AddPoint(point);
            }
            doc.Add(std::move(lines));
        }
    }

    void RenderSVG::AddBusesNames(
        svg::Document& doc, const std::deque<domain::Bus*> buses,
        const SphereProjector& projector) const
    {
        svg::Text text1, text2;
        text1.SetFontSize(
            static_cast<uint32_t>(settings_.bus_label_font_size_)
        );
        text1.SetOffset(settings_.bus_label_offset_);
        text1.SetFontFamily("Verdana"s);
        text1.SetFontWeight("bold"s);
        text2 = text1;
        text1.SetStrokeWidth(settings_.underlayer_width_);
        text1.SetStrokeColor(settings_.underlayer_color_);
        text1.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text1.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text1.SetFillColor(settings_.underlayer_color_);

        for (size_t i = 0; i < buses.size(); ++i){
            const auto& bus = buses.at(i);
            const auto& first = bus -> stops.begin();
            const auto coordinates = projector((*first) -> coordinates);

            text1.SetData(bus -> name);
            text1.SetPosition(coordinates);
            doc.Add(text1);

            text2.SetFillColor(settings_.color_palette_[i % settings_.color_palette_.size()]);
            text2.SetPosition(coordinates);
            text2.SetData(bus -> name);
            doc.Add(text2);

            if (!bus -> is_roundtrip_){
                const auto mid = std::next(first, static_cast<int>((bus -> stops.size() /2)));
                if ((*first) != (*mid)){
                    text1.SetPosition(projector((*mid) -> coordinates));
                    doc.Add(text1);

                    text2.SetPosition(projector((*mid) -> coordinates));
                    doc.Add(text2);
                }
            }
        }
    }


    void RenderSVG::AddStopsSymbols(
        svg::Document& doc, const std::deque<domain::Stop*> stops,
        const SphereProjector& projector
    ) const {
        svg::Circle circle;
        circle.SetRadius(settings_.stop_radius_);
        circle.SetFillColor("white"s);
        for (const auto& stop : stops){
            circle.SetCenter(projector(stop -> coordinates));
            doc.Add(circle);
        }
    }


    void RenderSVG::AddStopsName(
        svg::Document& doc, const std::deque<domain::Stop*> stops,
        const SphereProjector& projector
    ) const {
        svg::Text text1, text2;
        text1.SetFontSize(
            static_cast<uint32_t>(settings_.stop_label_font_size_)
        );
        text1.SetOffset(settings_.stop_label_offset_);
        text1.SetFontFamily("Verdana"s);
        text2 = text1;
        text1.SetStrokeWidth(settings_.underlayer_width_);
        text1.SetStrokeColor(settings_.underlayer_color_);
        text1.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
        text1.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
        text1.SetFillColor(settings_.underlayer_color_);

        text2.SetFillColor("black"s);

        for (const auto& stop : stops){
            const svg::Point coordinates = projector(stop -> coordinates);
            text1.SetData(stop -> name);
            text1.SetPosition(coordinates);
            doc.Add(text1);

            text2.SetData(stop -> name);
            text2.SetPosition(coordinates);
            doc.Add(text2);
        }
    }


    void RenderSVG::RenderMap(
        std::ostream& out,
        const std::deque<domain::Bus*>& buses,
        const std::deque<domain::Stop*>& stops)
    const {
        const SphereProjector projector{
            stops.begin(), stops.end(),
            settings_.width_, settings_.height_, settings_.padding_};

        svg::Document doc;

        AddLines(doc, buses, projector);

        AddBusesNames(doc, buses, projector);

        AddStopsSymbols(doc, stops, projector);

        AddStopsName(doc, stops, projector);

        doc.Render(out);
    }
}