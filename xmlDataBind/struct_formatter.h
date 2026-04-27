#ifndef STRUCT_FORMATTER_H
#  define STRUCT_FORMATTER_H
#include <sstream>
#include "formatter.h"
#include "serializer_decl_formatter.h"
#include "IndentStream.h"
#include "fmt/format.h"
using fmt::format;
using std::ostringstream;

namespace mpcs {
namespace v1 {
// Don't output the deserializers immediately but instead
// write them to an ostringstream, so they can be output
// at the end when everything they use has been defined
static ostringstream cachedDeserializersString;
static IndentStream cachedDeserializers(cachedDeserializersString);
inline generate_args gaForDeserialization(generate_args f)
{
    return { f.factory, cachedDeserializers };
}

template<typename T> struct struct_formatter;
template<typename T> 
struct struct_formatter_bases 
  : public inheriter<struct_formatter, direct_bases_t<T, all_types>>, virtual public formatter<T> {
    virtual void generate(generate_args& f, T const&) {}
    virtual ~struct_formatter_bases() = default;
};

template<typename T>
struct struct_formatter : public struct_formatter_bases<T> {
};

template<>
struct struct_formatter<complex_type> : public struct_formatter_bases<complex_type> {
    void generate_begin(generate_args& f, complex_type const& ct) {
        f.os << format("struct {} {{\n", ct.name)
             << indent;
    }
    void generate_member_types(generate_args& f, complex_type const& ct) {
        for (auto& memberType : ct.memberTypes) {
            memberType.second->accept(formatter_visitor(f));
        }
    }

    static string dataMemberTypeName(data_member const& dm) {
        return dm.optional ? format("std::optional<{}>", dm.type.name) : dm.type.name;
    }
    void generate_data_members(generate_args& f, complex_type const& ct) {
        for (auto& dm : ct.dataMembers) {
            f.os << dataMemberTypeName(dm) << " " << dm.name << ";\n";
        }
    }

    void generate_members(generate_args& f, complex_type const& ct) {
        generate_member_types(f, ct);
        generate_data_members(f, ct);
    }
    void generate_end(generate_args& f, complex_type const& ct) {
        f.os << unindent << "};\n\n";
    }

    string memberAssignment(data_member const& dm) {
        return format("result.{} = fromXML<{}>(p, \"{}\", alreadyInElement);\nalreadyInElement = false;\n",
                      dm.name, dm.type.name, dm.name);
    }
    void generateMemberDeserializer(generate_args& f, data_member const& dm) {
        f.os << "if(!alreadyInElement) munchSpace(p);\n";
        if (!dm.optional) {
            f.os << memberAssignment(dm) << "\n";
        } else {
            f.os << format(
R"(if(!alreadyInElement)
    p.next_expect(xml::parser::start_element);
alreadyInElement = true;
if(p.name() == "{}") {{
    {}}}

)", dm.name, memberAssignment(dm));
        }
    }

    void generate_deserializer(generate_args& f, complex_type const& ct) {
        f.os << deserializer_specialization(ct) << " {\n" << indent; 
        if(!ct.anonymous)
            f.os << format("if(name.empty()) name = \"{}\";\n", ct.containingElementName);
        f.os << format(
R"({} result;
if(!alreadyInElement)
    p.next_expect(xml::parser::start_element);
alreadyInElement = false;
if(p.name() != name)
    throw std::runtime_error("expected " + name + ". Got " + p.name());
)", ct.name);

        for (auto& dm : ct.dataMembers) {
            generateMemberDeserializer(f, dm);
        }
        f.os << 
R"(munchSpace(p);
p.next_expect(xml::parser::end_element);
return result;
)"; 
        f.os << unindent << "}\n\n";
    }

    virtual void generate(generate_args& f, complex_type const &ct) override {
        generate_begin(f, ct);
        generate_members(f, ct);
        generate_end(f, ct);
        generate_args fa = gaForDeserialization(f);
        generate_deserializer(fa, ct);
    }
};

// Class note: I forgot to put in the override here and had to debug a lot. Given how nasty template debugging is, we would rather the compiler work for us
template<>
struct struct_formatter<global_scope> : public struct_formatter_bases<global_scope> {
    unique_ptr<formatter_factory> declFactory = std::make_unique<serializer_decl_formatter_factory>();

    void outputHeader(ostream& os) {
        os <<
R"(// Generated XML Schema Binding file
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
)";
    }

    virtual void generate(generate_args &f, global_scope const& ct) override {
        cachedDeserializersString.str(""); // Clear the cached declarations

        outputHeader(f.os);
        generate_member_types(f, ct);
        generate_args fa{ *declFactory, f.os };
        ct.accept(formatter_visitor(fa));
        f.os << "\n\n" << cachedDeserializersString.str();
    }
};

template<>
struct struct_formatter<string_type> : public struct_formatter_bases<string_type> {
    void generate_deserializer(generate_args& f, string_type const &s) {
        f.os << "template<>\nstd::string fromXML<std::string>(xml::parser &p, std::string name, bool alreadyInElement) {\n";
        f.os << indent << 
R"(if(!alreadyInElement)
    p.next_expect(xml::parser::start_element);
std::string result;
while(p.peek() == xml::parser::characters) {
    p.next();
    result += p.value();
}
p.next_expect(xml::parser::end_element);
return result;        
)";
        f.os << unindent << "}\n\n";
    }
    virtual void generate(generate_args & f, string_type const& s) override {
        generate_args fa = gaForDeserialization(f);
        generate_deserializer(fa, s);
    }

};
using struct_formatter_factory = parallel_concrete_factory<formatter_factory, struct_formatter>;

}
}
#endif
