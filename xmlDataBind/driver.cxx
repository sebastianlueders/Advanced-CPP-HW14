// file      : examples/performance/driver.cxx
// copyright : not copyrighted - public domain

#include <string>
#include <fstream>
#include <iostream>
#include <variant>
#include <utility>
#include <functional>
#include <map>
#include <xml/parser>
#include "my_type_traits.h"
#include "advanced_factory.h"
#include "IndentStream.h"
#include <set>
#include "xsd2cpp.h"
#include "formatter.h"
#include "struct_formatter.h"
#include "class_formatter.h"

using namespace std;
using namespace xml;
using namespace mpcs;


int main(int argc, char* argv[])
{
    struct_formatter_factory f;
    unique_ptr<formatter_factory> formatterFactory = make_unique<struct_formatter_factory>();
    if (argc != 3)
    {
        cerr << "usage: " << argv[0] << " <xml-file> <.h-file>" << endl;
        return 1;
    }

    ifstream ifs(argv[1]);
    ofstream ofs(argv[2]);
    xsd2cpp(ifs, ofs, *formatterFactory);
}
