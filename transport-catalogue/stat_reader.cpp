#include "stat_reader.h"
#include "input_reader.h"

using namespace std;

namespace transport_catalogue
{
    namespace output_reader
    {
        void OutputReader::RequestInfo(istream &input, ostream &output, TransportCatalogue &transport_catalogue)
        {
            string line;
            getline(input, line);

            int n = stoi(line);

            for (int i = 0; i < n; ++i)
            {
                getline(input, line);

                detail::LeftTrim(line);
                detail::RightTrim(line);

                size_t delim_pos = line.find(' ');

                if (line.find("Bus") == string::npos)
                {
                    string stop_name = line.substr(delim_pos + 1);

                    StopInfo stop_info;

                    try
                    {
                        stop_info = transport_catalogue.GetStopInfo(stop_name);
                    }
                    catch (const invalid_argument &e)
                    {
                        output << "Stop "s << stop_name << ": not found"s << endl;
                        continue;
                    }

                    if (stop_info.buses.size() == 0)
                    {
                        output << "Stop "s << stop_name << ": no buses"s << '\n';
                    }
                    else
                    {
                        output << "Stop "s << stop_name << ": buses"s;
                        for (const auto &bus : stop_info.buses)
                        {
                            output << " "s << bus;
                        }
                        output << endl;
                    }
                }
                else
                {
                    string bus_name = line.substr(delim_pos + 1);

                    BusInfo bus_info;

                    try
                    {
                        bus_info = transport_catalogue.GetBusInfo(bus_name);
                    }
                    catch (const invalid_argument &e)
                    {
                        output << "Bus "s << bus_name << ": not found"s << endl;
                        continue;
                    }
                    output << "Bus "s << bus_name << ": " << bus_info.all_stops << " stops on route, "s
                           << bus_info.unique_stops << " unique stops, "
                           << bus_info.route_length << " route length, "
                           << bus_info.curvature << " curvature"s << endl;
                }
            }
        }
    } // namespace output_reader

} // namespace transport_catalogue
