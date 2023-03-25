#include "transport_router.h"
#include <iostream>

using namespace std;
using namespace graph;

namespace transport_catalogue
{
    namespace router
    {
        TransportRouter::TransportRouter(RoutingSettings &routing_settings) : routing_settings_(move(routing_settings)) {}

        void TransportRouter::SetOrUpdateRoutingSettings(RoutingSettings &settings)
        {
            routing_settings_ = move(settings);
        }

        bool TransportRouter::HasRoutingSettings() const
        {
            return routing_settings_.has_value();
        }

        size_t TransportRouter::GetVertexAmount() const
        {
            return vertex_amount_;
        }
        void TransportRouter::SetVertexAmount(size_t vertex_amount)
        {
            vertex_amount_ = vertex_amount;
            graph_ = DirectedWeightedGraph<double>(vertex_amount_);
        }

        void TransportRouter::FillDataToGraph(const deque<Bus> &buses,
                                              const DictStopsPairToDistances &distances_map)
        {
            for (const Bus &bus : buses)
            {
                AddBusToGraph(bus.name, bus.route.begin(), bus.route.end(), distances_map);

                if (!bus.is_roundtrip)
                {
                    AddBusToGraph(bus.name, bus.route.rbegin(), bus.route.rend(), distances_map);
                }
            }
            router_ptr_ = make_unique<Router<double>>(graph_);
        }

        optional<PathData> TransportRouter::GetShortWayBetween(Stop *start_stop, Stop *end_stop)
        {
            auto ans = router_ptr_->BuildRoute(start_stop->id, end_stop->id);

            if (!ans.has_value())
                return nullopt;

            PathData res;

            res.total_time = (*ans).weight;
            if (res.total_time == 0)
                return res;

            transform((*ans).edges.begin(), (*ans).edges.end(), back_inserter(res.items),
                      [&](EdgeId id)
                      {
                          return edge_id_to_path_data_.at(id);
                      });

            return res;
        }

    } // namespace router

} // namespace transport_catalogue
