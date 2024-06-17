#include "transport_router.h"


bool operator<(const EdgeWeight& lhs, const EdgeWeight& rhs){
    return lhs.time_ < rhs.time_;
}

bool operator>(const EdgeWeight& lhs, const EdgeWeight& rhs){
    return lhs.time_ > rhs.time_;
}

EdgeWeight operator+(const EdgeWeight& lhs, const EdgeWeight& rhs){
    return EdgeWeight{lhs.time_ + rhs.time_};
}

void RouterHelper::LoadGraph(const transport_directory::TransportCatalogue& db){
    using Edge = graph::Edge<EdgeWeight>;

    static const double meters_per_min = (1000.0 / 60.0) * static_cast<double>(bus_velocity_);
    size_t index = 0;

    for (const auto& bus: db.GetAllBuses()){
        auto begin = bus.GetStops().begin();
        auto end = bus.GetStops().end();
        for (;begin != end; ++begin){
            const auto [idx, is_created_idx] = GetOrCreateIndex(index, (*begin) -> name);
            const auto [wait_idx, is_created_wait] = GetOrCreateWaitVertex(index, (*begin) -> name);
             if (is_created_idx || is_created_wait)
            {
                graph_.AddEdge(
                    Edge{idx, wait_idx, EdgeWeight{(*begin) -> name, static_cast<double>(bus_wait_time_)}.SetAction(
                            RoutesType::WAIT)
                    }
                );
            }

            {
                double time = 0.0;
                int span_counter = 0;
                auto first = begin;
                auto next = begin;
                for (++next; next != end; ++first, ++next){
                    const auto [idx_first, _] = GetOrCreateIndex(index, (*next) -> name);
                    time += db.GetDistance((*first), (*next)) / meters_per_min;
                    graph_.AddEdge(Edge{
                        wait_idx, idx_first, EdgeWeight{bus.name, time}.SetSpan(++span_counter)
                    });
                }
            }
        }
    }
}

std::optional<graph::VertexId> RouterHelper::GetVertexId(std::string_view name) const {
    if (stopname_to_vertex_id.count(name) == 0){
        return {};
    }
    return stopname_to_vertex_id.at(name);
}

std::pair<size_t, bool> RouterHelper::GetOrCreateIndex(size_t& index, std::string_view stopname){
    if (stopname_to_vertex_id.count(stopname) == 0){
        stopname_to_vertex_id[stopname] = index;
        return std::make_pair<size_t, bool>(index++, true);
    }
    return {stopname_to_vertex_id.at(stopname), false};
}
const graph::Edge<EdgeWeight>& RouterHelper::GetEdge(graph::EdgeId id) const {
    return graph_.GetEdge(id);
}

std::pair<size_t, bool> RouterHelper::GetOrCreateWaitVertex(size_t& index, std::string_view stopname){
    if (wait_stopname_to_index.count(stopname) == 0){
        wait_stopname_to_index[stopname] = index;
        return std::make_pair<size_t, bool>(index++, true);
    }
    return {wait_stopname_to_index.at(stopname), false};
}
