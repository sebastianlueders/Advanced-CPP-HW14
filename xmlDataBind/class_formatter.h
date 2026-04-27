#ifndef CLASS_FORMATTER_H
#  define CLASS_FORMATTER_H
#include "formatter.h"
#include "ostream_joiner.h"
#include <algorithm>
#include <string>
#include <iterator>
#include <sstream>
using std::copy;
using std::transform;
using std::back_inserter;
using std::ostringstream;

namespace mpcs {
namespace v1 {
    template<typename T> struct class_formatter;

    template<typename T>
    struct class_formatter_bases
        : public inheriter<class_formatter, direct_bases_t<T, all_types>>, virtual public formatter<T> {
    };

    template<typename T>
    struct class_formatter : public class_formatter_bases<T> {
        virtual ~class_formatter() = default;
    };

    template<>
    struct class_formatter<complex_type> : public class_formatter_bases<complex_type> {
        void generate_begin(generate_args& f, complex_type const& ct) {
            f.os << "class " << ct.name << "{\nprivate:\n"
                << indent;
        }
        void generate_member_types(generate_args& f, complex_type const& ct) {
            for (auto& memberType : ct.memberTypes) {
                memberType.second->accept(formatter_visitor(f));
            }
        }
        void generate_data_members_private(generate_args& f, complex_type const& ct) {
            for (auto& dataMember : ct.dataMembers) {
                f.os << dataMember.type.name << " " << dataMember.name << ";\n";
            }
        }

        void generate_members_private(generate_args& f, complex_type const& ct) {
            generate_member_types(f, ct);
            generate_data_members_private(f, ct);
        }

        void generate_constructor(generate_args& f, complex_type const& ct) {
            f.os << ct.name << "(";
            vector<string> arguments;
            transform(ct.dataMembers.begin(), ct.dataMembers.end(), back_inserter(arguments),
                [&](data_member const& dm) { return dm.type.name + " " + dm.name; });
            ostringstream argstr;
            copy(arguments.begin(), arguments.end(), ostream_joiner(argstr, ", "));
            f.os << argstr.str() << ")\n  : ";

            vector<string> initializers;
            transform(ct.dataMembers.begin(), ct.dataMembers.end(), back_inserter(initializers),
                [&](data_member const& dm) { return dm.name + "(move(" + dm.name + "))"; });
            ostringstream initstr;
            copy(initializers.begin(), initializers.end(), ostream_joiner(initstr, ", "));
            f.os << initstr.str() << " {}\n";
        }

        void generate_setters(generate_args& f, data_member const &dataMember) {
            f.os << "void set_" << dataMember.name << "(" << dataMember.type.name << " const & x) {\n";
            f.os << indent << dataMember.name << " = x;\n" << unindent << "}\n";
        }

        void generate_getters(generate_args& f, data_member const &dataMember) {
            f.os << dataMember.type.name << " get_" << dataMember.name << "() && {\n";
            f.os << indent << "return std::move(" << dataMember.name << ");\n" << unindent << "}\n";
 
            f.os << dataMember.type.name << " const & get_" << dataMember.name << "() const & {\n";
            f.os << indent << "return " << dataMember.name << ";\n" << unindent << "}\n";
        }

        void generate_data_members_public(generate_args& f, complex_type const& ct) {
            for (auto& dataMember : ct.dataMembers) {
                generate_setters(f, dataMember);
                generate_getters(f, dataMember);
                f.os << "\n";
            }
        }

        void generate_members_public(generate_args& f, complex_type const& ct) {
            generate_constructor(f, ct);
            generate_member_types(f, ct);
            generate_data_members_public(f, ct);
        }

        void generate_end(generate_args& f, complex_type const& ct) {
            f.os << unindent << "};\n\n";
        }

        virtual void generate(generate_args& f, complex_type const& ct) override {
            generate_begin(f, ct);
            generate_members_private(f, ct);
            f.os << unindent << "public:\n" << indent;
            generate_members_public(f, ct);
            generate_end(f, ct);
        }
    };

    // Class note: I forgot to put in the override here and had to debug a lot. Given how nasty template debugging is, we would rather the compiler work for us
    template<>
    struct class_formatter<global_scope> : public class_formatter_bases<global_scope> {
        virtual void generate(generate_args& f, global_scope const& ct) override {
            generate_member_types(f, ct);
        }
    };

    using class_formatter_factory = parallel_concrete_factory<formatter_factory, class_formatter>;

}
}
#endif
