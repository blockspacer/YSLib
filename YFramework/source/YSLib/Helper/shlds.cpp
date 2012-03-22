﻿/*
	Copyright (C) by Franksoft 2010 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file shlds.cpp
\ingroup Helper
\ingroup DS
\brief Shell 类库 DS 版本。
\version r2032;
\author FrankHB<frankhb1989@gmail.com>
\since 早于 build 132 。
\par 创建时间:
	2010-03-13 14:17:14 +0800;
\par 修改时间:
	2012-03-20 16:17 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::Helper::Shell_DS;
*/


#include "YSLib/Helper/shlds.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Shells)

using namespace Messaging;

int
ShlCLI::ExecuteCommand(const ucs2_t*)
{
	// TODO: impl;
	return 0;
}


YSL_END_NAMESPACE(Shells)


YSL_BEGIN_NAMESPACE(DS)

ShlDS::ShlDS()
	: Shell(),
	hDskUp(make_shared<Desktop>(FetchGlobalInstance().GetScreenUp())),
	hDskDown(make_shared<Desktop>(FetchGlobalInstance().GetScreenDown())),
	bUpdateUp(), bUpdateDown()
{
	YAssert(bool(hDskUp), "Null up desktop handle found @ ShlDS::ShlDS;");
	YAssert(bool(hDskDown), "Null down desktop handle found @ ShlDS::ShlDS;");

	YSL_ Components::FetchGUIState().Reset();
}

int
ShlDS::OnGotMessage(const Message& msg)
{
	switch(msg.GetMessageID())
	{
	case SM_PAINT:
#if 0
		{
			const auto h(FetchTarget<SM_PAINT>(msg));
			
			if(h)
				h->Refresh(PaintContext(h->GetContext(), Point::Zero,
					Rect(Point::Zero, GetSizeOf(*h))));
		}
#endif
		{
			using Drawing::Rect;

			yunseq(bUpdateUp = hDskUp->Validate() != Rect::Empty,
				bUpdateDown = hDskDown->Validate() != Rect::Empty);
			OnPaint();
			if(bUpdateUp)
				hDskUp->Update();
			if(bUpdateDown)
				hDskDown->Update();
		}
		return 0;
	case SM_INPUT:
		//平台相关输入处理。
		{
			const auto& content(Messaging::FetchTarget<SM_INPUT>(msg));

			const KeysInfo& k(content.Keys);
			Desktop& d(*hDskDown); // TODO: assertion & etc;
		//	Desktop& d(FetchGlobalInstance().GetTouchableDesktop());

			using namespace YSL_ KeySpace;
			using namespace YSL_ Components;

			auto& st(FetchGUIState());

			if(k.Up & Touch)
			{
				TouchEventArgs e(d, content.CursorLocation);

				st.ResponseTouch(e, TouchUp);
			}
			else if(k.Up)
			{
				KeyEventArgs e(d, k.Up);

				st.ResponseKey(e, KeyUp);
			}
			if(k.Down & Touch)
			{
				TouchEventArgs e(d, content.CursorLocation);

				st.ResponseTouch(e, TouchDown);
			}
			else if(k.Down)
			{
				KeyEventArgs e(d, k.Down);

				st.ResponseKey(e, KeyDown);
			}
			if(k.Held & Touch)
			{
				TouchEventArgs e(d, content.CursorLocation);

				st.ResponseTouch(e, TouchHeld);
			}
			else if(k.Held)
			{
				KeyEventArgs e(d, k.Held);

				st.ResponseKey(e, KeyHeld);
			}
		}
		OnInput();
		return 0;
	default:
		break;
	}
	return Shell::OnGotMessage(msg);
}

void
ShlDS::OnInput()
{
	SendMessage<SM_PAINT>(FetchShellHandle(), 0xE0, nullptr);
}

void
ShlDS::OnPaint()
{}

YSL_END_NAMESPACE(DS)

YSL_END
