﻿/*
	© 2010-2016, 2019 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file YComponent.h
\ingroup UI
\brief 平台中立的 UI 组件。
\version r2424
\author FrankHB <frankhb1989@gmail.com>
\since build 426
\par 创建时间:
	2010-03-19 20:05:08 +0800
\par 修改时间:
	2019-01-06 14:11 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YSLib::UI::YComponent
*/


#ifndef YSL_INC_UI_YComponent_h_
#define YSL_INC_UI_YComponent_h_ 1

#include "YModules.h"
#include YFM_YSLib_Core_YGraphics // for ystdex::optional_function;
#include YFM_YSLib_Core_YFunc
#include YFM_YSLib_Core_YMessageDefinition
#include <typeinfo> // for mandated header;

namespace YSLib
{

/*!	\defgroup DataModel Data Models
\ingroup UI
\brief 数据模型。
*/

namespace Drawing
{

//! \since build 319
class Image;

} // namespace Drawing;

namespace UI
{

class AController;
//! \since build 422
struct CursorEventArgs;
//! \since build 422
class GUIState;
FwdDeclI(IWidget)
//! \since build 293
//@{
struct InputEventArgs;
struct KeyEventArgs;
struct PaintEventArgs;
class Renderer;
struct RoutedEventArgs;
struct UIEventArgs;
class WidgetController;
//@}
//! \since build 573
class Panel;


//类型别名。
/*!
\brief 画刷回调函数。
\since build 293
*/
using HBrush = ystdex::optional_function<void(PaintEventArgs&&)>;


//! \since build 559
using Drawing::Pixel;
using Drawing::BitmapPtr;
using Drawing::ConstBitmapPtr;
using Drawing::Color;

using Drawing::Point;
using Drawing::Vec;
using Drawing::Size;
using Drawing::Rect;

using Drawing::Graphics;
//! \since build 366
using Drawing::PaintContext;

} // namespace UI;

//! \since build 388
namespace Messaging
{

//! \since build 454
DefMessageTarget(SM_Paint, shared_ptr<UI::IWidget>)

} // namespace Messaging;

} // namespace YSLib;

#endif

