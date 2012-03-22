﻿/*
	Copyright (C) by Franksoft 2011 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file uicontx.cpp
\ingroup UI
\brief 样式无关的图形用户界面附加容器。
\version r1180;
\author FrankHB<frankhb1989@gmail.com>
\since build 192 。
\par 创建时间:
	2011-02-21 09:01:13 +0800;
\par 修改时间:
	2012-03-18 21:46 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::UIContainerEx;
*/


#include "YSLib/UI/uicontx.h"
#include "YSLib/UI/ygui.h"
#include "YSLib/UI/YBrush.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

DialogBox::DialogBox(const Rect& r)
	: Control(r),
	btnClose(Rect(GetWidth() - 20, 4, 16, 16))
{
	SetContainerPtrOf(btnClose, this),
	FetchEvent<Paint>(*this).Add(BorderBrush(), BackgroundPriority);
}

IWidget*
DialogBox::GetTopWidgetPtr(const Point& pt, bool(&f)(const IWidget&))
{
	if(auto p = CheckWidget(btnClose, pt, f))
		return p;
	return nullptr;
}

void
DialogBox::Refresh(PaintEventArgs&& e)
{
	PaintChild(btnClose, e);
	e.ClipArea = Rect(e.Location, GetSizeOf(*this));
}


DialogPanel::DialogPanel(const Rect& r)
	: Panel(r),
	btnClose(Rect(GetWidth() - 20, 4, 16, 16)),
	btnOK(Rect(GetWidth() - 40, 4, 16, 16))
{
	*this += btnClose,
	*this += btnOK,
	yunseq(
		btnOK.Text = "○",
		FetchEvent<Paint>(*this).Add(BorderBrush(), BackgroundPriority),
		FetchEvent<Click>(btnOK) += [this](TouchEventArgs&&){
			Close(*this);
		}
	);
}

YSL_END_NAMESPACE(Components)

YSL_END
