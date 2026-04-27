#ifndef XMLBIND_H
#  define XMLBIND_H
#include<type_traits>
#include<stack>
#include<variant>
#include<string>
#include<functional>
#include<string_view>
#include<map>
#include<vector>
#include<memory>
#include<utility>
#include<iostream>
#include <xml/parser>
using xml::parser;
using std::index_sequence;
using std::make_index_sequence;
using std::variant;
using std::stack;
using std::string;
using std::reference_wrapper;
using std::ref;
using std::string_view;
using std::function;
using std::less;
using std::map;
using std::vector;
using std::make_unique;
using std::unique_ptr;
using std::pair;
using std::istream;
using std::ostream;
using std::forward;
using namespace std::string_literals;

namespace mpcs {
namespace v1 {
template<parser::event_type e>
struct event_t {
    static parser::event_type constexpr event = e;
};

// Some ugly boilerplate because we can only use parameter
// packs inside templates. P2858 proposes to remove this.
template<typename T> struct PETTHelper;
template<size_t... nums>
struct PETTHelper<index_sequence<nums...>> {
    using type = variant<event_t<parser::event_type(nums)>...>;
};
using event_variant = PETTHelper<make_index_sequence<parser::eof + 1>>::type;

struct event_holder : public event_variant {
    using event_variant::event_variant;
    inline event_holder(parser::event_type et);
};

template<typename ...Ts>
event_holder event_to_variant_helper(parser::event_type e, variant<Ts...>&&)
{
    static map<parser::event_type, event_holder> val_to_type = {
        {Ts::event, event_t<Ts::event>()}...
    };
    return val_to_type[e];
}

event_holder event_to_variant(parser::event_type e)
{
    return event_to_variant_helper(e, event_variant());
}

event_holder::event_holder(parser::event_type et)
    : event_variant(event_to_variant(et)) {}

template<typename T>
struct processor {
    virtual void doProcess(T&&) {}
    void afterProcess(T&&) {}
    virtual ~processor() = default;
};

struct event_processor;
stack<unique_ptr<event_processor>> eventProcessors;
template<typename T> struct event_processor_helper;


template <typename ...Ts>
struct event_processor_helper<variant<Ts...>> : public processor<Ts>... {
    using processor<Ts>::doProcess...;
    using processor<Ts>::afterProcess...;

    event_processor_helper(parser& p) : p(p) {}

    void process(parser::event_type e) {
        visit([this](auto&& x) {
            doProcess(forward<decltype(x)>(x));
            afterProcess(forward<decltype(x)>(x));
            }, 
            static_cast<event_variant&&>(event_holder(e))); // Workaround https://stackoverflow.com/questions/51309467/using-stdvisit-on-a-class-inheriting-from-stdvariant-libstdc-vs-libc
    }

    void afterProcess(event_t<parser::start_element>&&);

    void afterProcess(event_t<parser::end_element>&&) {
        eventProcessors.pop();
    }
    parser& p;
};

struct event_processor : public event_processor_helper<event_variant> {
    using event_processor_helper<event_variant>::event_processor_helper;
};

struct elt : public event_processor {
    using event_processor::event_processor;
};

struct schema_elt : public elt {
    using elt::elt;
};

struct element_elt : public elt {
    element_elt(parser& p);
    string name = p.attribute("name");
    string typeName = p.attribute("type", defaultTypeName());
    bool optional = (p.attribute("minOccurs", 1) == 0) && p.attribute("maxOccurs", 1) == 1;
    ~element_elt();
    string defaultTypeName() { return name + "_type"s; }
};

struct sequence_elt : public elt {
    using elt::elt;
};

struct complex_type_elt : public elt {
    complex_type_elt(parser& p);
    ~complex_type_elt();
};

stack<reference_wrapper<element_elt>> elementStack;
element_elt& cur_element() { return elementStack.top().get(); }

// Note: less<> for heterogeneous lookup for better semantics
unique_ptr<elt> elt_from_name(string_view name, parser& p) {
    static map<string, function<unique_ptr<elt>(parser&)>, less<>> makers = {
        {"schema", make_unique<schema_elt, parser&>},
        {"element", make_unique<element_elt, parser&>},
        {"sequence", make_unique<sequence_elt, parser&>},
        {"complexType", make_unique<complex_type_elt, parser&>}
    };
    return (makers.find(name)->second)(p);
}

template <typename ...Ts>
void event_processor_helper<variant<Ts...>>::afterProcess(event_t<parser::start_element>&&)
{
    eventProcessors.push(elt_from_name(p.name(), p));
}

struct type_base;
struct global_scope;
struct complex_type;
struct builtin_type;
struct string_type;

using all_types = typelist<type_base, global_scope, complex_type, builtin_type, string_type>;

struct data_member { // Aggregate
    string name;
    type_base& type;
    bool optional = false;
};

template<typename T>
struct single_type_visitor {
    virtual void visit(T const&) const {}
};

template<typename T> struct type_visitor_helper;
template<typename ...Ts>
struct type_visitor_helper<typelist<Ts...>> : public single_type_visitor<Ts>... {
    using single_type_visitor<Ts>::visit...;
};
using type_visitor = type_visitor_helper<all_types>;

struct scope {
    map<string, unique_ptr<type_base>> memberTypes;
    vector<data_member> dataMembers;
};

struct type_base {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    type_base() = default;
    type_base(string name) : name(name) {}
    virtual ~type_base() = default;
    string name;
};

struct builtin_type : public type_base {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    builtin_type(string name) : type_base(name) {}
};

struct string_type : public builtin_type {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    string_type() : builtin_type("std::string") {}
};

struct complex_type : virtual public type_base, public scope {
    virtual void accept(type_visitor const& tv) const { tv.visit(*this); }
    complex_type(parser& p) 
      : type_base(p.attribute("name", cur_element().defaultTypeName())),
        containingElementName(!elementStack.empty()? cur_element().name : ""),
        anonymous(p.attribute_present("name")){}
    string containingElementName;
    bool anonymous;
protected:
    complex_type() {}
};

struct global_scope : virtual public complex_type {
    global_scope() {
        memberTypes["xs:string"] = make_unique<string_type>();
    }
    virtual void accept(type_visitor const & tv) const { tv.visit(*this); }
};


vector<reference_wrapper<scope>> scopeStack;

type_base& findType(string name)
{
    for (auto it = scopeStack.rbegin(); it != scopeStack.rend(); it++) {
        auto lookup = it->get().memberTypes.find(name);
        if (lookup != it->get().memberTypes.end())
            return *lookup->second;
    }
    // Todo: Handle error
}

element_elt::element_elt(parser& p)
    : elt(p)
{
    elementStack.push(*this);
}

element_elt::~element_elt()
{
    scopeStack.back().get().dataMembers.push_back({ name, findType(typeName), optional });
    elementStack.pop();
}

complex_type_elt::complex_type_elt(parser& p)
    : elt(p)
{
    auto newType = make_unique<complex_type>(p);
    scope& newScope = *newType;
    scopeStack.back().get().memberTypes.insert(pair(newType->name, move(newType)));
    scopeStack.push_back(newScope);
}

complex_type_elt::~complex_type_elt()
{
    scopeStack.pop_back();
}
}
using namespace v1;
}

#endif
