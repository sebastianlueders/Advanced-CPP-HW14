#ifndef SERIALIZER_DECL_FORMATTER
#  define SERIALIZER_DECL_FORMATTER

// Declares (but does not define) the deserializers. E.g.,
// template<> std::string fromXML<std::string>(xml::parser& p, std::string name);

#include "formatter.h"
#include "fmt/format.h"

using fmt::format;

namespace mpcs {
namespace v1 {
bases_t<complex_type, typelist<type_base, global_scope, complex_type, builtin_type, string_type> >* foo;
inline string deserializer_specialization(type_base const& t) {
    return format("template<>\n{} fromXML<{}>(xml::parser &p, std::string name, bool alreadyInElement)",
        t.name, t.name);
}

template<typename T> struct serializer_decl_formatter;
template<typename T>
struct serializer_decl_formatter_bases
    : public inheriter<serializer_decl_formatter, direct_bases_t<T, all_types>>, virtual public formatter<T> {
};
template<typename T>
struct serializer_decl_formatter : public serializer_decl_formatter_bases<T> {
    virtual void generate(generate_args& f, T const& ct) override {
        f.os << deserializer_specialization(ct) << ";\n";
    }
    virtual ~serializer_decl_formatter() = default;
};

template<>
struct serializer_decl_formatter<complex_type> : public serializer_decl_formatter_bases<complex_type> {
    void generate_member_types(generate_args& f, complex_type const& ct) {
        for (auto& memberType : ct.memberTypes) {
            memberType.second->accept(formatter_visitor(f));
        }
    }

    virtual void generate(generate_args& f, complex_type const& ct) override {
        serializer_decl_formatter<type_base>::generate(f, ct);
        generate_member_types(f, ct);
    }
};

template<>
struct serializer_decl_formatter<global_scope> : public serializer_decl_formatter_bases<global_scope> {
    virtual void generate(generate_args& f, global_scope const& ct) override {
        generate_member_types(f, ct);
    }
};
using serializer_decl_formatter_factory
  = parallel_concrete_factory<formatter_factory, serializer_decl_formatter>;

}
}

#endif