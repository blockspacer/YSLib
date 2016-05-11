﻿/*
	© 2012-2016 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file any_iterator.hpp
\ingroup YStandardEx
\brief 动态泛型迭代器。
\version r1304
\author FrankHB <frankhb1989@gmail.com>
\since build 355
\par 创建时间:
	2012-11-08 14:28:42 +0800
\par 修改时间:
	2016-05-11 00:17 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YStandardEx::AnyIterator
*/


#ifndef YB_INC_ystdex_any_iterator_hpp_
#define YB_INC_ystdex_any_iterator_hpp_ 1

#include "any.h" // for "any.h", any_ops, unwrap_reference_t, cond_t,
//	is_reference_wrapper, ref_handler, _t. ptrdiff_t, any, exclude_self_t,
//	any_ops::with_handler_t, decay_t, is_convertible, indirect_t;
#include "iterator.hpp" // for is_undereferenceable, input_iteratable,
//	forward_iteratable, bidirectional_iteratable;

namespace ystdex
{

namespace any_ops
{

//! \since build 355
//@{
enum iterator_op : op_code
{
	//! \since build 400
	check_undereferenceable = end_base_op,
	dereference,
	increase,
	end_iterator_op
};


enum input_iterator_op : op_code
{
	equals = end_iterator_op,
	end_input_iterator_op,
	end_output_iterator_op = end_input_iterator_op,
	end_forward_iterator_op = end_input_iterator_op
};


enum bidirectional_iteartor_op : op_code
{
	decrease = end_forward_iterator_op,
	end_bidirectional_iterator_op
};


enum random_access_iteartor_op : op_code
{
	advance = end_forward_iterator_op,
	distance,
	less_compare,
	end_random_access_iteartor_op
};


template<typename _type>
struct wrap_handler
{
	using value_type = unwrap_reference_t<_type>;
	using type = cond_t<is_reference_wrapper<_type>, ref_handler<value_type>,
		value_handler<value_type>>;
};


template<typename _type>
class iterator_handler : public wrap_handler<_type>::type
{
public:
	using base = _t<wrap_handler<_type>>;
	using value_type = typename base::value_type;

	using base::get_reference;

	using base::init;

	//! \since build 692
	static void
	manage(any_storage& d, any_storage& s, any_ops::op_code op)
	{
		switch(op)
		{
		case check_undereferenceable:
			d.access<bool>() = is_undereferenceable(get_reference(s));
			break;
		case dereference:
			d = void_ref(*get_reference(s));
			break;
		case increase:
			++get_reference(d);
			break;
		default:
			base::manage(d, s, op);
		}
	}
};


template<typename _type>
class input_iterator_handler : public iterator_handler<_type>
{
public:
	using base = iterator_handler<_type>;
	using value_type = typename base::value_type;

	using base::get_reference;

	using base::init;

	//! \since build 692
	static void
	manage(any_storage& d, any_storage& s, any_ops::op_code op)
	{
		switch(op)
		{
		case equals:
			{
				const auto p(d.access<any_storage*>());

				d.access<bool>() = get_reference(*p) == get_reference(s);
			}
			break;
		default:
			base::manage(d, s, op);
		}
	}
};
//@}


//! \since build 400
template<typename _type>
class forward_iterator_handler : public input_iterator_handler<_type>
{
public:
	using base = input_iterator_handler<_type>;
	using value_type = typename base::value_type;

	using base::get_reference;

	using base::init;

	using base::manage;
};


//! \since build 400
template<typename _type>
class bidirectional_iterator_handler : public forward_iterator_handler<_type>
{
public:
	using base = forward_iterator_handler<_type>;
	using value_type = typename base::value_type;

	using base::get_reference;

	using base::init;

	//! \since build 692
	static void
	manage(any_storage& d, any_storage& s, any_ops::op_code op)
	{
		switch(op)
		{
		case decrease:
			--get_reference(d);
			break;
		default:
			base::manage(d, s, op);
		}
	}
};

} // namespace any_ops;


//! \since build 400
//@{
/*!
\ingroup iterator_adaptors
\brief 动态泛型输入迭代器。
\warning 非虚析构。
*/
template<typename _type, typename _tDifference = ptrdiff_t,
	typename _tPointer = _type*, typename _tReference = _type&>
class any_input_iterator : protected any,
	public input_iteratable<any_input_iterator<_type, _tDifference, _tPointer,
	_tReference>, _tReference>
{
protected:
	//! \since build 686
	using base = any;

public:
	//! \since build 676
	//@{
	using iterator_category = std::input_iterator_tag;
	using value_type = _type;
	using difference_type = _tDifference;
	//@}
	using pointer = _tPointer;
	using reference = _tReference;

	//! \since build 357
	any_input_iterator() = default;
	/*!
	\brief 构造：使用现有迭代器。
	\since build 675
	*/
	template<typename _tIter,
		yimpl(typename = exclude_self_t<any_input_iterator, _type>)>
	any_input_iterator(_tIter&& i)
		: any_input_iterator(any_ops::with_handler_t<
		any_ops::input_iterator_handler<decay_t<_tIter>>>(), yforward(i))
	{}

protected:
	//! \since build 686
	template<typename _tIter, typename _tHandler>
	any_input_iterator(any_ops::with_handler_t<_tHandler> t, _tIter&& i)
		: base(t, yforward(i))
	{
		static_assert(is_convertible<indirect_t<unwrap_reference_t<
			decay_t<_tIter>>&>, reference>::value,
			"Wrong target iterator type found.");
	}

public:
	//! \since build 356
	any_input_iterator(const any_input_iterator&) = default;
	//! \since build 356
	any_input_iterator(any_input_iterator&&) = default;

	//! \since build 454
	//@{
	any_input_iterator&
	operator=(const any_input_iterator&) = default;
	any_input_iterator&
	operator=(any_input_iterator&&) = default;
	//@}

	reference
	operator*() const
	{
		return reference(unchecked_access<void_ref>(any_ops::dereference));
	}

	pointer
	operator->() const
	{
		return &**this;
	}

	any_input_iterator&
	operator++()
	{
		call(get_storage(), any_ops::increase);
		return *this;
	}

	//! \since build 676
	friend bool
	operator==(const any_input_iterator& x, const any_input_iterator& y)
	{
		if(x.empty() && y.empty())
			return true;
		yassume(x.type() == y.type());

		any_ops::any_storage t(&x.get_storage());

		return y.template unchecked_access<bool>(t, any_ops::equals);
	}

	//! \since build 615
	//@{
	any&
	get() ynothrow
	{
		return *this;
	}
	const any&
	get() const ynothrow
	{
		return *this;
	}
	//@}

	/*!
	\ingroup is_undereferenceable
	\since build 676
	*/
	friend bool
	is_undereferenceable(const any_input_iterator& i)
	{
		return !i.empty() ? i.template unchecked_access<bool>(
			any_ops::check_undereferenceable) : true;
	}

	//! \since build 615
	using any::target;

	//! \since build 355
	using any::type;
};


using input_monomorphic_iterator
	= any_input_iterator<void_ref, ptrdiff_t, void*, void_ref>;
//@}


/*!
\ingroup iterator_adaptors
\brief 动态泛型前向迭代器。
*/
template<typename _type, typename _tDifference = ptrdiff_t,
	typename _tPointer = _type*, typename _tReference = _type&>
class any_forward_iterator : public any_input_iterator<_type, _tDifference,
	_tPointer, _tReference>, public forward_iteratable<any_forward_iterator<
	_type, _tDifference, _tPointer, _tReference>, _tReference>
{
protected:
	//! \since build 686
	using base
		= any_input_iterator<_type, _tDifference, _tPointer, _tReference>;

public:
	using iterator_category = std::forward_iterator_tag;
	using pointer = _tPointer;
	using reference = _tReference;

	any_forward_iterator() = default;
	/*!
	\brief 构造：使用现有迭代器。
	\since build 686
	*/
	template<typename _tIter,
		yimpl(typename = exclude_self_t<any_forward_iterator, _type>)>
	any_forward_iterator(_tIter&& i)
		: any_forward_iterator(any_ops::with_handler_t<
		any_ops::forward_iterator_handler<decay_t<_tIter>>>(), yforward(i))
	{
		static_assert(is_convertible<indirect_t<unwrap_reference_t<
			decay_t<_tIter>>&>, reference>::value,
			"Wrong target iterator type found.");
	}

protected:
	//! \since build 686
	template<typename _tIter, typename _tHandler>
	any_forward_iterator(any_ops::with_handler_t<_tHandler> t, _tIter&& i)
		: base(t, yforward(i))
	{}

public:
	any_forward_iterator(const any_forward_iterator&) = default;
	any_forward_iterator(any_forward_iterator&&) = default;

	//! \since build 454
	//@{
	any_forward_iterator&
	operator=(const any_forward_iterator&) = default;
	any_forward_iterator&
	operator=(any_forward_iterator&&) = default;
	//@}

	any_forward_iterator&
	operator++()
	{
		base::operator++();
		return *this;
	}

	//! \since build 676
	friend bool
	operator==(const any_forward_iterator& x, const any_forward_iterator& y)
	{
		return static_cast<const base&>(x) == static_cast<const base&>(y);
	}
};


using forward_monomorphic_iterator
	= any_forward_iterator<void_ref, ptrdiff_t, void*, void_ref>;


/*!
\ingroup iterator_adaptors
\brief 动态泛型双向迭代器。
*/
template<typename _type, typename _tDifference = ptrdiff_t,
	typename _tPointer = _type*, typename _tReference = _type&>
class any_bidirectional_iterator : public any_forward_iterator<_type,
	_tDifference, _tPointer, _tReference>, public bidirectional_iteratable<
	any_bidirectional_iterator<_type, _tDifference, _tPointer, _tReference>,
	_tReference>
{
protected:
	//! \since build 686
	using base
		= any_forward_iterator<_type, _tDifference, _tPointer, _tReference>;

public:
	using iterator_category = std::bidirectional_iterator_tag;
	using pointer = _tPointer;
	using reference = _tReference;

	any_bidirectional_iterator() = default;
	/*!
	\brief 构造：使用现有迭代器。
	\since build 686
	*/
	template<typename _tIter>
	any_bidirectional_iterator(_tIter&& i)
		: any_forward_iterator<_type, _tPointer, _tReference>(yforward(i))
	{}

protected:
	//! \since build 686
	template<typename _tIter, typename _tHandler>
	any_bidirectional_iterator(any_ops::with_handler_t<_tHandler> t, _tIter&& i)
		: base(t, yforward(i))
	{}

public:
	any_bidirectional_iterator(const any_bidirectional_iterator&) = default;
	any_bidirectional_iterator(any_bidirectional_iterator&&) = default;

	//! \since build 454
	//@{
	any_bidirectional_iterator&
	operator=(const any_bidirectional_iterator&) = default;
	any_bidirectional_iterator&
	operator=(any_bidirectional_iterator&&) = default;
	//@}

	any_bidirectional_iterator&
	operator++()
	{
		base::operator++();
		return *this;
	}

	any_bidirectional_iterator&
	operator--()
	{
		this->call(this->get_storage(), any_ops::decrease);
		return *this;
	}

	//! \since build 676
	friend bool
	operator==(const any_bidirectional_iterator& x,
		const any_bidirectional_iterator& y)
	{
		return static_cast<const base&>(x) == static_cast<const base&>(y);
	}
};


using bidirectional_monomorphic_iterator
	= any_bidirectional_iterator<void_ref, ptrdiff_t, void*, void_ref>;
//@}

} // namespace ystdex;

#endif

