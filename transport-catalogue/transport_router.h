#pragma once

#include "graph.h"
#include "router.h"
#include "transport_catalogue.h"

#include <optional>
#include <string_view>
#include <unordered_map>

struct RoutingSettings{
    int bus_wait_time;
    int bus_velocity;
};

enum class RoutesType{
    WAIT,
    BUS
};

struct EdgeWeight{
    std::string_view name_ = {};
    double time_ = {};
    RoutesType action_ = RoutesType::BUS;

    int span_counter = 0;

    EdgeWeight() = default;

    EdgeWeight(double time)
        : time_(time){}

    EdgeWeight(std::string_view name, double time)
        : name_(name)
        , time_(time){}

    EdgeWeight& SetAction(RoutesType action){
        action_ = action;
        return *this;
    }

    EdgeWeight& SetSpan(int i){
        span_counter = i;
        return *this;
    }
};

bool operator<(const EdgeWeight& lhs, const EdgeWeight& rhs);
bool operator>(const EdgeWeight& lhs, const EdgeWeight& rhs);

EdgeWeight operator+(const EdgeWeight& lhs, const EdgeWeight& rhs);

class RouterHelper{
private:
    int bus_wait_time_;
    int bus_velocity_;
    std::unordered_map<std::string_view, size_t> stopname_to_vertex_id;
    std::unordered_map<std::string_view, size_t> wait_stopname_to_index;
    graph::DirectedWeightedGraph<EdgeWeight> graph_;

    std::pair<size_t, bool> GetOrCreateIndex(size_t& index, std::string_view stopname);
    std::pair<size_t, bool> GetOrCreateWaitVertex(size_t& index, std::string_view stopname);

public:
    explicit RouterHelper(RoutingSettings settings, size_t graph_size)
        : bus_wait_time_(settings.bus_wait_time)
        , bus_velocity_(settings.bus_velocity)
        , graph_(graph_size * 2)
    {
        stopname_to_vertex_id.reserve(graph_size);
        wait_stopname_to_index.reserve(graph_size);
    }

    void LoadGraph(const transport_directory::TransportCatalogue& db);
    const graph::DirectedWeightedGraph<EdgeWeight>& GetGraph() const {
        return graph_;
    }

    std::optional<graph::VertexId> GetVertexId(std::string_view name) const;

    const graph::Edge<EdgeWeight>& GetEdge(graph::EdgeId id) const;

};
