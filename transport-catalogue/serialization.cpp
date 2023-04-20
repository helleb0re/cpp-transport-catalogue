#include "serialization.h"

using namespace std;
using namespace transport_catalogue;
using namespace serialization;

Serialization::Serialization(TransportCatalogue &db, RequestHandler &req_handler) :
    db_(db), req_handler_(req_handler) {}

void Serialization::MakeBase(std::istream &input) const {
    json::Document doc = json::Load(input);
    const auto& values = doc.GetRoot().AsDict();

    string filename;
    if (values.count("serialization_settings"s) != 0 && !values.at("serialization_settings"s).AsDict().empty()) {
        filename = values.at("serialization_settings"s).AsDict().at("file"s).AsString();
    } else {
        throw std::logic_error("You have not specified a filename for serialization"s);
    }

    transport_catalogue_serialize::TransportCatalogue tc_pb;
    if (values.count("base_requests"s) != 0 && !values.at("base_requests"s).AsArray().empty()) {
        const auto& base_requests = values.at("base_requests"s).AsArray();
        SerializeBaseData(tc_pb, base_requests);
    }

    transport_catalogue_serialize::RenderSettings render_settings_pb;
    if (values.count("render_settings"s) != 0 && !values.at("render_settings"s).AsDict().empty()) {
        const auto& render_settings = values.at("render_settings"s).AsDict();
        SerializeRenderSettings(render_settings_pb, render_settings);
    }

    transport_catalogue_serialize::RoutingSettings routing_settings_pb;
    if (values.count("routing_settings"s) != 0 && !values.at("routing_settings"s).AsDict().empty()) {
        const auto& routing_settings = values.at("routing_settings"s).AsDict();
        SerializeRoutingSettings(routing_settings_pb, routing_settings);
    }

    transport_catalogue_serialize::DataBase db_pb;
    *db_pb.mutable_tc() = std::move(tc_pb);
    *db_pb.mutable_render_settings() = std::move(render_settings_pb);
    *db_pb.mutable_route_settings() = std::move(routing_settings_pb);

    ofstream out(filename, ios::binary);
    db_pb.SerializeToOstream(&out);
}

void Serialization::ProcessRequests(istream &input, ostream &out) {
    json::Document doc = json::Load(input);
    const auto& values = doc.GetRoot().AsDict();

    transport_catalogue_serialize::DataBase db_pb;
    if (values.count("serialization_settings"s) != 0 && !values.at("serialization_settings"s).AsDict().empty()) {
        string filename = values.at("serialization_settings"s).AsDict().at("file"s).AsString();

        ifstream inf(filename, ios::binary);
        db_pb.ParseFromIstream(&inf);
    } else {
        throw std::logic_error("You have not specified a filename for serialization"s);
    }

    DeserializeBaseData(db_pb.tc());
    DeserializeRenderSettings(db_pb.render_settings());
    DeserializeRoutingSettings(db_pb.route_settings());

    iodata::JsonReader json_reader(db_, req_handler_);
    json_reader.LoadFile(json::Document(values));
    json_reader.SaveResponseFile(out);
}

void Serialization::DeserializeBaseData(const transport_catalogue_serialize::TransportCatalogue& tc_pb) {
    unordered_map<string_view, const google::protobuf::Map<string, uint32_t> &> stop_to_stops_distances_queries;
    for (int i = 0; i < tc_pb.stops_size(); ++i) {
        const auto& stop_pb = tc_pb.stops(i);
        db_.AddStop(stop_pb.name(), {stop_pb.coords().lat(), stop_pb.coords().lng()});

        stop_to_stops_distances_queries.insert({stop_pb.name(), stop_pb.road_distances()});
    }

    for (const auto &[stop_name, stops] : stop_to_stops_distances_queries)
    {
        vector<pair<string_view, double>> distances(stops.size());

        transform(stops.begin(), stops.end(), distances.begin(),
                  [](const auto &stops_node)
                  { return pair{string_view(stops_node.first), stops_node.second}; });

        db_.AddStopDistances(stop_name, distances);
    }

    for (int i = 0; i < tc_pb.buses_size(); ++i)
    {
        const auto& bus_pb = tc_pb.buses(i);

        vector<string_view> stops(bus_pb.route_size());

        transform(bus_pb.route().begin(), bus_pb.route().end(),
                  stops.begin(), [](const string &stop_name)
                  { return string_view(stop_name); });

        db_.AddBus(bus_pb.name(), stops, bus_pb.is_roundtrip());
    }
}

void Serialization::DeserializeRenderSettings(const transport_catalogue_serialize::RenderSettings &rs_pb) {
    renderer::RenderSettings rs;

    rs.width = rs_pb.width();
    rs.height = rs_pb.height();

    rs.padding = rs_pb.padding();

    rs.line_width = rs_pb.line_width();
    rs.stop_radius = rs_pb.stop_radius();

    rs.bus_label_font_size = rs_pb.bus_label_font_size();

    rs.bus_label_offset = {
            rs_pb.bus_label_offset(0),
            rs_pb.bus_label_offset(1)
    };

    rs.stop_label_font_size = rs_pb.stop_label_font_size();

    rs.stop_label_offset = {
            rs_pb.stop_label_offset(0),
            rs_pb.stop_label_offset(1)
    };

    rs.underlayer_color = DeserializationColor(rs_pb.underlayer_color());

    rs.underlayer_width = rs_pb.underlayer_width();

    rs.color_palette.reserve(rs_pb.color_palette_size());
    for (int i = 0; i < rs_pb.color_palette_size(); ++i) {
        rs.color_palette.emplace_back(DeserializationColor(rs_pb.color_palette(i)));
    }

    req_handler_.SetRenderSettings(rs);
}

void Serialization::DeserializeRoutingSettings(const transport_catalogue_serialize::RoutingSettings &rs_pb) {
    router::RoutingSettings rs{};

    rs.bus_velocity = rs_pb.bus_velocity();
    rs.bus_wait_time = rs_pb.bus_wait_time();

    req_handler_.SetRoutingSettings(rs);
}

void Serialization::SerializeBaseData(transport_catalogue_serialize::TransportCatalogue &tc_pb,
                                      const std::vector<json::Node>& base_requests) const {
    for (const auto& base_request : base_requests) {
        const auto &dict = base_request.AsDict();
        if (dict.at("type"s) == "Stop"s) {
            transport_catalogue_serialize::Stop stop_pb;

            const auto &name = dict.at("name"s).AsString();
            stop_pb.set_name(name);

            const auto lat = dict.at("latitude").AsDouble();
            const auto lng = dict.at("longitude").AsDouble();
            transport_catalogue_serialize::Coordinates coords_pb;
            coords_pb.set_lat(lat);
            coords_pb.set_lng(lng);
            *stop_pb.mutable_coords() = std::move(coords_pb);

            const auto& road_distances = dict.at("road_distances"s).AsDict();
            for (const auto& [key, node_value]: road_distances) {
                const auto value = node_value.AsInt();
                (*stop_pb.mutable_road_distances())[key] = value;
            }

            *tc_pb.add_stops() = std::move(stop_pb);
        } else if (dict.at("type"s) == "Bus"s) {
            transport_catalogue_serialize::Bus bus_pb;

            const auto &name = dict.at("name"s).AsString();
            bus_pb.set_name(name);

            const auto is_roundtrip = dict.at("is_roundtrip"s).AsBool();
            bus_pb.set_is_roundtrip(is_roundtrip);

            const auto& stops = dict.at("stops"s).AsArray();
            for (const auto& node_stop: stops) {
                const auto& stop = node_stop.AsString();
                bus_pb.add_route(stop);
            }

            *tc_pb.add_buses() = std::move(bus_pb);
        }
    }
}

void Serialization::SerializeRenderSettings(transport_catalogue_serialize::RenderSettings &rs_pb,
                                            const json::Dict &render_settings) const {
    rs_pb.set_width(render_settings.at("width"s).AsDouble());
    rs_pb.set_height(render_settings.at("height"s).AsDouble());

    rs_pb.set_padding(render_settings.at("padding"s).AsDouble());

    rs_pb.set_line_width(render_settings.at("line_width"s).AsDouble());
    rs_pb.set_stop_radius(render_settings.at("stop_radius"s).AsDouble());

    rs_pb.set_bus_label_font_size(render_settings.at("bus_label_font_size"s).AsInt());

    const auto& bus_label_offset_tmp = render_settings.at("bus_label_offset"s).AsArray();
    rs_pb.add_bus_label_offset(bus_label_offset_tmp[0].AsDouble());
    rs_pb.add_bus_label_offset(bus_label_offset_tmp[1].AsDouble());

    rs_pb.set_stop_label_font_size(render_settings.at("stop_label_font_size"s).AsInt());

    const auto& stop_label_offset_tmp = render_settings.at("stop_label_offset"s).AsArray();
    rs_pb.add_stop_label_offset(stop_label_offset_tmp[0].AsDouble());
    rs_pb.add_stop_label_offset(stop_label_offset_tmp[1].AsDouble());

    svg::Color underlayer_color = detail::ParseColor(render_settings.at("underlayer_color"s));
    transport_catalogue_serialize::Color underlayer_color_pb;
    SetSerializationColor(underlayer_color_pb, underlayer_color);
    *rs_pb.mutable_underlayer_color() = std::move(underlayer_color_pb);

    rs_pb.set_underlayer_width(render_settings.at("underlayer_width"s).AsDouble());

    const auto& color_palette_tmp = render_settings.at("color_palette"s).AsArray();
    for (const auto &node : color_palette_tmp) {
        svg::Color color = detail::ParseColor(node);
        transport_catalogue_serialize::Color color_pb;
        SetSerializationColor(color_pb, color);
        *rs_pb.add_color_palette() = std::move(color_pb);
    }
}

void Serialization::SerializeRoutingSettings(transport_catalogue_serialize::RoutingSettings &rs_pb,
                                             const json::Dict &routing_settings) const {
    rs_pb.set_bus_velocity(routing_settings.at("bus_velocity"s).AsInt());
    rs_pb.set_bus_wait_time(routing_settings.at("bus_wait_time"s).AsInt());
}

void Serialization::SetSerializationColor(transport_catalogue_serialize::Color &color_pb,const svg::Color color) const {
    switch (color.index()) {
        case 0:
            break;
        case 1:
            color_pb.set_color_str(get<string>(color));
            break;
        case 2:
            color_pb.add_color_arr(get<svg::Rgb>(color).red);
            color_pb.add_color_arr(get<svg::Rgb>(color).green);
            color_pb.add_color_arr(get<svg::Rgb>(color).blue);
            color_pb.set_opacity(1);
            break;
        case 3:
            color_pb.add_color_arr(get<svg::Rgba>(color).red);
            color_pb.add_color_arr(get<svg::Rgba>(color).green);
            color_pb.add_color_arr(get<svg::Rgba>(color).blue);
            color_pb.set_opacity(get<svg::Rgba>(color).opacity);
            break;
        default:
            break;
    }
}

svg::Color Serialization::DeserializationColor(const transport_catalogue_serialize::Color& color_pb) const {
    if (color_pb.color_str() != ""s) {
        return {color_pb.color_str()};
    } else {
        if (color_pb.opacity() == 1) {
            return svg::Rgb{
                    static_cast<uint8_t>(color_pb.color_arr(0)),
                    static_cast<uint8_t>(color_pb.color_arr(1)),
                    static_cast<uint8_t>(color_pb.color_arr(2)),
            };
        } else {
            return svg::Rgba{
                    static_cast<uint8_t>(color_pb.color_arr(0)),
                    static_cast<uint8_t>(color_pb.color_arr(1)),
                    static_cast<uint8_t>(color_pb.color_arr(2)),
                    color_pb.opacity()
            };
        }
    }
}