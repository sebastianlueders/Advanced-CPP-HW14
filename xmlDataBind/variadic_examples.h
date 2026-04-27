#ifndef VARIADIC_EXAMPLES_H
#  define VARIADIC_EXAMPLES_H
#include <utility>
#include <type_traits>
using std::enable_if_t;
using std::true_type;
using std::false_type;

namespace mpcs {
template <typename... Ts> struct typelist {};
// Declaration of Length template that calculates the number of parameters in a typelist
template<typename T> struct Length;

// Define specialized implementation for zero length typelists
template<>
struct Length<typelist<>> {
	static int constexpr value{0};
};

// Define specialization for typelists of any length
// This will only be matched for typelists with at least one parameter
// because the specialization above is a better match for typelists with no parameters
template<class T, typename... Us>
struct Length<typelist<T, Us...> >
{
	// We could just use sizeof...(Us),
	// but let’s see how to do ourselves
	static int constexpr value = 1 + Length<typelist<Us...>>::value;
};

template<class List, int i> struct TypeAt;
template<class Head, typename... Tail>
struct TypeAt<typelist<Head, Tail...>, 0>
{
  using type = Head;
};
template<class Head, typename... Tail, int i>
struct TypeAt<typelist<Head, Tail...>, i>
	: public TypeAt<typelist<Tail...>, i - 1>
{
};

// Find the index corresponding to a given type
template<class List, class Target> struct IndexOf;
template<class Target>
struct IndexOf<typelist<>, Target>
{
	static int constexpr value = -1; // Return -1 if not found
};
template<typename Target, typename... Tail>
struct IndexOf<typelist<Target, Tail...>, Target>
{
  static int constexpr value{0};
};
template<class Head, typename... Tail, class Target>
struct IndexOf<typelist<Head, Tail...>, Target>
{
private: // Store the index of the type in the tail
	static int constexpr temp = IndexOf<typelist<Tail...>, Target>::value;
public:
	// The index in the original typelist will be one higher than
	// the index in the tail, but we need to be careful and not
	// increment when the type was not found in the tail (indicated
	// by an index of -1)
	static int constexpr value = temp == -1 ? -1 : 1 + temp;
};

// Count of type in a typelist. Adding for typelist3 problems. Use TplType to handle both std::typelist and typelist3
template<typename X, typename Tpl> struct Count;

template<typename X, template<typename...> class TplType>
struct Count<X, TplType<>> {
	static int constexpr value = 0; // No occurrences
};

template<typename X, template<typename...> class TplType, typename ...Ts>
struct Count<X, TplType<X, Ts...>> {
	static int constexpr value = Count<X, TplType<Ts...>>::value + 1;
};

template<typename X, typename T, template<typename...> class TplType, typename ...Ts>
struct Count<X, TplType<T, Ts...>> : public Count<X, TplType<Ts...>> { // Inherit value
};

// Append the types in two typelists
template<class First, class Second> struct Append;

template<typename... Ts, typename... Us>
struct Append<typelist<Ts...>, typelist<Us...> >
{
	typedef typelist<Ts..., Us...> type;
};
template<typename A, typename B>
using Append_t = typename Append<A, B>::type;

template<typename T, typename A, typename B> struct Replace;

template<typename A, typename B>
struct Replace<typelist<>, A, B> {
	using type = typelist<>;
};

template<typename H, typename... Ts, typename A, typename B>
struct Replace<typelist<H, Ts...>, A, B>
 : public Append<typelist<H>, typename Replace<typelist<Ts...>, A, B>::type> {
};

template<typename... Ts, typename A, typename B>
struct Replace<typelist<A, Ts...>, A, B> {
	using type = typelist<B, Ts...>;
};

template<typename T, typename A, typename B> struct ReplaceAll;

template<typename A, typename B>
struct ReplaceAll<typelist<>, A, B> {
	using type = typelist<>;
};

template<typename H, typename... Ts, typename A, typename B>
struct ReplaceAll<typelist<H, Ts...>, A, B>
	: public Append<typelist<H>, typename ReplaceAll<typelist<Ts...>, A, B>::type> {
};

template<typename... Ts, typename A, typename B>
struct ReplaceAll<typelist<A, Ts...>, A, B> {
  using type 
	  = typename Append<typelist<B>, 
	                    typename ReplaceAll<typelist<Ts...>, A, B>::type>::type;
  /* If we had "_t" versions we could have done 
  using type = Append_t<typelist<B>, ReplaceAll_t<typelist<Ts...>, A, B>>;
  */
};


template<typename T> struct Reverse;
template<>
struct Reverse<typelist<>> {
	using type = typelist<>;
};

template<typename H, typename... Ts>
struct Reverse<typelist<H, Ts...>>
	: public Append<typename Reverse<typelist<Ts...>>::type, typelist<H>> {

};

// New for Lecture 8
template<typename X, class T> struct Remove;
template<typename X>
struct Remove<X, typelist<>> {
	using type = typelist<>;
};

template<typename X, typename ...Ts>
struct Remove<X, typelist<X, Ts...>> {
	using type = typelist<Ts...>;
};

template<typename X, typename H, typename ...Ts>
struct Remove<X, typelist<H, Ts...>>
	: public Append<typelist<H>, typename Remove<X, typelist<Ts...>>::type> {
};
template<typename X, class T>
using Remove_t = typename Remove<X, T>::type;

template<class C, template<typename> class P, class = void> struct Any;
template<template<typename> class P>
struct Any<typelist<>, P> : public false_type {};

template<typename H, typename ...Ts, template<typename> class P>
struct Any<typelist<H, Ts...>, P, enable_if_t<P<H>::value>>
	: public true_type {};

template<typename H, typename ...Ts, template<typename> class P>
struct Any<typelist<H, Ts...>, P, enable_if_t<!P<H>::value>>
	: public Any<typelist<Ts...>, P> {};

template<template<class...> class TT, class ...A>
struct Curry {
	template<class B> // Curry down to single argument to avoid clang bug
	using type = TT<A..., B>;
};
template<class, template<typename> class, class = void> struct Filter;

template<template<typename> class F>
struct Filter<typelist<>, F> {
	using type = typelist<>;
};

template<typename H, typename ...Ts, template<typename> class F>
struct Filter<typelist<H, Ts...>, F, enable_if_t<F<H>::value>> {
	using type = Append_t<typelist<H>, typename Filter<typelist<Ts...>, F>::type>;
};

template<typename H, typename ...Ts, template<typename> class F>
struct Filter<typelist<H, Ts...>, F, enable_if_t<!F<H>::value>>
	: public Filter<typelist<Ts...>, F> {
};

template<class T, template<typename> class F>
using Filter_t = typename Filter<T, F>::type;

template<template<typename> class T>
struct Not {
	template<typename U>
	struct type {
		static bool constexpr value = !T<U>::value;
	};
};

template<class C, template<typename> class F>
using FilterOut = Filter<C, Not<F>::template type>;
template<class C, template<typename> class F>
using FilterOut_t = FilterOut<C, F>;
}
#endif