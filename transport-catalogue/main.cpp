#include <iostream>
#include <fstream>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json_reader.h"

using namespace std;
using namespace transport_catalogue;

int main()
{
    TransportCatalogue db;
    renderer::MapRenderer mr;
    RequestHandler rh{db, mr};

    ifstream input_file("input.json"s);
    ofstream output_file("output.json"s);
    iodata::JsonReader json_reader(db, rh, output_file);
    json_reader.LoadFile(input_file);
}