#include <fstream>
#include <iostream>
#include <string_view>

#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "request_handler.h"
#include "serialization.h"

using namespace std;
using namespace transport_catalogue;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    TransportCatalogue db;
    renderer::MapRenderer mr;
    router::TransportRouter tr;
    RequestHandler rh{db, mr, tr};

    ifstream input_file("input.json"s);
    ofstream output_file("output.json"s);

    const std::string_view mode(argv[1]);

    serialization::Serialization serializator(db, rh);
    if (mode == "make_base"sv) {
        serializator.MakeBase(input_file);
    } else if (mode == "process_requests"sv) {
        serializator.ProcessRequests(input_file, output_file);
    } else {
        PrintUsage();
        return 1;
    }
}