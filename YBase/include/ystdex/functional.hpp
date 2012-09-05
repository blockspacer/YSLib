﻿/*
	Copyright (C) by Franksoft 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file utility.hpp
\ingroup YStandardEx
\brief 函数和可调用对象。
\version r355
\author FrankHB<frankhb1989@gmail.com>
\since build 333
\par 创建时间:
	2010-08-22 13:04:29 +0800
\par 修改时间:
	2012-09-04 12:32 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YStandardEx::Functional
*/


#ifndef YB_INC_YSTDEX_FUNCTIONAL_HPP_
#define YB_INC_YSTDEX_FUNCTIONAL_HPP_ 1

#include "type_op.hpp" // for ../ydef.h, ystdex::variadic_sequence,
	// ystdex::make_natural_sequence;
#include <functional>
#include <tuple>
#include <string> // for std::char_traits;

namespace ystdex
{

/*!
\brief 顺序递归调用。
\since build 327
*/
//@{
template<typename _fCallable>
inline void
seq_apply(_fCallable&&)
{}
template<typename _fCallable, typename _type, typename... _tParams>
inline void
seq_apply(_fCallable&& f, _type&& arg, _tParams&&... args)
{
	f(yforward(arg)), ystdex::seq_apply(f, yforward(args)...);
}
//@}


/*!
\brief 无序调用。
\since build 327
*/
template<typename _fCallable, typename... _tParams>
inline void
unseq_apply(_fCallable&& f, _tParams&&... args)
{
	yunseq((f(yforward(args)), 0)...);
}


/*!
\ingroup metafunctions
\brief 取参数列表元组。
\since build 333
*/
//@{
template<typename>
struct make_parameter_tuple;

template<typename _tRet, typename... _tParams>
struct make_parameter_tuple<_tRet(_tParams...)>
{
	typedef std::tuple<_tParams...> type;
};

template<typename _tRet, typename... _tParams>
struct make_parameter_tuple<_tRet(*)(_tParams...)>
{
	typedef std::tuple<_tParams...> type;
};

template<typename _tRet, typename... _tParams>
struct make_parameter_tuple<_tRet(&)(_tParams...)>
{
	typedef std::tuple<_tParams...> type;
};

template<typename _tRet, class _tClass, typename... _tParams>
struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...)>
{
	typedef std::tuple<_tParams...> type;
};
//@}


/*!
\ingroup metafunctions
\brief 取返回类型。
\since build 333
*/
//@{
template<typename>
struct return_of;

template<typename Res, typename... _tParams>
struct return_of<Res(_tParams...)> {
    typedef Res type;
};

template<typename Res, typename... _tParams>
struct return_of<Res(*)(_tParams...)> {
    typedef Res type;
};

template<typename Res, typename... _tParams>
struct return_of<Res(&)(_tParams...)> {
    typedef Res type;
};

template<typename Res, class _tClass, typename... _tParams>
struct return_of<Res(_tClass::*)(_tParams...)> {
    typedef Res type;
};
//@}


/*!
\ingroup metafunctions
\brief 取指定索引的参数类型。
\since build 333
*/
template<size_t _vIdx, typename _fCallable>
struct parameter_of
{
	typedef typename std::tuple_element<_vIdx, typename
		make_parameter_tuple<_fCallable>::type>::type type;
};


/*!
\ingroup metafunctions
\brief 取参数列表大小。
\since build 333
*/
template<typename _fCallable>
struct paramlist_size : std::integral_constant<size_t, std::tuple_size<typename
	make_parameter_tuple<_fCallable>::type>::value>
{};


/*!	\defgroup functors General Functors
\brief 仿函数。
\note 函数对象包含函数指针和仿函数。
\since build 243
*/

/*!
\ingroup functors
\brief 引用相等关系仿函数。
*/
template<typename _type>
struct ref_eq
{
	yconstfn bool
	operator()(const _type& _x, const _type& _y) const
	{
		return &_x == &_y;
	}
};

/*!
\ingroup functors
\brief 编译期选择自增/自减运算仿函数。
*/
//@{
template<bool, typename _tScalar>
struct xcrease_t
{
	inline _tScalar&
	operator()(_tScalar& _x)
	{
		return ++_x;
	}
};
template<typename _tScalar>
struct xcrease_t<false, _tScalar>
{
	inline _tScalar&
	operator()(_tScalar& _x)
	{
		return --_x;
	}
};
//@}

/*!
\ingroup functors
\brief 编译期选择加法/减法复合赋值运算仿函数。
\since build 284
*/
//@{
template<bool, typename _tScalar1, typename _tScalar2>
struct delta_assignment
{
	yconstfn _tScalar1&
	operator()(_tScalar1& x, _tScalar2 y)
	{
		return x += y;
	}
};
template<typename _tScalar1, typename _tScalar2>
struct delta_assignment<false, _tScalar1, _tScalar2>
{
	yconstfn _tScalar1&
	operator()(_tScalar1& x, _tScalar2 y)
	{
		return x -= y;
	}
};
//@}

/*!
\ingroup helper_functions
\brief 编译期选择自增/自减运算。
*/
template<bool _bIsPositive, typename _tScalar>
yconstfn _tScalar&
xcrease(_tScalar& _x)
{
	return xcrease_t<_bIsPositive, _tScalar>()(_x);
}

/*!
\ingroup HelperFunctions
\brief 编译期选择加法/减法复合赋值运算。
\since build 284
*/
template<bool _bIsPositive, typename _tScalar1, typename _tScalar2>
yconstfn _tScalar1&
delta_assign(_tScalar1& _x, _tScalar2& _y)
{
	return delta_assignment<_bIsPositive, _tScalar1, _tScalar2>()(_x, _y);
}


/*!
\ingroup Functors
\brief 引用仿函数。
*/
template<typename _type>
struct deref_op
{
	/*!
	\brief 对指定对象使用 operator& 并返回结果。
	*/
	yconstfn _type*
	operator()(_type& _x) const
	{
		return &_x;
	}
};


/*!
\ingroup Functors
\brief const 引用仿函数。
*/
template<typename _type>
struct const_deref_op
{
	/*!
	\brief 对指定 const 对象使用 operator& 并返回结果。
	*/
	inline const _type*
	operator()(const _type& _x) const
	{
		return &_x;
	}
};


/*!
\ingroup Functors
\brief 间接访问比较仿函数。
\warning 非虚析构。
\since build 315
*/
template<typename _type, typename _tPointer = _type*,
	class _fCompare = std::less<_type>>
struct deref_comp
{
	/*
	\brief 比较指针指向的对象。
	\return 若参数有空指针则 false ，否则判断是否满足 _fCompare()(*_x, *_y) 。
	*/
	bool
	operator()(const _tPointer& _x, const _tPointer& _y) const
	{
		return _x && _y && _fCompare()(*_x, *_y);
	}
};


/*!
\ingroup Functors
\brief 间接访问字符串比较仿函数。
\warning 非虚析构。
\since build 315
*/
template<typename _tChar, class _fCompare = std::less<_tChar>>
struct deref_str_comp
{
	/*!
	\brief 比较指定字符串首字符的指针。
	\return 若参数有空指针则 false ，否则判断指定字符串是否满足字典序严格偏序关系。
	*/
	bool
	operator()(const _tChar* x, const _tChar* y) const
	{
		typedef std::char_traits<_tChar> traits_type;

		return x && y && std::lexicographical_compare(x, x + traits_type
			::length(x), y, y + traits_type::length(y), _fCompare());
	}
};

} // namespace ystdex;

#endif
