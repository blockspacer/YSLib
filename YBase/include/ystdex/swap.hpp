﻿/*
	© 2014-2018 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file swap.hpp
\ingroup YStandardEx
\brief 交换操作。
\version r236
\author FrankHB <frankhb1989@gmail.com>
\since build 831
\par 创建时间:
	2018-07-12 16:38:36 +0800
\par 修改时间:
	2018-07-23 21:35 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YStandardEx::Swap
*/


#ifndef YB_INC_ystdex_swap_hpp_
#define YB_INC_ystdex_swap_hpp_ 1

#include "type_pun.hpp" // for "type_pun.hpp", std::swap,
//	is_nothrow_constructible, is_nothrow_assignable, std::move, add_volatile_t,
//	std::addressof, bool_, is_standard_layout, pun_storage_t,
//	aligned_replace_cast;
#include "type_traits.hpp" // for is_nothrow_swappable,
//	is_nothrow_swappable_with;

namespace ystdex
{

/*!
\brief 支持基于 std::swap 的 ADL 的左值交换。
\since build 831
\sa boost::swap
\sa std::iter_swap
\see LWG 809 。
\see WG21 P0934R0 。

在 swap 成员函数中无法直接使用 using 引入 std::swap 。
注意，除非崔在对参数更特定的 swap 函数，以 ystdex::swap 为名称的模板和 std::swap 被
	ADL 同时查找到后，重载时引起歧义。
若以 ystdex 中的类型作为模板参数，使用模板参数的依赖名称查找，只要包含头文件，
	实例化时总是不论顺序总是查找到 ystdex::swap ，
	若此时重载候选中同时存在 std::swap ，则引起歧义。
由于 is_nothrow_swappable 的实现（不论在哪个命名空间）总是要求 std::swap 作为重载
	候选之一，和 ystdex::swap 共存时总是在调用 ystdex::swap （不论是否使用 ystdex::
	限定）的用户代码中引起歧义导致无法使用（没有更特定的 swap 函数作为候选时），
	或失去使用 ystdex::swap 模板代理 ADL 调用的意义（对每个类型都另行声明 swap 函数
	作为重载候选时），本模板不使用 ystdex::swap 作为名称。
*/
template<typename _type>
yconstfn_relaxed void
swap_dependent(_type& x, _type& y) ynothrow(is_nothrow_swappable<_type>())
{
	using std::swap;

	swap(x, y);
}

/*!
\brief 交换非 volatile 左值和 volatile 左值。
\pre 可赋值。
\since build 704
*/
//@{
template<typename _type>
yimpl(enable_if_t)<is_nothrow_constructible<_type, volatile _type&&>()
	&& is_nothrow_assignable<_type&, volatile _type&&>()
	&& is_nothrow_assignable<volatile _type&, _type&&>()>
swap_volatile(_type& x, volatile _type& y)
	ynoexcept(is_nothrow_constructible<_type, volatile _type&>()
	&& noexcept(x = std::move(y)) && noexcept(y = std::move(x)))
{
	auto t(std::move(x));

	x = std::move(y);
	y = std::move(t);
}
template<typename _type>
inline auto
swap_volatile(volatile _type& x, _type& y)
	ynoexcept_spec(ystdex::swap_volatile(y, x))
	-> decltype(ystdex::swap_volatile(y, x))
{
	ystdex::swap_volatile(y, x);
}
template<typename _type, size_t _vN>
void
swap_volatile(_type(&x)[_vN], add_volatile_t<_type[_vN]>& y)
	ynoexcept(noexcept(ystdex::swap_volatile(x[0], y[0])))
{
	auto first(std::addressof(x[0]));
	auto first2(std::addressof(y[0]));

	for(const auto last(first + _vN); first != last; yunseq(++first, ++first2))
		ystdex::swap_volatile(*first, *first2);
}
//@}

//! \since build 704
namespace details
{

template<typename _type, typename _type2>
using swap_volatile_avail = bool_<is_volatile<remove_reference_t<_type>>::value
	!= is_volatile<remove_reference_t<_type2>>::value>;

#if YB_HAS_NOEXCEPT
template<typename _type, typename _type2>
using swap_volatile_noexcept = and_<swap_volatile_avail<_type, _type2>, bool_<
	noexcept(swap_volatile(std::declval<_type>(), std::declval<_type2>()))>>;
#else
template<typename _type, typename _type2>
using swap_volatile_noexcept = swap_volatile_avail<_type, _type2>;
#endif

} // namespace details;

/*!
\brief 交换相同标准布局类型可修改左值的存储。
\since build 620
*/
template<typename _type>
void
swap_underlying(_type& x, _type& y) ynothrow
{
	static_assert(is_standard_layout<_type>(),
		"Invalid underlying type found.");
	using utype = pun_storage_t<_type>;

	std::swap(ystdex::aligned_replace_cast<utype&>(x),
		ystdex::aligned_replace_cast<utype&>(y));
}

/*!
\brief 交换可能是 volatile 的左值或右值。
\pre 参数类型可交换，或具有一个 volatile 类型可用 swap_volatie 交换。
\since build 704
*/
//@{
//! \note 使用 ADL swap 或 std::swap 。
template<typename _type, typename _type2, yimpl(typename
	= enable_if_t<!details::swap_volatile_avail<_type, _type2>::value>)>
void
vswap(_type&& x, _type2&& y)
	ynoexcept(is_nothrow_swappable_with<_type&&, _type2&&>())
{
	ystdex::swap_dependent(yforward(x), yforward(y));
}
//! \note 使用 ADL swap_volatile 或 ystdex::swap_volatile 。
//@{
template<typename _type, typename _type2>
inline auto
vswap(_type&& x, _type2&& y) ynoexcept(detected_or_t<false_,
	details::swap_volatile_noexcept, _type, _type2>())
	-> yimpl(enable_if_t)<details::swap_volatile_avail<_type, _type2>::value>
{
	swap_volatile(yforward(x), yforward(y));
}
//@}
//@}


//! \note 使用 ADL swap 或 std::swap 。
//@{
/*!
\brief 复制后交换。
\since build 768
*/
template<typename _type, typename _type2 = _type>
inline _type&
copy_and_swap(_type& obj, const _type2& new_val)
{
	auto t(new_val);

	ystdex::swap_dependent(t, obj);
	return obj;
}

/*!
\brief 转移后交换。
\since build 818
*/
template<typename _type, typename _type2 = _type>
inline _type&
move_and_swap(_type& obj, _type2&& new_val)
{
	auto t(std::move(new_val));

	ystdex::swap_dependent(t, obj);
	return obj;
}
//@}

inline namespace cpp2014
{

#if __cpp_lib_exchange_function >= 201304
using std::exchange;
#else
/*!
\brief 交换值并返回旧值。
\return 被替换的原值。
\see WG21 N3797 20.2.3[utility.exchange] 。
\see http://www.open-std.org/JTC1/sc22/WG21/docs/papers/2013/n3668.html 。
\since build 517
*/
template<typename _type, typename _type2 = _type>
_type
exchange(_type& obj, _type2&& new_val)
{
	_type old_val = std::move(obj);

	obj = std::forward<_type2>(new_val);
	return old_val;
}
#endif

} // inline namespace cpp2014;

} // namespace ystdex;

#endif
