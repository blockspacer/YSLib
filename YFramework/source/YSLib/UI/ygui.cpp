﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ygui.cpp
\ingroup UI
\brief 平台无关的图形用户界面。
\version r4167;
\author FrankHB<frankhb1989@gmail.com>
\since 早于 build 132 。
\par 创建时间:
	2009-11-16 20:06:58 +0800;
\par 修改时间:
	2012-03-17 20:19 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::YGUI;
*/


#include "YSLib/UI/ygui.h"
#include "YSLib/UI/ydesktop.h"

YSL_BEGIN

using namespace Drawing;
using namespace Components;

YSL_BEGIN_NAMESPACE(Components)

GUIState::GUIState() ynothrow
	: KeyHeldState(Free), TouchHeldState(Free),
	DraggingOffset(Vec::Invalid), HeldTimer(Timers::TimeSpan(1000U)),
	ControlLocation(Point::Invalid),
	LastControlLocation(Point::Invalid), Colors(),
	p_KeyDown(), p_TouchDown(), control_entered(false)
{}

bool
GUIState::RepeatHeld(HeldStateType& s,
	Timers::TimeSpan InitialDelay, Timers::TimeSpan RepeatedDelay)
{
	//三状态自动机。
	switch(s)
	{
	case Free:
		/*
		必须立即转移状态，否则 Activate(HeldTimer) 会使 HeldTimer.Refresh()
		始终为 false ，导致状态无法转移。
		*/
		s = Pressed;
		HeldTimer.SetInterval(InitialDelay); //初始键按下延迟。
		Activate(HeldTimer);
		break;

	case Pressed:
	case Held:
		if(YCL_UNLIKELY(HeldTimer.Refresh()))
		{
			if(s == Pressed)
			{
				s = Held;
				HeldTimer.SetInterval(RepeatedDelay); //重复键按下延迟。
			}
			return true;
		}
		break;
	}
	return false;
}

void
GUIState::Reset()
{
	yunseq(KeyHeldState = Free, TouchHeldState = Free,
		DraggingOffset = Vec::Invalid);
	HeldTimer.SetInterval(Timers::TimeSpan(1000));
	Deactivate(HeldTimer);
	yunseq(ControlLocation = Point::Invalid,
		LastControlLocation = Point::Invalid,
		p_TouchDown = nullptr, p_KeyDown = nullptr);
}

void
GUIState::ResetHeldState(HeldStateType& s)
{
	Deactivate(HeldTimer);
	s = Free;
}

void
GUIState::TryEntering(TouchEventArgs&& e)
{
	if(!control_entered)
	{
		CallEvent<Enter>(e.GetSender(), e);
		control_entered = true;
	}
}

void
GUIState::TryLeaving(TouchEventArgs&& e)
{
	if(control_entered)
	{
		CallEvent<Leave>(e.GetSender(), e);
		control_entered = false;
	}
}

bool
GUIState::ResponseKeyBase(KeyEventArgs& e, Components::VisualEvent op)
{
	auto& wgt(e.GetSender());

	switch(op)
	{
	case KeyUp:
		ResetHeldState(KeyHeldState);
		CallEvent<KeyUp>(wgt, e);
		if(p_KeyDown == &wgt)
		{
			CallEvent<KeyPress>(wgt, e);
			p_KeyDown = nullptr;
		}
		break;
	case KeyDown:
		p_KeyDown = &wgt;
		CallEvent<KeyDown>(wgt, e);
		break;
	case KeyHeld:
	/*	if(e.Strategy == RoutedEventArgs::Direct && p_KeyDown != &c)
		{
			ResetHeldState(KeyHeldState);
			return false;
		}*/
		CallEvent<KeyHeld>(wgt, e);
		break;
	default:
		YAssert(false, "Invalid operation found @ GUIState::ResponseKeyBase;");
	}
	return true;
}

bool
GUIState::ResponseTouchBase(TouchEventArgs& e, Components::VisualEvent op)
{
	auto& wgt(e.GetSender());

	switch(op)
	{
	case TouchUp:
		ResetHeldState(TouchHeldState);
		DraggingOffset = Vec::Invalid;
		CallEvent<TouchUp>(wgt, e);
		if(p_TouchDown)
		{
			e.SetSender(*p_TouchDown);
			TryLeaving(std::move(e));
			e.SetSender(wgt);
		}
		if(p_TouchDown == &wgt)
		{
			CallEvent<Click>(wgt, e);
			p_TouchDown = nullptr;
		}
		break;
	case TouchDown:
		p_TouchDown = &wgt;
		TryEntering(std::move(e));
		CallEvent<TouchDown>(wgt, e);
		break;
	case TouchHeld:
		if(!p_TouchDown)
			return false;
		if(p_TouchDown == &wgt)
			TryEntering(TouchEventArgs(e));
		else
			TryLeaving(TouchEventArgs(*p_TouchDown,
				e - LocateForWidget(wgt, *p_TouchDown)));
		CallEvent<TouchHeld>(*p_TouchDown, e);
		break;
	default:
		YAssert(false, "Invalid operation found"
			" @ GUIState::ResponseTouchBase;");
	}
	return true;
}

bool
GUIState::ResponseKey(KeyEventArgs& e, Components::VisualEvent op)
{
	auto p(&e.GetSender());
	IWidget* pCon;
	bool r(false);

	e.Strategy = Components::RoutedEventArgs::Tunnel;
	while(true)
	{
		if(!(IsVisible(*p) && IsEnabled(*p)))
			return false;
		if(e.Handled)
			return true;
		pCon = p;

		auto t(FetchFocusingPtr(*pCon));

		if(!t || t == pCon)
			break;
		e.SetSender(*p);
		r |= ResponseKeyBase(e, op);
		p = t;
	}

	YAssert(p, "Null pointer found @ GUIState::ResponseKey");

	e.Strategy = Components::RoutedEventArgs::Direct;
	e.SetSender(*p);
	r |= ResponseKeyBase(e, op);
	e.Strategy = Components::RoutedEventArgs::Bubble;
	while(!e.Handled && (pCon = FetchContainerPtr(*p)))
	{
		p = pCon;
		e.SetSender(*p);
		r |= ResponseKeyBase(e, op);
	}
	return r;
}

bool
GUIState::ResponseTouch(TouchEventArgs& e, Components::VisualEvent op)
{
	ControlLocation = e;

	auto p(&e.GetSender());
	IWidget* pCon;
	bool r(false);

	e.Strategy = Components::RoutedEventArgs::Tunnel;
	while(true)
	{
		if(!(IsVisible(*p) && IsEnabled(*p)))
			return false;
		if(e.Handled)
			return true;
		pCon = p;

		auto t(pCon->GetTopWidgetPtr(e, IsEnabledAndVisible));

		if(!t || t == pCon)
			break;
		e.SetSender(*p);
		r |= DoEvent<HTouchEvent>(p->GetController(), op, e) != 0;
		p = t;
		e -= GetLocationOf(*p);
	};

	YAssert(p, "Null pointer found @ GUIState::ResponseTouch");

	e.Strategy = Components::RoutedEventArgs::Direct;
	e.SetSender(*p);
	r |= ResponseTouchBase(e, op);
	e.Strategy = Components::RoutedEventArgs::Bubble;
	while(!e.Handled && (pCon = FetchContainerPtr(*p)))
	{
		e += GetLocationOf(*p);
		p = pCon;
		e.SetSender(*p);
		r |= DoEvent<HTouchEvent>(p->GetController(), op, e) != 0;
	}
	return r;
}


GUIState&
FetchGUIState()
{
	static GUIState* pState(new GUIState());

	return *pState;
}

YSL_END_NAMESPACE(Components)

YSL_END
