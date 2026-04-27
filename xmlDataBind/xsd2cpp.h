#ifndef XSD2CPP_H
#  define XSD2CPP_H
#include <iostream>
#include "xmlbind.h"
#include "formatter.h"
using std::istream;
using std::ostream;

namespace mpcs {
namespace v1 {

global_scope inhaleSchema(istream& is)
{
    global_scope global;

    scopeStack.push_back(std::ref(static_cast<scope &>(global)));
    parser p(is, "schema");
    event_processor ep(p);
    eventProcessors.push(make_unique<event_processor>(p));
    for (parser::event_type e : p)
    {
        eventProcessors.top()->process(e);
        p.attribute_map(); // https://www.codesynthesis.com/pipermail/studxml-users/2015-February/000006.html
    }
    return global;
}

void toCPP(global_scope const& global, ostream& os, formatter_factory const& ff)
{
    IndentStream ios(os);
    generate_args fa{ ff, ios };
    global.accept(formatter_visitor(fa));
}

void xsd2cpp(istream& is, ostream& os, formatter_factory const& ff)
{
    auto global = inhaleSchema(is);
    toCPP(global, os, ff ); // Doesn't work on MSVC due to weird compiler bug, 
                               // but copying the body is fine, so that what we do as a workaround :/
    // IndentStream ios(os);
    // generate_args fa{ ff, ios };
    // global.accept(formatter_visitor(fa));
}

}
}
#endif