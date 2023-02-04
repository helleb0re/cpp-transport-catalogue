#include "input_reader.h"

using namespace std;

namespace transport_catalogue
{
    namespace detail
    {
        void RightTrim(string_view &word)
        {
            size_t x = 0;
            while (isspace(word[word.size() - 1 - x]))
            {
                ++x;
            }
            word.remove_suffix(x);
        }

        void RightTrim(string &word)
        {
            size_t x = word.size();
            while (isspace(word[x - 1]))
            {
                --x;
            }
            word.erase(x, word.size() - x);
        }

        void LeftTrim(string_view &word)
        {
            size_t x = 0;
            while (isspace(word[x]))
            {
                ++x;
            }
            word.remove_prefix(x);
        }

        void LeftTrim(string &word)
        {
            size_t x = 0;
            while (isspace(word[x]))
            {
                ++x;
            }
            word.erase(0, x);
        }

        string_view sub_string(
            string_view s,
            size_t p,
            size_t n)
        {
            return s.substr(p, n);
        }
    } // namespace detail

    namespace input_reader
    {
        vector<string_view> InputReader::FillStopsVector(const string &stops_route_info, char delim)
        {
            vector<string_view> res;

            size_t stops_delim_pos = stops_route_info.find(delim);
            size_t prev_pos = 0;

            while (stops_delim_pos != string::npos)
            {
                string_view stop_name = detail::sub_string(stops_route_info, prev_pos, stops_delim_pos - prev_pos - 1);

                detail::RightTrim(stop_name);
                detail::LeftTrim(stop_name);

                res.push_back(stop_name);

                prev_pos = stops_delim_pos + 1;
                stops_delim_pos = stops_route_info.find(delim, stops_delim_pos + 1);
            }

            string_view stop_name = detail::sub_string(stops_route_info, prev_pos);

            detail::RightTrim(stop_name);
            detail::LeftTrim(stop_name);

            res.push_back(stop_name);

            return res;
        }

        void InputReader::Load(istream &input, TransportCatalogue &transport_catalogue)
        {
            string line;
            getline(input, line);

            int n = stoi(line);

            vector<string> stops_queries;
            vector<string> buses_routes_queries;
            unordered_map<string, string> stops_distances_queries;

            for (int i = 0; i < n; ++i)
            {
                getline(input, line);
                if (line.find("Bus") == string::npos)
                {
                    stops_queries.push_back(move(line));
                }
                else
                {
                    buses_routes_queries.push_back(move(line));
                }
            }

            for (const auto &stop_query : stops_queries)
            {
                size_t delim_pos = stop_query.find(':');
                string stop_name = stop_query.substr(5, delim_pos - 5);

                string coordinates_info = stop_query.substr(delim_pos + 1);
                delim_pos = coordinates_info.find(',');

                string lat_str = coordinates_info.substr(0, delim_pos);

                coordinates_info = coordinates_info.substr(delim_pos + 1);
                delim_pos = coordinates_info.find(',');

                string lng_str = coordinates_info.substr(0, delim_pos);

                double lat = stod(lat_str);
                double lng = stod(lng_str);

                transport_catalogue.AddStop(stop_name, lat, lng);

                if (delim_pos != string::npos)
                {
                    stops_distances_queries[stop_name] = coordinates_info.substr(delim_pos + 1);
                }
            }

            for (const auto &[stop_name, stops_distances_info] : stops_distances_queries)
            {
                vector<pair<string_view, double>> data;
                
                size_t delim_pos;
                size_t prev_pos = 0;

                while (prev_pos != string::npos)
                {
                    delim_pos = stops_distances_info.find(',', prev_pos);

                    size_t delim_to_pos = stops_distances_info.find("to", prev_pos);

                    string dist_str = stops_distances_info.substr(prev_pos, delim_to_pos);
                    double dist = stod(dist_str);

                    string_view second_stop_name = detail::sub_string(stops_distances_info,
                                                                      delim_to_pos + 2, delim_pos - delim_to_pos - 2);
                    detail::RightTrim(second_stop_name);
                    detail::LeftTrim(second_stop_name);

                    data.push_back({second_stop_name, dist});

                    prev_pos = (delim_pos == string::npos) ? string::npos : delim_pos + 1;
                }

                transport_catalogue.AddStopDistances(stop_name, data);
            }

            for (const auto &bus_route_query : buses_routes_queries)
            {
                size_t delim_pos = bus_route_query.find(':');
                string bus_route_name = bus_route_query.substr(4, delim_pos - 4);

                string route_info = bus_route_query.substr(delim_pos + 1);

                vector<string_view> stops_names;

                size_t stops_delim_pos = route_info.find('>');
                if (stops_delim_pos != string::npos)
                {
                    stops_names = FillStopsVector(route_info, '>');
                }
                else
                {
                    stops_names = FillStopsVector(route_info, '-');
                    stops_names.insert(stops_names.end(), stops_names.rbegin() + 1, stops_names.rend());
                }

                transport_catalogue.AddBus(bus_route_name, stops_names);
            }
        }
    } // namespace input_reader

} // namespace transport_catalogue
