#include "json_reader.h"

using namespace std;

namespace transport_catalogue
{
    namespace iodata
    {
        using namespace json;
        JsonReader::JsonReader(TransportCatalogue &db, RequestHandler &req_handler, std::ostream &out)
            : db_(db), req_handler_(req_handler), out_(out) {}

        void JsonReader::LoadFile(istream &input)
        {
            doc_ = json::Load(input);

            Dict values = doc_.value().GetRoot().AsMap();

            if (values.count("base_requests"s) != 0 && !values.at("base_requests"s).AsArray().empty())
                InputDataBase();

            if (values.count("render_settings"s) != 0 && !values.at("render_settings"s).AsMap().empty())
                InputRenderSettings();

            if (values.count("stat_requests"s) != 0 && !values.at("stat_requests"s).AsArray().empty())
                OutputData();
        }

        const Document &JsonReader::GetJsonDocument() const
        {
            if (!doc_.has_value())
            {
                throw runtime_error("Json document wasn't uploaded"s);
            }
            return doc_.value();
        }

        void JsonReader::InputDataBase()
        {
            Array base_requests = doc_.value().GetRoot().AsMap().at("base_requests"s).AsArray();

            unordered_map<string_view, const Dict &> stop_to_stops_distances_queries;

            for (const auto &node : base_requests)
            {
                if (node.AsMap().at("type"s) != "Stop"s)
                    continue;

                const auto &stop_node = node.AsMap();

                const auto &stop_name = stop_node.at("name"s).AsString();

                db_.AddStop(stop_name,
                            {stop_node.at("latitude"s).AsDouble(),
                             stop_node.at("longitude"s).AsDouble()});

                stop_to_stops_distances_queries.insert({stop_name, stop_node.at("road_distances"s).AsMap()});
            }

            for (const auto &[stop_name, stops] : stop_to_stops_distances_queries)
            {
                vector<pair<string_view, double>> distances(stops.size());

                transform(stops.begin(), stops.end(), distances.begin(), [](const auto &stops_node)
                          { return pair{string_view(stops_node.first), stops_node.second.AsDouble()}; });

                db_.AddStopDistances(stop_name, distances);
            }

            for (const auto &node : base_requests)
            {
                if (node.AsMap().at("type"s) != "Bus"s)
                    continue;

                const auto &bus_node = node.AsMap();

                const auto &bus_name = bus_node.at("name"s).AsString();
                bool is_roundtrip = bus_node.at("is_roundtrip"s).AsBool();

                vector<string_view> stops(bus_node.at("stops"s).AsArray().size());

                transform(bus_node.at("stops"s).AsArray().begin(), bus_node.at("stops"s).AsArray().end(),
                          stops.begin(), [](const Node &stops_node)
                          { return string_view(stops_node.AsString()); });

                db_.AddBus(bus_name, stops, is_roundtrip);
            }
        }

        void JsonReader::InputRenderSettings()
        {
            Dict render_settings = doc_.value().GetRoot().AsMap().at("render_settings"s).AsMap();

            renderer::RenderSettings rs;

            rs.width = render_settings.at("width"s).AsDouble();
            rs.height = render_settings.at("height"s).AsDouble();

            rs.padding = render_settings.at("padding"s).AsDouble();

            rs.line_width = render_settings.at("line_width"s).AsDouble();
            rs.stop_radius = render_settings.at("stop_radius"s).AsDouble();

            rs.bus_label_font_size = render_settings.at("bus_label_font_size"s).AsInt();

            Array bus_label_offset_tmp = render_settings.at("bus_label_offset"s).AsArray();
            rs.bus_label_offset = {bus_label_offset_tmp[0].AsDouble(),
                                   bus_label_offset_tmp[1].AsDouble()};

            rs.stop_label_font_size = render_settings.at("stop_label_font_size"s).AsInt();

            Array stop_label_offset_tmp = render_settings.at("stop_label_offset"s).AsArray();
            rs.stop_label_offset = {stop_label_offset_tmp[0].AsDouble(),
                                    stop_label_offset_tmp[1].AsDouble()};

            rs.underlayer_color = ParseColor(render_settings.at("underlayer_color"s));

            rs.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();

            Array color_palette_tmp = render_settings.at("color_palette"s).AsArray();
            rs.color_palette.reserve(color_palette_tmp.size());

            for (const auto &node : color_palette_tmp)
                rs.color_palette.push_back(ParseColor(node));

            req_handler_.SetRenderSettings(rs);
        }

        void JsonReader::OutputData()
        {
            Array stat_requests = doc_.value().GetRoot().AsMap().at("stat_requests"s).AsArray();

            Array response;

            for (const auto &node : stat_requests)
            {
                const auto &type = node.AsMap().at("type"s);

                if (type == "Stop"s)
                {
                    const auto &stop_req_data = node.AsMap();

                    auto *res = req_handler_.GetBusesByStop(stop_req_data.at("name"s).AsString());

                    Dict stop_json_response{{"request_id"s, stop_req_data.at("id"s).AsInt()}};

                    if (res)
                    {
                        Array buses(res->size());
                        transform(res->begin(), res->end(), buses.begin(), [](const auto &bus_name)
                                  { return string(bus_name); });
                        sort(buses.begin(), buses.end(), [](const auto &node_a, const auto &node_b)
                             { return node_a.AsString() < node_b.AsString(); });
                        stop_json_response.insert({"buses"s, buses});
                    }
                    else
                    {
                        stop_json_response.insert({"error_message"s, "not found"s});
                    }

                    response.push_back({move(stop_json_response)});
                }
                else if (type == "Bus"s)
                {
                    const auto &bus_req_data = node.AsMap();

                    auto bus_info = req_handler_.GetBusStat(bus_req_data.at("name"s).AsString());

                    Dict bus_json_response{{"request_id"s, bus_req_data.at("id"s).AsInt()}};

                    if (bus_info.has_value())
                    {
                        bus_json_response.insert({"curvature"s, bus_info.value().curvature});
                        bus_json_response.insert({"route_length"s, bus_info.value().route_length});
                        bus_json_response.insert({"stop_count"s, bus_info.value().all_stops});
                        bus_json_response.insert({"unique_stop_count"s, bus_info.value().unique_stops});
                    }
                    else
                    {
                        bus_json_response.insert({"error_message"s, "not found"s});
                    }

                    response.push_back({move(bus_json_response)});
                }
                else if (type == "Map"s)
                {
                    const auto &map_req_data = node.AsMap();
                    Dict map_json_response{{"request_id"s, map_req_data.at("id"s).AsInt()}};

                    ostringstream output_map;
                    req_handler_.RenderMap().Render(output_map);

                    string map_str = output_map.str();
                    map_str.erase(map_str.size() - 1);
                    map_json_response.insert({"map"s, map_str});

                    response.push_back({move(map_json_response)});
                }
            }

            PrintValue(response, out_);
        }

        svg::Color JsonReader::ParseColor(const Node &node) const
        {
            if (node.IsString())
            {
                return node.AsString();
            }
            else
            {
                Array color_tmp = node.AsArray();
                if (color_tmp.size() == 3)
                {
                    return svg::Rgb{static_cast<uint8_t>(color_tmp[0].AsInt()),
                                    static_cast<uint8_t>(color_tmp[1].AsInt()),
                                    static_cast<uint8_t>(color_tmp[2].AsInt())};
                }
                else
                {
                    return svg::Rgba{static_cast<uint8_t>(color_tmp[0].AsInt()),
                                     static_cast<uint8_t>(color_tmp[1].AsInt()),
                                     static_cast<uint8_t>(color_tmp[2].AsInt()),
                                     color_tmp[3].AsDouble()};
                }
            }
        }

    } // namespace iodata

} // namespace transport_catalogue
