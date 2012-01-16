﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file TextBase.cpp
\ingroup Service
\brief 基础文本渲染逻辑对象。
\version r6894;
\author FrankHB<frankhb1989@gmail.com>
\since build 275 。
\par 创建时间:
	2009-11-13 00:06:05 +0800;
\par 修改时间:
	2012-01-15 17:46 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::Service::TextBase;
*/


#include "YSLib/Service/TextBase.h"
#include "YSLib/Service/yblit.h"

using namespace ystdex;

YSL_BEGIN

using namespace Drawing;
using namespace Text;

YSL_BEGIN_NAMESPACE(Drawing)

TextState::TextState(const Drawing::Font& font)
	: PenStyle(font),
	Margin(), PenX(0), PenY(0), LineGap(0)
{}
TextState::TextState(FontCache& fc)
	: PenStyle(Drawing::Font(*fc.GetDefaultTypefacePtr()->GetFontFamilyPtr())),
	Margin(), PenX(0), PenY(0), LineGap(0)
{}

/*Typeface*
TextState::SetFont(const char* name, CPATH filename)
{
	return pFont = GetCache().SetFont(name, filename);
}*/

void
TextState::PutNewline()
{
	CarriageReturn(*this);
	PenY += GetTextLineHeightExOf(*this);
}

void
TextState::ResetPen()
{
	CarriageReturn(*this);
	//	PenY = Margin.Top + GetTextLineHeightExOf(*this);
	//	PenY = Margin.Top + pCache->GetAscender();
	SetCurrentTextLineNOf(*this, 0);
}

void
TextState::ResetForBounds(const Rect& r, const Size& s, const Padding& m)
{
	Margin = FetchMargin(r + m, s);
	yunseq(PenX = r.X + m.Left,
		PenY = r.Y + GetCache().GetAscender() + m.Top);
}


void
SetCurrentTextLineNOf(TextState& s, u16 n)
{
	s.PenY = s.Margin.Top + s.GetCache().GetAscender()
		+ GetTextLineHeightExOf(s) * n;
}

void
MovePen(TextState& ts, ucs4_t c)
{
	FontCache& cache(ts.GetCache());
	CharBitmap sbit(cache.GetGlyph(c));

	ts.PenX += cache.GetAdvance(c, sbit);
}

YSL_END_NAMESPACE(Drawing)

YSL_BEGIN_NAMESPACE(Text)


YSL_END_NAMESPACE(Text)

YSL_END

