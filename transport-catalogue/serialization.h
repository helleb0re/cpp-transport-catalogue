#pragma once

#include "map_renderer.h"
#include "transport_router.h"
#include "json.h"
#include "json_builder.h"
#include "json_reader.h"

#include <transport_catalogue.pb.h>

#include <optional>
#include <fstream>
#include <unordered_map>
#include <map>
#include <stdexcept>

namespace transport_catalogue::serialization {

    class Serialization {
    public:
        Serialization(TransportCatalogue &db, RequestHandler &req_handler);

        void MakeBase(std::istream &input) const;
        void ProcessRequests(std::istream &input, std::ostream &out);
    private:
        void SetSerializationColor(transport_catalogue_serialize::Color& color_pb,svg::Color color) const;
        svg::Color DeserializationColor(const transport_catalogue_serialize::Color& color_pb) const;

        void SerializeBaseData(transport_catalogue_serialize::TransportCatalogue &tc_pb, const std::vector<json::Node>& base_requests) const;
        void DeserializeBaseData(const transport_catalogue_serialize::TransportCatalogue& tc_pb);

        void SerializeRenderSettings(transport_catalogue_serialize::RenderSettings &rs_pb, const json::Dict & render_settings) const;
        void DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings& rs_pb);

        void SerializeRoutingSettings(transport_catalogue_serialize::RoutingSettings &rs_pb, const json::Dict& routing_settings) const;
        void DeserializeRoutingSettings(const transport_catalogue_serialize::RoutingSettings& rs_pb);

        TransportCatalogue &db_;
        RequestHandler &req_handler_;
    };

} // namespace transport_catalogue::serialization