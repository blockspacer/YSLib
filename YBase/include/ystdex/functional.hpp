﻿/*
	Copyright by FrankHB 2012 - 2013.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file functional.hpp
\ingroup YStandardEx
\brief 函数和可调用对象。
\version r588
\author FrankHB <frankhb1989@gmail.com>
\since build 333
\par 创建时间:
	2010-08-22 13:04:29 +0800
\par 修改时间:
	2013-07-02 06:27 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YStandardEx::Functional
*/


#ifndef YB_INC_ystdex_functional_hpp_
#define YB_INC_ystdex_functional_hpp_ 1

#include "utility.hpp" // for ../ydef.h, ystdex::decay_forward;
#include "variadic.hpp" // for ystdex::variadic_sequence,
//	ystdex::make_natural_sequence;
#include <functional>
#include <tuple>
#include <string> // for std::char_traits;

namespace ystdex
{

/*!
\brief 统计函数参数列表中的参数个数。
\since build 360 。
*/
template<typename... _tParams>
yconstfn size_t
sizeof_params(_tParams&&...)
{
	return sizeof...(_tParams);
}


//! \since build 412
//@{
//! \brief 变长参数操作模板。
//@{
template<size_t _vN>
struct variadic_param
{
	template<typename _type, typename... _tParams>
	yconstfn static auto
	get(_type&&, _tParams&&... args)
		-> decltype(variadic_param<_vN - 1>::get(yforward(args)...))
	{
		static_assert(sizeof...(args) == _vN,
			"Wrong variadic arguments number found.");

		return variadic_param<_vN - 1>::get(yforward(args)...);
	}
};

template<>
struct variadic_param<0U>
{
	template<typename _type>
	yconstfn static auto
	get(_type&& arg) -> decltype(yforward(arg))
	{
		return yforward(arg);
	}
};
//@}


/*!
\brief 取指定位置的变长参数。
\tparam _vN 表示参数位置的非负数，从左开始计数，第一个参数为 0 。
*/
template<size_t _vN, typename... _tParams>
yconstexpr auto
varg(_tParams&&... args)
	-> decltype(variadic_param<_vN>::get(yforward(args)...))
{
	static_assert(_vN < sizeof...(args),
		"Out-of-range index of variadic argument found.");

	return variadic_param<_vN>::get(yforward(args)...);
}
//@}


/*!
\brief 顺序递归调用。
\see 关于调用参数类型： ISO C++11 30.3.1.2[thread.thread.constr] 。
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
	(ystdex::decay_forward(yforward(f)))(ystdex::decay_forward(yforward(arg))),
		ystdex::seq_apply(yforward(f), yforward(args)...);
}
//@}


/*!
\brief 无序调用。
\see 关于调用参数类型： ISO C++11 30.3.1.2[thread.thread.constr] 。
\since build 327
*/
template<typename _fCallable, typename... _tParams>
inline void
unseq_apply(_fCallable&& f, _tParams&&... args)
{
	yunseq(((ystdex::decay_forward(yforward(f)))(
		ystdex::decay_forward(yforward(args))), 0)...);
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

// !\since build 358
template<typename _tRet, class _tClass, typename... _tParams>
struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...) const>
{
	typedef std::tuple<_tParams...> type;
};

// !\since build 358
template<typename _tRet, class _tClass, typename... _tParams>
struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...) volatile>
{
	typedef std::tuple<_tParams...> type;
};

// !\since build 358
template<typename _tRet, class _tClass, typename... _tParams>
struct make_parameter_tuple<_tRet(_tClass::*)(_tParams...) const volatile>
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

template<typename _tResult, typename... _tParams>
struct return_of<_tResult(_tParams...)>
{
    typedef _tResult type;
};

template<typename _tResult, typename... _tParams>
struct return_of<_tResult(*)(_tParams...)>
{
    typedef _tResult type;
};

template<typename _tResult, typename... _tParams>
struct return_of<_tResult(&)(_tParams...)>
{
    typedef _tResult type;
};

template<typename _tResult, class _tClass, typename... _tParams>
struct return_of<_tResult(_tClass::*)(_tParams...)>
{
    typedef _tResult type;
};

// !\since build 358
template<typename _tResult, class _tClass, typename... _tParams>
struct return_of<_tResult(_tClass::*)(_tParams...) const>
{
    typedef _tResult type;
};

// !\since build 358
template<typename _tResult, class _tClass, typename... _tParams>
struct return_of<_tResult(_tClass::*)(_tParams...) volatile>
{
    typedef _tResult type;
};

// !\since build 358
template<typename _tResult, class _tClass, typename... _tParams>
struct return_of<_tResult(_tClass::*)(_tParams...) const volatile>
{
    typedef _tResult type;
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


/*!
\ingroup unary_type_trait
\brief 取 std::reference_wrapper 实例特征。
\since build 348
*/
//@{
template<typename _type>
struct wrapped_traits : public std::false_type
{
	typedef _type type;
};

template<typename _tWrapped>
struct wrapped_traits<std::reference_wrapper<_tWrapped>> : public std::true_type
{
	typedef _tWrapped type;
};
//@}


/*!
\brief 解除引用包装。
\note 默认只对于 std::reference_wrapper 的实例类型的对象重载。
\note 使用 ADL 。
\since build 348
*/
//@{
template<typename _type>
_type&
unref(_type&& x) ynothrow
{
	return x;
}
template<typename _type>
_type&
unref(const std::reference_wrapper<_type>& x) ynothrow
{
	return x.get();
}
//@}


/*!	\defgroup hash_extensions Hash Extensions
\brief 散列扩展接口。
\note 当前使用 Boost 定义的接口和近似实现。
\see http://www.boost.org/doc/libs/1_54_0/doc/html/hash/reference.html#boost.hash_combine
\since build 421
*/
//@{
/*!
\brief 重复计算散列。
\note <tt>(1UL << 31) / ((1 + std::sqrt(5)) / 4) == 0x9E3779B9</tt> 。
\warning 实现（ Boost 文档作为 Effects ）可能改变，不应作为接口依赖。
*/
template<typename _type>
inline void
hash_combine(size_t& seed, const _type& val)
{
	seed ^= std::hash<_type>()(val) + 0x9E3779B9 + (seed << 6) + (seed >> 2);
}

/*!
\ingroup helper_functions
\brief 重复计算序列散列。
\sa hash_combine
*/
//@{
template<typename _type>
inline size_t
hash_combine_seq(size_t seed, const _type& val)
{
	ystdex::hash_combine(seed, val);

	return seed;
}
template<typename _type, typename... _tParams>
inline size_t
hash_combine_seq(size_t seed, const _type& x, const _tParams&... args)
{
	return ystdex::hash_combine_seq(ystdex::hash_combine_seq(seed, x), args...);
}
//@}

/*!
\brief 重复对范围计算散列。
\note 使用 ADL 。
*/
//@{
template<typename _tIn>
inline size_t
hash_range(_tIn first, _tIn last)
{
	size_t seed(0);

	for(; first != last; ++first)
		hash_combine(seed, *first);
	return seed;
}
template<typename _tIn>
inline size_t
hash_range(size_t& seed, _tIn first, _tIn last)
{
	for(; first != last; ++first)
		 hash_combine(seed, *first);
	return seed;
}
//@}
//@}


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
	operator()(_tScalar1& x, _tScalar2 y) const
	{
		return x += y;
	}
};
template<typename _tScalar1, typename _tScalar2>
struct delta_assignment<false, _tScalar1, _tScalar2>
{
	yconstfn _tScalar1&
	operator()(_tScalar1& x, _tScalar2 y) const
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
\ingroup helper_functions
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
\ingroup functors
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
\ingroup functors
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
\ingroup functors
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
		return bool(_x) && bool(_y) && _fCompare()(*_x, *_y);
	}
};


/*!
\ingroup functors
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

