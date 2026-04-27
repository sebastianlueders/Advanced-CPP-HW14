// Generated XML Schema Binding file
// UChicago MPCS51045

#include <xml/parser>
#include <string>
#include <cctype>
#include <algorithm>
#include <stdexcept>
#include <optional>

template<typename T>
T fromXML(xml::parser &p, std::string name = "", bool alreadyInElement = false);
// skip over whitespace in XML file
void munchSpace(xml::parser &p)
{
    while(p.peek() == p.characters) {
        p.next();
        auto s = p.value();
        if(!std::all_of(s.begin(),s.end(),static_cast<int(*)(int)>(std::isspace)))
            throw std::runtime_error("Unexpected characters: " + s);
    }
}
struct note_type {
    std::string to;
    std::string from;
    std::string body;
};

template<>
note_type fromXML<note_type>(xml::parser &p, std::string name, bool alreadyInElement);
template<>
std::string fromXML<std::string>(xml::parser &p, std::string name, bool alreadyInElement);


template<>
note_type fromXML<note_type>(xml::parser &p, std::string name, bool alreadyInElement) {
    if(name.empty()) name = "note";
    note_type result;
    if(!alreadyInElement)
        p.next_expect(xml::parser::start_element);
    alreadyInElement = false;
    if(p.name() != name)
        throw std::runtime_error("expected " + name + ". Got " + p.name());
    if(!alreadyInElement) munchSpace(p);
    result.to = fromXML<std::string>(p, "to", alreadyInElement);
    alreadyInElement = false;

    if(!alreadyInElement) munchSpace(p);
    result.from = fromXML<std::string>(p, "from", alreadyInElement);
    alreadyInElement = false;

    if(!alreadyInElement) munchSpace(p);
    result.body = fromXML<std::string>(p, "body", alreadyInElement);
    alreadyInElement = false;

    munchSpace(p);
    p.next_expect(xml::parser::end_element);
    return result;
}

template<>
std::string fromXML<std::string>(xml::parser &p, std::string name, bool alreadyInElement) {
    if(!alreadyInElement)
        p.next_expect(xml::parser::start_element);
    std::string result;
    while(p.peek() == xml::parser::characters) {
        p.next();
        result += p.value();
    }
    p.next_expect(xml::parser::end_element);
    return result;        
}

