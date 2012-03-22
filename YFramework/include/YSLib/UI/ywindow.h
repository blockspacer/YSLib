﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ywindow.h
\ingroup UI
\brief 样式无关的图形用户界面窗口。
\version r4781;
\author FrankHB<frankhb1989@gmail.com>
\since 早于 build 132 。
\par 创建时间:
	2009-12-28 16:46:40 +0800;
\par 修改时间:
	2012-03-19 15:25 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::YWindow;
*/


#ifndef YSL_INC_UI_YWINDOW_H_
#define YSL_INC_UI_YWINDOW_H_

#include "ypanel.h"
#include "../Service/yblit.h"
#include "YBrush.h"

YSL_BEGIN

#undef YWindowAssert

YSL_BEGIN_NAMESPACE(Components)

/*!
\brief 窗口。
\since build 264 。
*/
class Window : public Panel
{
public:
	/*!
	\brief 构造：使用指定边界和背景图像。
	*/
	explicit
	Window(const Rect& = Rect::Empty,
		const shared_ptr<Drawing::Image>& = make_shared<Drawing::Image>());
	inline DefDeMoveCtor(Window)

	virtual void
	operator+=(IWidget&);
	virtual void
	operator+=(Window&);
	template<class _type>
	inline void
	operator+=(_type& p)
	{
		return operator+=(typename std::conditional<
			std::is_convertible<_type&, Window&>::value,
			Window&, IWidget&>::type(p));
	}

	virtual bool
	operator-=(IWidget&);
	virtual bool
	operator-=(Window&);
	template<class _type>
	inline bool
	operator-=(_type& p)
	{
		return operator-=(typename std::conditional<
			std::is_convertible<_type&, Window&>::value,
			Window&, IWidget&>::type(p));
	}

	using MUIContainer::Contains;

	virtual PDefH(IWidget*, GetTopWidgetPtr, const Point& pt,
		bool(&f)(const IWidget&))
		ImplBodyBase(MUIContainer, GetTopWidgetPtr, pt, f)

	virtual void
	Add(IWidget&, ZOrderType = DefaultZOrder);
};

YSL_END_NAMESPACE(Components)

YSL_END

#endif
