#include "json_reader.h"

using namespace std;

namespace transport_catalogue
{

    namespace iodata
    {
        using namespace json;
        JsonReader::JsonReader(TransportCatalogue &db, RequestHandler &req_handler)
            : db_(db), req_handler_(req_handler) {}

        void JsonReader::LoadFile(istream &input)
        {
            doc_ = json::Load(input);
        }

        void JsonReader::LoadFile(const json::Document& document) {
            doc_ = document;
        }

        void JsonReader::InputData() {
            Dict values = doc_.value().GetRoot().AsDict();

            if (values.count("base_requests"s) != 0 && !values.at("base_requests"s).AsArray().empty())
                InputDataBase();

            if (values.count("render_settings"s) != 0 && !values.at("render_settings"s).AsDict().empty())
                InputRenderSettings();

            if (values.count("routing_settings"s) != 0 && !values.at("routing_settings"s).AsDict().empty())
                InputRoutingSettings();
        }

        void JsonReader::SaveResponseFile(std::ostream &out) {
            Dict values = doc_.value().GetRoot().AsDict();

            if (values.count("stat_requests"s) != 0 && !values.at("stat_requests"s).AsArray().empty())
                OutputData(out);
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
            Array base_requests = doc_.value().GetRoot().AsDict().at("base_requests"s).AsArray();

            unordered_map<string_view, const Dict &> stop_to_stops_distances_queries;

            for (const auto &node : base_requests)
            {
                if (node.AsDict().at("type"s) != "Stop"s)
                    continue;

                const auto &stop_node = node.AsDict();

                const auto &stop_name = stop_node.at("name"s).AsString();

                db_.AddStop(stop_name,
                            {stop_node.at("latitude"s).AsDouble(),
                             stop_node.at("longitude"s).AsDouble()});

                stop_to_stops_distances_queries.insert({stop_name, stop_node.at("road_distances"s).AsDict()});
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
                if (node.AsDict().at("type"s) != "Bus"s)
                    continue;

                const auto &bus_node = node.AsDict();

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
            Dict render_settings = doc_.value().GetRoot().AsDict().at("render_settings"s).AsDict();

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

            rs.underlayer_color = detail::ParseColor(render_settings.at("underlayer_color"s));

            rs.underlayer_width = render_settings.at("underlayer_width"s).AsDouble();

            Array color_palette_tmp = render_settings.at("color_palette"s).AsArray();
            rs.color_palette.reserve(color_palette_tmp.size());

            for (const auto &node : color_palette_tmp)
                rs.color_palette.push_back(detail::ParseColor(node));

            req_handler_.SetRenderSettings(rs);
        }

        void JsonReader::InputRoutingSettings()
        {
            Dict routing_settings = doc_.value().GetRoot().AsDict().at("routing_settings"s).AsDict();

            router::RoutingSettings rs;
            rs.bus_wait_time = routing_settings.at("bus_wait_time").AsDouble();
            rs.bus_velocity = routing_settings.at("bus_velocity").AsDouble();

            req_handler_.SetRoutingSettings(rs);
        }

        void JsonReader::OutputData(std::ostream &out) {
            Array stat_requests = doc_.value().GetRoot().AsDict().at("stat_requests"s).AsArray();

            Array responses_array = Builder{}
                                        .StartArray()
                                        .EndArray()
                                        .Build()
                                        .AsArray();

            for (const auto &node : stat_requests)
            {
                const auto &type = node.AsDict().at("type"s);

                if (type == "Stop"s)
                {
                    const auto &stop_req_data = node.AsDict();

                    string key = "error_message"s;
                    Node::Value res_value{"not found"s};

                    auto *res = req_handler_.GetBusesByStop(stop_req_data.at("name"s).AsString());

                    if (res)
                    {
                        Array buses(res->size());
                        transform(res->begin(), res->end(), buses.begin(), [](const auto &bus_name)
                                  { return string(bus_name); });
                        sort(buses.begin(), buses.end(), [](const auto &node_a, const auto &node_b)
                             { return node_a.AsString() < node_b.AsString(); });

                        key = "buses"s;
                        res_value = move(buses);
                    }

                    responses_array.push_back(
                        Builder{}
                            .StartDict()
                            .Key("request_id"s)
                            .Value(stop_req_data.at("id"s).AsInt())
                            .Key(move(key))
                            .Value(move(res_value))
                            .EndDict()
                            .Build());
                }
                else if (type == "Bus"s)
                {
                    const auto &bus_req_data = node.AsDict();

                    auto bus_info = req_handler_.GetBusStat(bus_req_data.at("name"s).AsString());

                    if (bus_info.has_value())
                    {
                        responses_array.push_back(
                            Builder{}
                                .StartDict()
                                .Key("request_id"s)
                                .Value(std::move(bus_req_data.at("id"s).AsInt()))
                                .Key("curvature"s)
                                .Value(bus_info.value().curvature)
                                .Key("route_length"s)
                                .Value(bus_info.value().route_length)
                                .Key("stop_count"s)
                                .Value(bus_info.value().all_stops)
                                .Key("unique_stop_count"s)
                                .Value(bus_info.value().unique_stops)
                                .EndDict()
                                .Build());
                    }
                    else
                    {
                        responses_array.push_back(
                            Builder{}
                                .StartDict()
                                .Key("request_id"s)
                                .Value(bus_req_data.at("id"s).AsInt())
                                .Key("error_message"s)
                                .Value("not found"s)
                                .EndDict()
                                .Build());
                    }
                }
                else if (type == "Map"s)
                {
                    const auto &map_req_data = node.AsDict();

                    ostringstream output_map;
                    req_handler_.RenderMap().Render(output_map);

                    string map_str = output_map.str();
                    map_str.erase(map_str.size() - 1);

                    responses_array.push_back(
                        Builder{}
                            .StartDict()
                            .Key("request_id"s)
                            .Value(map_req_data.at("id"s).AsInt())
                            .Key("map"s)
                            .Value(move(map_str))
                            .EndDict()
                            .Build());
                }
                else if (type == "Route"s)
                {
                    const auto &route_req_data = node.AsDict();

                    string_view start_stop = route_req_data.at("from").AsString();
                    string_view end_stop = route_req_data.at("to").AsString();
                    auto ans = req_handler_.GetShortWayBetween(start_stop, end_stop);

                    if (ans.has_value())
                    {
                        Array items((*ans).items.size());
                        transform((*ans).items.begin(), (*ans).items.end(), items.begin(),
                                  [](const PathDataItem &data)
                                  {
                                      Dict dict;
                                      switch (data.index())
                                      {
                                      case 0:
                                      {
                                          const PathDataItemBus &info = get<PathDataItemBus>(data);
                                          dict.insert({"type"s, info.type});
                                          dict.insert({"bus"s, string(info.name)});
                                          dict.insert({"span_count"s, info.span_count});
                                          dict.insert({"time"s, info.time});
                                          break;
                                      }
                                      case 1:
                                      {
                                          const PathDataItemWait &info = get<PathDataItemWait>(data);
                                          dict.insert({"type"s, info.type});
                                          dict.insert({"stop_name"s, string(info.stop_name)});
                                          dict.insert({"time"s, info.time});
                                          break;
                                      }
                                      }
                                      return dict;
                                  });

                        responses_array.push_back(
                            Builder{}
                                .StartDict()
                                .Key("request_id"s)
                                .Value(route_req_data.at("id"s).AsInt())
                                .Key("total_time"s)
                                .Value((*ans).total_time)
                                .Key("items"s)
                                .Value(move(items))
                                .EndDict()
                                .Build());
                    }
                    else
                    {
                        responses_array.push_back(
                            Builder{}
                                .StartDict()
                                .Key("request_id"s)
                                .Value(route_req_data.at("id"s).AsInt())
                                .Key("error_message"s)
                                .Value("not found"s)
                                .EndDict()
                                .Build());
                    }
                }
            }

            Print(Document{Node{responses_array}}, out);
        }

    } // namespace iodata

    namespace detail {
        svg::Color ParseColor(const json::Node &node)
        {
            if (node.IsString())
            {
                return node.AsString();
            }
            else
            {
                json::Array color_tmp = node.AsArray();
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
    } // detail

} // namespace transport_catalogue
