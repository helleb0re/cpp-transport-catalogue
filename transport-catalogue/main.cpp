#include <iostream>
#include <fstream>

#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

int main()
{
    transport_catalogue::TransportCatalogue tc;
    transport_catalogue::input_reader::InputReader ir;
    transport_catalogue::output_reader::OutputReader orr;

    ifstream cin("test.txt");
    ofstream cout("tt.txt");

    ir.Load(cin, tc);
    orr.RequestInfo(cin, cout, tc);
}
