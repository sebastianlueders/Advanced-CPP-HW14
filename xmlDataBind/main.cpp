#include <iostream>
#include <fstream>
#include <xml/parser>
#include "note.h"

int main()
{
    std::ifstream f("note.xml");
    xml::parser p(f, "note.xml");
    note_type note = fromXML<note_type>(p);
    std::cout << "To: " << note.to << "\n";
    std::cout << "From: " << note.from << "\n";
    std::cout << "\n>> " << note.body << "\n";
    return 0;
}