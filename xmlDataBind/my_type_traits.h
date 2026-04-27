#ifndef MY_TYPE_TRAITS_H
#  define MY_TYPE_TRAITS_H
#include <type_traits>
#include "variadic_examples.h"

using std::conjunction;
using std::negation;
using std::is_base_of;
using std::is_same;

namespace mpcs {
template<class D, class B> struct is_derived_from
    : public is_base_of<B, D> {
};

template<class X, class Candidates>
struct bases
    : public Filter<Remove_t<X, Candidates>,
                    Curry<is_derived_from, X>::template type> {};

template<class X, class Candidates>
using bases_t = typename bases<X, Candidates>::type;

template<typename B, typename D>
struct is_nontrivial_base_of
    : public conjunction<is_base_of<B, D>, negation<is_same<B, D>>> {};

template<class Candidates, class X, class B>
struct is_indirect_base_of
    : Any<bases_t<X, Candidates>,
    Curry<is_nontrivial_base_of, B>::template type> {};

template<class Candidates, class X, class B>
inline bool constexpr is_indirect_base_of_v = is_indirect_base_of<Candidates, X, B>::value;

template<class X, class Candidates>
struct direct_bases
    : public FilterOut<bases_t<X, Candidates>, Curry<is_indirect_base_of, Candidates, X>::template type> { };
template<class X, class Candidates>
using direct_bases_t = typename direct_bases<X, Candidates>::type;

template<template<typename> class TTP, typename T> struct inheriter;
template<template<typename> class TTP, typename ...Ts>
struct inheriter<TTP, typelist<Ts...>> : virtual public TTP<Ts>... {};
template<template<typename> class TTP, class T, typename Candidates>
struct parallel_bases
    : virtual public inheriter<TTP, direct_bases_t<T, Candidates>>,
      virtual public T {};

}
#endif