﻿/*
	Copyright (C) by Franksoft 2010 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\defgroup UI UI
\ingroup YSLib
\brief YSLib UI 模块。
*/

/*!	\file ycomp.h
\ingroup UI
\brief 平台无关的 Shell 组件。
\version r3267;
\author FrankHB<frankhb1989@gmail.com>
\since 早于 build 132 。
\par 创建时间:
	2010-03-19 20:05:08 +0800;
\par 修改时间:
	2012-03-16 15:40 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::YComponent;
*/


#ifndef YSL_INC_UI_YCOMP_H_
#define YSL_INC_UI_YCOMP_H_

#include "../Core/ygdibase.h"
#include "../Core/yfunc.hpp"
#include "../Adaptor/ycont.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Drawing)

YSL_END_NAMESPACE(Drawing)

YSL_BEGIN_NAMESPACE(Components)

//前向声明。
class AController;
FwdDeclI(IWidget)
/*!
\since build 294 。
*/
class ImageBrush;
/*!
\since build 293 。
*/
struct InputEventArgs;
/*!
\since build 293 。
*/
struct KeyEventArgs;
/*!
\since build 293 。
*/
struct TouchEventArgs;
/*!
\since build 293 。
*/
struct PaintContext;
/*!
\since build 293 。
*/
struct PaintEventArgs;
/*!
\since build 293 。
*/
class Renderer;
/*!
\since build 293 。
*/
struct RoutedEventArgs;
/*!
\since build 294 。
*/
class SolidBrush;
/*!
\since build 293 。
*/
struct UIEventArgs;
/*!
\since build 293 。
*/
class WidgetController;
class Window;


//类型别名。
/*!
\brief 画刷回调函数。
\since build 293 。
*/
typedef std::function<void(PaintEventArgs&&)> HBrush;


//名称引用。
using Drawing::PixelType;
using Drawing::BitmapPtr;
using Drawing::ConstBitmapPtr;
using Drawing::ScreenBufferType;
using Drawing::Color;

using Drawing::Point;
using Drawing::Vec;
using Drawing::Size;
using Drawing::Rect;

using Drawing::Graphics;

YSL_END_NAMESPACE(Components)

YSL_END

#endif
