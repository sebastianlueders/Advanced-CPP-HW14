#ifndef FORMATTER_H
#  define FORMATTER_H
#include<optional>
#include "xmlbind.h"
#include "advanced_factory.h"
using std::optional;

namespace mpcs {
namespace v1 {
// Note: I had to struggle with forward references
struct generate_args;

template<typename T>
struct formatter : public inheriter<formatter, direct_bases_t<T, all_types>> {
    virtual void generate(generate_args& f, T const&) = 0;
    virtual ~formatter() = default;
};

using formatter_factory = parallel_abstract_factory<formatter, all_types>;

struct generate_args {
    formatter_factory const& factory;
    ostream& os;
};

/*
    We want formatter_visitor to look something like this
    (and programmers usually  do this manually)
    struct formatter_visitor : public type_visitor {
    formatter_visitor(generate_args &f) : fa(f) {}
    virtual void visit(base_type const &t) override { f.factory.create<formatter<base_type>>()->format(fa, t); }
    virtual void visit(complex_type const &t) override { f.factory.create<formatter<complex_type>>()->format(fa, t); }
    // ...
    generate_args fa;
    };
    But since all of the visit methods are of the same form, we can use the same trick as for type_visitor
*/
struct formatter_visitor_base : public type_visitor {
    formatter_visitor_base() = default; // Question: This will never be called. Why? Why do I need it?
    formatter_visitor_base(generate_args& fa) : fa(fa) {}
    optional<reference_wrapper<generate_args>> fa;
};


// static_assert using consteval: enforces compile-time consistency between
// all_types and the formatter/visitor infrastructure. If all_types grows,
// this fires immediately at compile time rather than silently misbehaving.
static_assert(typelist_size<all_types>() == 5,
    "all_types size changed; update visitor and formatter infrastructure accordingly");

// Constrained with Visitable (C++20 concept): T must support accept().
template<Visitable T>
struct single_formatter_visitor : virtual public formatter_visitor_base {
    virtual void visit(T const& t) const override {
        fa->get().factory.template create<formatter<T>>()->generate(*fa, t);
    }
};

template<typename T> struct formatter_visitor_helper;
template<typename ...Ts>
struct formatter_visitor_helper<typelist<Ts...>>
    : public single_formatter_visitor<Ts>..., virtual public formatter_visitor_base{
    formatter_visitor_helper(generate_args & fa) : formatter_visitor_base(fa) {}
};
using formatter_visitor = formatter_visitor_helper<all_types>;

}
}
#endif