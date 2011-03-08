﻿/*
	Copyright (C) by Franksoft 2010 - 2011.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ycontrol.cpp
\ingroup Shell
\brief 平台无关的控件实现。
\version 0.4053;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2010-02-18 13:44:34 +0800;
\par 修改时间:
	2011-03-06 22:19 +0800;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::Shell::YComponent;
*/


#include "ycontrol.h"
#include "ygui.h"
#include "../Helper/yshelper.h"
#include "yuicont.h"
#include "ywindow.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Controls)

void
OnKeyHeld(IControl& c, KeyEventArgs& e)
{
	GHHandle<YGUIShell> hShl(FetchGUIShellHandle());

	if(hShl->RepeatHeld(hShl->KeyHeldState, 240, 120))
		FetchEvent<KeyDown>(c)(c, e);
}

void
OnTouchHeld(IControl& c, TouchEventArgs& e)
{
	GHHandle<YGUIShell> hShl(FetchGUIShellHandle());

	if(hShl->DraggingOffset == Vec::FullScreen)
		hShl->DraggingOffset = c.GetLocation() - hShl->ControlLocation;
	else
		FetchEvent<TouchMove>(c)(c, e);
	hShl->LastControlLocation = hShl->ControlLocation;
}

void
OnTouchMove(IControl& c, TouchEventArgs& e)
{
	GHHandle<YGUIShell> hShl(FetchGUIShellHandle());

	if(hShl->RepeatHeld(hShl->TouchHeldState, 240, 60))
		FetchEvent<TouchDown>(c)(c, e);
}

void
OnTouchMove_Dragging(IControl& c, TouchEventArgs&)
{
	GHHandle<YGUIShell> hShl(FetchGUIShellHandle());

	if(hShl->LastControlLocation != hShl->ControlLocation)
	{
		c.SetLocation(hShl->LastControlLocation + hShl->DraggingOffset);
		c.Refresh();
	}
}


Control::Control(const Rect& r, IUIBox* pCon)
	: Widget(r, pCon), AFocusRequester(), enabled(true), EventMap()
{
	FetchEvent<GotFocus>(*this) += &Control::OnGotFocus;
	FetchEvent<LostFocus>(*this) += &Control::OnLostFocus;
	FetchEvent<TouchDown>(*this) += &Control::OnTouchDown;
	FetchEvent<TouchHeld>(*this) += OnTouchHeld;

	IUIContainer* p(dynamic_cast<IUIContainer*>(GetContainerPtr()));

	if(p)
	{
		*p += static_cast<IWidget&>(*this);
		*p += static_cast<IControl&>(*this);
	}
}
Control::~Control() ythrow()
{
	ReleaseFocus(GetStaticRef<EventArgs>());
	IUIContainer* p(dynamic_cast<IUIContainer*>(GetContainerPtr()));

	if(p)
	{
		*p -= static_cast<IWidget&>(*this);
		*p -= static_cast<IControl&>(*this);
	}
}

bool
Control::IsFocused() const
{
	IUIBox* p(GetContainerPtr());

	return p ? p->GetFocusingPtr() == this : false;
}

void
Control::SetLocation(const Point& pt)
{
	Visual::SetLocation(pt);
	GetEventMap().DoEvent<EventTypeMapping<Move>::HandlerType>(Move,
		*this, GetStaticRef<EventArgs>());
}
void
Control::SetSize(const Size& s)
{
	Visual::SetSize(s);
	GetEventMap().DoEvent<EventTypeMapping<Resize>::HandlerType>(Resize,
		*this, GetStaticRef<EventArgs>());
}

void
Control::RequestFocus(EventArgs& e)
{
	IUIBox* p(GetContainerPtr());

	if(p && p->ResponseFocusRequest(*this))
		EventMap.GetEvent<HVisualEvent>(GotFocus)(*this, e);
}

void
Control::ReleaseFocus(EventArgs& e)
{
	IUIBox* p(GetContainerPtr());

	if(p && p->ResponseFocusRelease(*this))
		EventMap.GetEvent<HVisualEvent>(LostFocus)(*this, e);
}

void
Control::OnGotFocus(EventArgs&)
{
	Refresh();
}

void
Control::OnLostFocus(EventArgs&)
{
	Refresh();
}

void
Control::OnTouchDown(TouchEventArgs& e)
{
	RequestFocus(e);
}


YControl::YControl(const Rect& r, IUIBox* pCon)
	: YComponent(),
	Control(r, pCon)
{}

YSL_END_NAMESPACE(Controls)

YSL_END_NAMESPACE(Components)

YSL_END
