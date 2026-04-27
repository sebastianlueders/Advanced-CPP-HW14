#ifndef FACTORY_H
#define FACTORY_H
#include<utility>
#include<memory>
#include<type_traits>
#include "my_type_traits.h"
#pragma warning(disable:4250)
// Notes for class: Had to add a lot of const-correctness
using std::unique_ptr;
using std::add_const_t;

namespace mpcs {
	template<typename T>
	struct Type2Type {
		typedef T type;
	};

	template<typename T> struct adapt_signature {
		using type = add_const_t<T> &;
	};

	template<typename T> 
	struct adapt_signature<T &> {
		using type = T;
	};

	template<typename T>
	struct adapt_signature<T &&> {
		using type = T;
	};

	template<typename T>
    struct abstract_creator {
      virtual unique_ptr<T> doCreate(Type2Type<T> &&) const = 0;
    };

	template<typename Result, typename ...Args>
	struct abstract_creator<Result*(Args...)>
	{
      virtual unique_ptr <Result> 
		  doCreate(Type2Type<Result>&&, 
			       typename adapt_signature<Args>::type...) const = 0;
	};

	template<typename Result, typename ...Ts> struct Signature;
	template<typename Result, typename ...Ts> struct Signature<Result, Result, Ts...> {
		using type = Result;
	};
	template<typename Result, typename ...Args, typename ...Ts> 
	struct Signature<Result, Result*(Args...), Ts...> {
		using type = Result*(Args...);
	};

	template<typename Result, typename T, typename ...Ts> 
    struct Signature<Result, T, Ts...> : public Signature<Result, Ts...> {};


	template<typename T> struct abstract_factory;
	template<typename ...Ts>
	struct abstract_factory<typelist<Ts...>> : public abstract_creator<Ts>... {
		template<class U, typename ...Args> unique_ptr<U> create(Args&&... args) const {
			abstract_creator<typename Signature<U, Ts...>::type> const& creator = *this;
			return creator.doCreate(Type2Type<U>(), std::forward<Args>(args)...);
		}
		virtual ~abstract_factory() {}
	};

	template<typename AbstractFactory, typename Abstract, typename Concrete>
	struct concrete_creator : virtual public AbstractFactory {
		unique_ptr<Abstract> doCreate(Type2Type<Abstract>&&) const {
			return std::make_unique<Concrete>();
		}
	};

	template<typename AbstractFactory, typename Result, typename... Args, typename Concrete>
	struct concrete_creator<AbstractFactory, Result*(Args...), Concrete> : virtual public AbstractFactory {
		unique_ptr<Result> doCreate(Type2Type<Result>&&, 
			                        typename adapt_signature<Args>::type... args) const {
			return std::make_unique<Concrete>(std::forward<adapt_signature<Args>::type>(args)...);
		}
	};

	template<typename AbstractFactory, typename... ConcreteTypes>
	struct concrete_factory;

	template<typename... AbstractTypes, typename... ConcreteTypes>
	struct concrete_factory
		<abstract_factory<typelist<AbstractTypes...>>, typelist<ConcreteTypes...>>
		: public concrete_creator<abstract_factory<typelist<AbstractTypes...>>, AbstractTypes, ConcreteTypes>... {
	};

	template<typename AbstractFactory, template<typename> class Concrete>
	struct parameterized_factory;

	template<template<typename> class Concrete, typename... AbstractTypes>
	struct parameterized_factory<abstract_factory<typelist<AbstractTypes...>>, Concrete>
		: public concrete_factory<abstract_factory<typelist<AbstractTypes...>>, 
		                          typelist<Concrete<AbstractTypes>...>> {
	};

	// Alternate implemenation of flexible factory based on C++17 variadic using statements
	// No need for Signature metaprogramming!
	template<typename T> struct flexible_abstract_factory;
	template<typename... Ts>
	struct flexible_abstract_factory<typelist<Ts...>> : public abstract_creator<Ts>... {
		using abstract_creator<Ts>::doCreate...;
		template<class U, typename ...Args> unique_ptr<U> create(Args&&... args) {
			return doCreate(Type2Type<U>(), args...);
		}
		virtual ~flexible_abstract_factory() {}
	};


	template<typename... AbstractTypes, typename... ConcreteTypes>
	struct concrete_factory
		<flexible_abstract_factory<AbstractTypes...>, typelist<ConcreteTypes...>>
		: public concrete_creator<flexible_abstract_factory<AbstractTypes...>, AbstractTypes, ConcreteTypes>... {
	};

	template<template<typename> class Abstract, class Types> struct parallel_abstract_factory_helper;
	template<template<typename> class Abstract, typename ...Ts>
	struct parallel_abstract_factory_helper<Abstract, typelist<Ts...>> {
		using type = abstract_factory<typelist<Abstract<Ts>...>>;
	};

	template<template<typename> class Abstract, class Types> 
	using parallel_abstract_factory = typename parallel_abstract_factory_helper<Abstract, Types>::type;

	template<typename AbstractFactory, template<typename> class Concrete>
	struct parallel_concrete_factory;

	template<template<typename> class Abstract, typename ...Ts, template<typename> class Concrete>
	struct parallel_concrete_factory<abstract_factory<typelist<Abstract<Ts>...>>, Concrete>
		: public concrete_factory<abstract_factory<typelist<Abstract<Ts>...>>, typelist<Concrete<Ts>...>> {
	};


}
#endif
