#ifndef FACTORY_H
#define FACTORY_H
#include<tuple>
#include<utility>
#include<memory>
#include<type_traits>
#pragma warning(disable:4250)

using std::tuple;
using std::unique_ptr;
using std::add_const_t;
using std::make_unique;
using std::enable_if_t;
using std::is_base_of_v;

namespace mpcs {
	template<typename T>
	struct TT {
	};

	template<typename T>
    struct abstract_creator {
      virtual unique_ptr<T> doCreate(TT<T> &&) = 0;
    };

	template<typename Result, typename ...Args>
	struct abstract_creator<Result(Args...)>
	{
      virtual unique_ptr <Result> 
		  doCreate(TT<Result>&&, 
			       Args...) = 0;
	};


	template<typename... Ts>
	struct abstract_factory : public abstract_creator<Ts>... {
		using abstract_creator<Ts>::doCreate...;
		template<class U, typename ...Args> unique_ptr<U> create(Args&&... args) {
			return doCreate(TT<U>(), std::forward<Args>(args)...);
		}
		virtual ~abstract_factory() = default;
	};

	template<typename AbstractFactory, typename Abstract, typename Concrete>
	struct concrete_creator : virtual public AbstractFactory {
		unique_ptr<Abstract> doCreate(TT<Abstract> &&) override {
			return make_unique<Concrete>();
		}
	};

	template<typename AbstractFactory, typename Result, typename... Args, typename Concrete>
	struct concrete_creator<AbstractFactory, Result(Args...), Concrete> : virtual public AbstractFactory {
		unique_ptr<Result> doCreate(TT<Result>&&, Args... args) {
			return make_unique<Concrete>(args...);
		}
	};

	template<typename AbstractFactory, typename ...ConcreteTypes>
	struct concrete_factory;

	template<typename... AbstractTypes, typename... ConcreteTypes>
	struct concrete_factory
		<abstract_factory<AbstractTypes...>, ConcreteTypes...>
		: public concrete_creator<abstract_factory<AbstractTypes...>, AbstractTypes, ConcreteTypes>... {
	};

	template<typename AbstractFactory, template<typename> class Concrete>
	struct parameterized_factory;

	template<template<typename> class Concrete, typename... AbstractTypes>
	struct parameterized_factory<abstract_factory<AbstractTypes...>, Concrete>
		: public concrete_factory<abstract_factory<AbstractTypes...>, 
		                          Concrete<AbstractTypes>...> {
	};



}
#endif
