#pragma once

#include "domain.h"
#include "geo.h"
#include "svg.h"

#include <deque>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

namespace map_render {

class SphereProjector;

struct RenderSettings{
    RenderSettings() = default;
    double width_;
    double height_;

    double padding_;

    double line_width_;
    double stop_radius_;

    int bus_label_font_size_;
    svg::Point bus_label_offset_;

    int stop_label_font_size_;
    svg::Point stop_label_offset_;

    svg::Color underlayer_color_;
    double underlayer_width_;

    std::deque<svg::Color> color_palette_;
};


class RenderSVG{
private:
    const RenderSettings settings_;

    void AddLines(
        svg::Document& doc, const std::deque<domain::Bus*>& buses,
        const SphereProjector& projector
    ) const;

    void AddBusesNames(
        svg::Document& doc, const std::deque<domain::Bus*> buses,
        const SphereProjector& projector
    ) const;

    void AddStopsSymbols(
        svg::Document& doc, const std::deque<domain::Stop*> buses,
        const SphereProjector& projector
    ) const;

    void AddStopsName(
        svg::Document& doc, const std::deque<domain::Stop*> buses,
        const SphereProjector& projector
    ) const;

public:
    explicit RenderSVG(RenderSettings&& settings)
        : settings_(std::move(settings)){}

    void RenderMap(
        std::ostream& out,
        const std::deque<domain::Bus*>& buses,
        const std::deque<domain::Stop*>& stops
    ) const;
};

}