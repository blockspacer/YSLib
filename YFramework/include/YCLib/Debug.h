﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Debug.h
\ingroup YCLib
\brief YCLib 调试设施。
\version r1039;
\author FrankHB<frankhb1989@gmail.com>
\since build 299 。
\par 创建时间:
	2012-04-07 14:20:49 +0800;
\par 修改时间:
	2012-04-07 14:28 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YCLib::Debug;
*/


#ifndef YCL_INC_DEBUG_H_
#define YCL_INC_DEBUG_H_

#include "ycommon.h"

namespace platform
{

/*!
\brief 调试模式：设置状态。
\note 当且仅当状态为 true 时，
	以下除 YDebugGetStatus 外的调试模式函数有效。
*/
void
YDebugSetStatus(bool = true);

/*!
\brief 调试模式：取得状态。
*/
bool
YDebugGetStatus();

/*!
\brief 调试模式：显示控制台（fc 为前景色，bc 为背景色）。
*/
void
YDebugBegin(Color fc = ColorSpace::White, Color bc = ColorSpace::Blue);

/*!
\brief 调试模式：按键继续。
*/
void
YDebug();
/*!
\brief 调试模式：显示控制台字符串，按键继续。
*/
void
YDebug(const char*);

/*!
\brief 调试模式 printf ：显示控制台格式化输出 ，按键继续。
*/
int
yprintf(const char*, ...)
	YCL_ATTRIBUTE((format (printf, 1, 2)));

} // namespace platform_ex;

#endif

