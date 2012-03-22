﻿/*
	Copyright (C) by Franksoft 2011 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file memory.hpp
\ingroup YStandardEx
\brief 存储和智能指针特性。
\version r1337;
\author FrankHB<frankhb1989@gmail.com>
\since build 209 。
\par 创建时间:
	2011-05-14 12:25:13 +0800;
\par 修改时间:
	2012-03-12 11:04 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YStandardEx::Memory;
*/


#ifndef YCL_INC_YSTDEX_MEMORY_HPP_
#define YCL_INC_YSTDEX_MEMORY_HPP_

#include "../ydef.h"
#include <memory>
#include <cstring> // for std::memcpy and std::memmove;

namespace ystdex
{
	/*!	\defgroup is_dereferencable Is Dereferencable Iterator
	\brief 判断迭代器实例是否确定可解引用。
	\tparam _tIterator 迭代器类型。
	\note 注意返回 \c false 不表示参数实际不可解引用。
	\note 默认实现对参数转换为 \c bool 类型判断是否为 true 。
	\since build 249 。
	*/
	//@{
	template<typename _tIterator>
	yconstfn bool
	is_dereferencable(const _tIterator&)
	{
		return false;
	}
	template<typename _type>
	yconstfn bool
	is_dereferencable(_type* p)
	{
		return bool(p);
	}
	//@}


	/*!	\defgroup is_undereferencable Is Undereferencable Iterator
	\brief 判断迭代器实例是否为可解引用。
	\tparam _tIterator 迭代器类型。
	\note 注意返回 \c false 不表示参数实际可解引用。
	\note 默认实现对参数转换为 \c bool 类型判断是否为 false 。
	\since build 250 。
	*/
	//@{
	template<typename _tIterator>
	yconstfn bool
	is_undereferencable(const _tIterator&)
	{
		return false;
	}
	template<typename _type>
	yconstfn bool
	is_undereferencable(_type* p)
	{
		return !bool(p);
	}
	//@}


	/*!	\defgroup raw Get Raw Pointers
	\brief 取内建指针。
	\since build 204 。
	*/
	//@{
	template<typename _type>
	yconstfn _type*
	raw(_type* const& p)
	{
		return p;
	}
	template<typename _type>
	yconstfn _type*
	raw(const std::unique_ptr<_type>& p)
	{
		return p.get();
	}
	template<typename _type>
	yconstfn _type*
	raw(const std::shared_ptr<_type>& p)
	{
		return p.get();
	}
	//@}

	/*!	\defgroup reset Reset Pointers
	\brief 安全删除指定引用的指针指向的对象。
	\post 指定引用的指针为空。
	\since build 209 。
	*/
	//@{
	template<typename _type>
	inline bool
	reset(std::unique_ptr<_type>& p) ynothrow
	{
		if(p.get())
		{
			p.reset();
			return true;
		}
		return false;
	}
	template<typename _type>
	inline bool
	reset(std::shared_ptr<_type>& p) ynothrow
	{
		if(p.get())
		{
			p.reset();
			return true;
		}
		return false;
	}
	//@}


	/*!	\defgroup unique_raw Get Unique Pointer
	\ingroup helper_functions
	\brief 使用指定类型指针构造 std::unique_ptr 实例。
	\tparam _type 被指向类型。
	\note 不检查指针是否有效。
	\since build 212 。
	*/
	//@{
	/*!
	\tparam _pSrc 指定指针类型。
	*/
	template<typename _type, typename _pSrc>
	yconstfn std::unique_ptr<_type>
	unique_raw(const _pSrc& p)
	{
		return std::unique_ptr<_type>(p);
	}
	/*!
	\tparam _pSrc 指定指针类型。
	*/
	template<typename _type, typename _pSrc>
	yconstfn std::unique_ptr<_type>
	unique_raw(_pSrc&& p)
	{
		return std::unique_ptr<_type>(p);
	}
	template<typename _type>
	yconstfn std::unique_ptr<_type>
	unique_raw(_type* p)
	{
		return std::unique_ptr<_type>(p);
	}
	/*!
	\note 使用空指针构造空实例。
	*/
	template<typename _type>
	yconstfn std::unique_ptr<_type>
	unique_raw(const nullptr_t& p)
	{
		return std::unique_ptr<_type>();
	}
	//@}


	/*!	\defgroup share_raw Get Shared Pointer
	\ingroup helper_functions
	\brief 使用指定类型指针构造 std::shared_ptr 实例。
	\tparam _type 被指向类型。
	\note 不检查指针是否有效。
	\since build 204 。
	*/
	//@{
	/*!
	\tparam _pSrc 指定指针类型。
	*/
	template<typename _type, typename _pSrc>
	yconstfn std::shared_ptr<_type>
	share_raw(const _pSrc& p)
	{
		return std::shared_ptr<_type>(p);
	}
	/*!
	\tparam _pSrc 指定指针类型。
	*/
	template<typename _type, typename _pSrc>
	yconstfn std::shared_ptr<_type>
	share_raw(_pSrc&& p)
	{
		return std::shared_ptr<_type>(p);
	}
	template<typename _type>
	yconstfn std::shared_ptr<_type>
	share_raw(_type* p)
	{
		return std::shared_ptr<_type>(p);
	}
	/*!
	\note 使用空指针构造空实例。
	*/
	template<typename _type>
	yconstfn std::shared_ptr<_type>
	share_raw(const nullptr_t& p)
	{
		return std::shared_ptr<_type>();
	}
	//@}


	/*!
	\ingroup helper_functions
	\brief 使用全局 new 和指定参数构造指定类型的 std::unique_ptr 实例。
	\tparam _type 被指向类型。
	\since build 293 。
	*/
	template<typename _type, typename... _tParams>
	yconstfn std::unique_ptr<_type>
	make_unique(_tParams&&... args)
	{
		return std::unique_ptr<_type>(::new _type(yforward(args)...));
	}
}

#endif
