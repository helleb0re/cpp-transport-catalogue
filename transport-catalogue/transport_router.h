#pragma once

#include <optional>
#include <unordered_map>
#include <memory>
#include <deque>

#include "domain.h"
#include "router.h"
#include "transport_catalogue.h"

constexpr double METERS_IN_KM = 1000;
constexpr double MINUTES_IN_HOUR = 60;
namespace transport_catalogue
{
    namespace router
    {
        struct RoutingSettings
        {
            double bus_velocity;
            double bus_wait_time;
        };

        class TransportRouter
        {
        public:
            TransportRouter() = default;

            explicit TransportRouter(RoutingSettings &routing_settings);

            void SetOrUpdateRoutingSettings(RoutingSettings &settings);

            bool HasRoutingSettings() const;

            void FillDataToGraph(const std::deque<Bus> &buses,
                                 const DictStopsPairToDistances &distances_map);
            std::optional<PathData> GetShortWayBetween(Stop *start_stop, Stop *end_stop);

            size_t GetVertexAmount() const;
            void SetVertexAmount(size_t vertex_amount);

        private:
            std::optional<RoutingSettings> routing_settings_;

            size_t vertex_amount_ = 0;
            graph::DirectedWeightedGraph<double> graph_;
            std::unique_ptr<graph::Router<double>> router_ptr_;

            std::unordered_map<graph::EdgeId, PathDataItem> edge_id_to_path_data_;

            template <typename It>
            void AddBusToGraph(std::string_view bus_name,
                               It b_stops, It e_stops,
                               const DictStopsPairToDistances &distances_map)
            {
                const double bus_multiplier = 1.0 / METERS_IN_KM / (*routing_settings_).bus_velocity * MINUTES_IN_HOUR;
                const double wait_multiplier = (*routing_settings_).bus_wait_time;

                std::deque<double> weights;
                std::deque<graph::VertexId> after_waits_vertex_id;
                for (auto it_stop = b_stops, it_next_stop = next(b_stops);
                     it_next_stop != e_stops;
                     ++it_stop, ++it_next_stop)
                {
                    edge_id_to_path_data_.emplace(
                        graph_.AddEdge({(*it_stop)->id, (*it_stop)->id + 1,
                                        wait_multiplier}),
                        PathDataItemWait{(*it_stop)->name, wait_multiplier});

                    after_waits_vertex_id.push_back((*it_stop)->id + 1);

                    double weight = 0;
                    if (distances_map.count({*it_stop, *it_next_stop}) > 0)
                        weight = distances_map.at({*it_stop, *it_next_stop}) * bus_multiplier;
                    else if (distances_map.count({*it_next_stop, *it_stop}) > 0)
                        weight = distances_map.at({*it_next_stop, *it_stop}) * bus_multiplier;
                    weights.push_back(weight);

                    int after_waits_vertex_size = after_waits_vertex_id.size();
                    for (int i = after_waits_vertex_size - 1; i >= 0; --i)
                    {
                        if (after_waits_vertex_size - i > 1)
                        {
                            weights.push_back(weights[weights.size() - (after_waits_vertex_size - i - 1)] +
                                              weights[weights.size() - after_waits_vertex_size]);
                        }

                        edge_id_to_path_data_.emplace(
                            graph_.AddEdge({after_waits_vertex_id[i],
                                            (*it_next_stop)->id,
                                            weights.back()}),
                            PathDataItemBus{bus_name, after_waits_vertex_size - i, weights.back()});
                    }
                }
            }
        };
    } // namespace router

} // namespace transport_catalogue
