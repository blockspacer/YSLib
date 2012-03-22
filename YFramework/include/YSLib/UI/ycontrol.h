﻿/*
	Copyright (C) by Franksoft 2010 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ycontrol.h
\ingroup UI
\brief 样式无关的控件。
\version r5525;
\author FrankHB<frankhb1989@gmail.com>
\since 早于 build 132 。
\par 创建时间:
	2010-02-18 13:44:24 +0800;
\par 修改时间:
	2012-03-15 16:54 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::YComponent;
*/


#ifndef YSL_INC_UI_YCONTROL_H_
#define YSL_INC_UI_YCONTROL_H_

#include "ywidget.h"
#include "ywgtevt.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

using namespace Drawing;

/*!
\brief 部件控制器。

保存部件的事件响应策略和状态。
*/
class Controller : public AController
{
protected:
	mutable EventMapping::MapType EventMap; //!< 事件映射表。

public:
	explicit
	Controller(bool);
	Controller(bool, const EventMapping::MapType&);
	Controller(bool, EventMapping::MapType&&);

	ImplI(AController) PDefH(EventMapping::ItemType&, GetItem,
		const VisualEvent& id)
		ImplRet(EventMap.at(id))
	virtual EventMapping::ItemType&
	GetItemRef(const VisualEvent&, EventMapping::MappedType(&)());
	DefGetter(const ynothrow, EventMapping::MapType&, EventMap, EventMap) \
		//!< 取事件映射表。

	ImplI(AController) DefClone(Controller, Clone)
};

inline
Controller::Controller(bool b)
	: AController(b), EventMap()
{}
inline
Controller::Controller(bool b, const EventMapping::MapType& m)
	: AController(b), EventMap(m)
{}
inline
Controller::Controller(bool b, EventMapping::MapType&& m)
	: AController(b), EventMap(std::move(m))
{}


/*!
\brief 判断部件是否为可用的控件。
*/
inline bool
IsEnabled(const IWidget& wgt)
{
	try
	{
		return wgt.GetController().IsEnabled();
	}
	catch(BadEvent&)
	{}
	return false;
}

/*!
\brief 判断部件是否为可用且可见的控件。
*/
inline bool
IsEnabledAndVisible(const IWidget& wgt)
{
	return IsEnabled(wgt) && IsVisible(wgt);
}

/*!
\brief 设置部件可用性。
*/
inline void
SetEnabledOf(IWidget& wgt, bool b)
{
	try
	{
		wgt.GetController().SetEnabled(b);
	}
	catch(BadEvent&)
	{}
}

/*!
\brief 设置部件可用性，且当可用性改变时无效化部件区域。
\return 可用性是否改变。
\since build 286 。
*/
bool
Enable(IWidget&, bool = true);


/*!
\brief 构造指针指向的 VisualEvent 指定的事件对象。
*/
template<VisualEvent _vID>
EventMapping::MappedType
NewEvent()
{
	return EventMapping::MappedType(new GEventWrapper<EventT(typename
		EventTypeMapping<_vID>::HandlerType), UIEventArgs>());
}

/*!
\brief 在事件映射表中取指定 id 对应的事件。
*/
EventMapping::ItemType&
GetEvent(EventMapping::MapType&, const VisualEvent&,
	EventMapping::MappedType(&)());

/*!
\ingroup HelperFunctions
\brief 取控件事件。
\note 需要确保 EventTypeMapping 中有对应的 EventType ，否则无法匹配此函数模板。
\note 若控件事件不存在则自动添加空事件。
*/
template<VisualEvent _vID>
EventT(typename EventTypeMapping<_vID>::HandlerType)&
FetchEvent(VisualEventMap& m)
{
	return dynamic_cast<EventT(typename EventTypeMapping<_vID>::HandlerType)&>(
		GetEvent(m, _vID, NewEvent<_vID>));
}
/*!
\ingroup HelperFunctions
\brief 取部件事件。
\tparam _vID 指定事件类型。
\param controller 指定部件的控制器。
\exception BadControl 异常中立：无事件映射表：由 GetController 抛出。
\note 需要确保 EventTypeMapping 中有对应的 EventType ，否则无法匹配此函数模板。
\note 若控件事件不存在则自动添加空事件。
\since build 195 。
*/
template<VisualEvent _vID>
EventT(typename EventTypeMapping<_vID>::HandlerType)&
FetchEvent(AController& controller)
{
	return dynamic_cast<EventT(typename EventTypeMapping<_vID>::HandlerType)&>(
		controller.GetItemRef(_vID, NewEvent<_vID>));
}
/*!
\ingroup HelperFunctions
\brief 取部件事件。
\tparam _vID 指定事件类型。
\param wgt 指定部件。
\exception BadControl 异常中立：无事件映射表：由 GetController 抛出。
\note 需要确保 EventTypeMapping 中有对应的 EventType ，否则无法匹配此函数模板。
\note 若控件事件不存在则自动添加空事件。
\since build 237 。
*/
template<VisualEvent _vID>
inline EventT(typename EventTypeMapping<_vID>::HandlerType)&
FetchEvent(IWidget& wgt)
{
	return FetchEvent<_vID>(wgt.GetController());
}

/*!
\brief 调用部件事件，并忽略 BadControl 异常。
\note 需要确保 EventTypeMapping 中有对应的 EventType ，否则无法匹配此函数模板。
\note 若控件事件不存在则忽略。
*/
template<VisualEvent _vID, typename _tEventArgs>
inline size_t
CallEvent(IWidget& wgt, _tEventArgs&& e)
{
	static_assert(std::is_same<typename std::remove_reference<_tEventArgs>
		::type, typename EventTypeMapping<_vID>::HandlerType::EventArgsType>
		::value, "Invalid event argument type found @ CallEvent;");

	try
	{
		return DoEvent<typename EventTypeMapping<_vID>::HandlerType>(
			wgt.GetController(), _vID, yforward(e));
	}
	catch(BadEvent&)
	{}
	return 0;
}


//公共事件处理器。

/*
\brief 处理事件：停止事件路由。
\since build 285 。
*/
template<class _tEventArgs>
inline void
OnEvent_StopRouting(_tEventArgs&& e)
{
	e.Handled = true;
}

/*!
\brief 处理键接触保持事件。
*/
void
OnKeyHeld(KeyEventArgs&&);

/*!
\brief 处理屏幕接触开始事件：使用一般接触策略。

屏幕接触时依次进行以下判断：
当事件路由策略非 RoutedEventArgs::Bubble 时请求置于顶端；
当事件路由策略为 RoutedEventArgs::Direct 时清除自身包含的焦点；
当事件路由策略非 RoutedEventArgs::Tunnel 时请求置于顶端。
\since build 275 。
*/
void
OnTouchDown_RequestToTopFocused(TouchEventArgs&&);

/*!
\brief 处理屏幕接触保持事件。

实现记录坐标偏移（用于拖放）或触发 TouchMove 事件。
*/
void
OnTouchHeld(TouchEventArgs&&);

/*!
\brief 处理屏幕接触移动事件。
\note 重复触发 TouchDown 事件。
*/
void
OnTouchMove(TouchEventArgs&&);

/*!
\brief 处理屏幕接触移动事件。
\note 使用拖放。
\note 无效化当前区域，但不无效化移动后区域；通过 Move 事件可以调用无效化。
*/
void
OnTouchMove_Dragging(TouchEventArgs&&);


// Control 事件处理器。

/*!
\brief 处理按键事件：按键-指针设备接触结束。

绑定触发 TouchUp 和 Leave 事件。
\note 仅对 Control 及其派生类有效。
*/
void
OnKey_Bound_TouchUpAndLeave(KeyEventArgs&&);

/*!
\brief 处理按键事件：按键-指针设备接触开始。

绑定触发 Enter 和 TouchDown 事件。
\note 仅对 Control 及其派生类有效。
*/
void
OnKey_Bound_EnterAndTouchDown(KeyEventArgs&&);

/*!
\brief 处理按键事件：按键-指针设备按下。

绑定触发 Click 事件。
\note 仅对 Control 及其派生类有效。
*/
void
OnKey_Bound_Click(KeyEventArgs&&);


/*!
\brief 控件。
\since build 168 。
*/
class Control : public Widget
{
protected:
	/*!
	\brief 扩展控件事件表。
	\note 加载 Components::OnTouchHeld 作为 TouchHeld 事件处理器。
	\since build 240 。
	*/
	DefExtendEventMap(ControlEventMap, VisualEventMap)

public:
	//标准控件事件见 VisualEvent 。

	//其它扩展控件事件示例。
//	DeclEvent(H1Event, Ext1) //!< 扩展事件 1 。
//	DeclEvent(H2Event, Ext2) //!< 扩展事件 2 。

	//事件组映射。
	/*!
	\brief 按键-指针设备输入事件组映射。
	\note 默认为 Control::GetBoundControlPtr 。

	转换按键输入事件为指定部件的指针设备输入事件。
	*/
	std::function<IWidget*(const KeyCode&)> BoundControlPtr;

	/*!
	\brief 构造：使用指定边界。
	\note 使用扩展控件事件表加载的事件处理器。
	\note 加载事件处理器： Move 、 Resize 、 GotFocus 和 LostFocus 时自动无效化。
	\note 加载事件处理器： OnTouchDown_RequestToTopFocused 。
	*/
	explicit
	Control(const Rect& = Rect::Empty);
	/*!
	\brief 复制构造：除容器为空外深复制。
	*/
	Control(const Control&);
	DefDeMoveCtor(Control)

	/*!
	\brief 取按键-指针设备输入默认事件组映射。
	*/
	virtual PDefH(IWidget*, GetBoundControlPtr, const KeyCode&)
		ImplRet(nullptr)

	/*!
	\brief 处理屏幕事件：关闭显示。

	调用 Components::Close 。
	*/
	void
	OnTouch_Close(TouchEventArgs&&);
};

YSL_END_NAMESPACE(Components)

YSL_END

#endif
