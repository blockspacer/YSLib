﻿/*
	Copyright (C) by Franksoft 2010 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ystyle.cpp
\ingroup UI
\brief 图形用户界面样式。
\version r1591;
\author FrankHB<frankhb1989@gmail.com>
\since build 194 。
\par 创建时间:
	2010-05-01 13:52:56 +0800;
\par 修改时间:
	2012-03-17 20:20 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::YStyle;
*/


#include "YSLib/UI/ystyle.h"
#include "YSLib/UI/ywindow.h"

using namespace ystdex;

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Drawing)

bool
DrawRectRoundCorner(const Graphics& g, const Point& pt, const Size& s, Color c)
{
	const SPos x1(pt.X + 1), y1(pt.Y + 1), x2(pt.X + s.Width - 1),
		y2(pt.Y + s.Height - 1);

	if(YCL_LIKELY(x1 <= x2 && y1 <= y2))
	{
		bool b(DrawVLineSeg(g, x1 - 1, y1, y2, c));

		b |= DrawHLineSeg(g, y2, x1, x2, c);
		b |= DrawVLineSeg(g, x2, y1, y2, c);
		b |= DrawHLineSeg(g, y1 - 1, x1, x2, c);
		if(YCL_LIKELY(s.Width > 4 && s.Height > 4))
		{
			DrawPoint(g, x1, y1, c);
			DrawPoint(g, x1, y2 - 1, c);
			DrawPoint(g, x2 - 1, y2 - 1, c);
			DrawPoint(g, x2 - 1, y1, c);
		}
		return b;
	}
	return false;
}


void
RectDrawArrow(const Graphics& g, const Point& pt, SDst half_size, Rotation rot,
	Color c)
{
	YAssert(g.IsValid(), "Invalid graphics context found"
		" @ Drawing::RectDrawArrow");

	SDst x(pt.X), y(pt.Y);

	switch(rot)
	{
	case RDeg0:
		{
			SDst t(pt.Y);

			for(SDst i(0); i < half_size; ++i)
				DrawVLineSeg(g, x--, y--, t++, c);
		}
		break;
	case RDeg90:
		{
			SDst t(pt.X);

			for(SDst i(0); i < half_size; ++i)
				DrawHLineSeg(g, y++, x--, t++, c);
		}
		break;
	case RDeg180:
		{
			SDst t(pt.Y);

			for(SDst i(0); i < half_size; ++i)
				DrawVLineSeg(g, x++, y--, t++, c);
		}
		break;
	case RDeg270:
		{
			SDst t(pt.X);

			for(SDst i(0); i < half_size; ++i)
				DrawHLineSeg(g, y--, x--, t++, c);
		}
	default:
		break;
	}
}

void
WndDrawArrow(const Graphics& g, const Rect& r, SDst half_size, Rotation rot,
	Color c)
{
	SPos x(r.X), y(r.Y);

	switch(rot)
	{
	case RDeg0:
	case RDeg180:
		x += (rot == RDeg180
			? (r.Width - half_size) : (r.Width + half_size)) / 2;
		y += (r.Height + 1) / 2;
		break;
	case RDeg90:
	case RDeg270:
		y += (rot == RDeg90
			? (r.Height - half_size) : (r.Height + half_size)) / 2;
		x += (r.Width + 1) / 2;
	default:
		break;
	}
	RectDrawArrow(g, Point(x, y), half_size, rot, c);
}


hsl_t
ColorToHSL(Color c)
{
	typedef float mid_t; //中间类型。

	const u8 r(c.GetR()), g(c.GetG()), b(c.GetB()),
		min_color(min(min(r, g), b)), max_color(max(max(r, g), b));
	mid_t h(0); // 此处 h 的值每 0x6 对应一个圆周。
	mid_t s(0);
	decltype(hsl_t::l) l;

	if(min_color == max_color)
		l = decltype(hsl_t::l)(min_color) / 0x100;
	else
	{
		const unsigned p(max_color + min_color);

		l = decltype(hsl_t::l)(p) / 0x200;
	/*
		l = 0.2126 * r + 0.7152 * g + 0.0722 * b; // Rec. 601 luma;
		l = 0.299 * r + 0.588 * g + 0.114 * b; // Rec. 709 luma;
	*/

		// chroma * 256;
		const mid_t q(max_color - min_color);

		s = q / (p < 0x100 ? p : 0x200 - p);
		if(r == max_color)
			h = (g - b) / q;
		else if(g == max_color)
			h = (b - r) / q + 0x2;
		else if(b == max_color)
			h = (r - g) / q + 0x4;
		if(h < 0)
			h += 0x6;
	}

	const hsl_t res = {h * 60, s, l};

	return res;
}

Color
HSLToColor(hsl_t c)
{
	if(c.s == 0)
		return Color(c.l * 0x100, c.l * 0x100, c.l * 0x100);

	typedef float mid_t; //中间类型。

	mid_t t2((c.l < 0.5 ? c.l * (1 + c.s) : (c.l + c.s - c.l * c.s)) * 0x100),
		t1((c.l * 0x200) - t2);
	mid_t t3(c.h); //角度制，即值 360 对应一个圆周。
	mid_t tmp[3] = {t3 + 120, t3, t3 - 120}; \
		// tmp 每个元素对应一个 RGB 分量，值 360 对应一个圆周。
	float dc[3]; //对应 RGB 分量。

	for(size_t i(0); i < 3; ++i)
	{
		if(tmp[i] < 0)
			tmp[i] += 360;
		else if(tmp[i] > 360)
			tmp[i] -= 360;
		if(tmp[i] < 60)
			dc[i] = t1 + (t2 - t1) * tmp[i] / 60;
		else if(tmp[i] < 180)
			dc[i] = t2;
		else if(tmp[i] < 240)
			dc[i] = t1 + (t2 - t1) * (240 - tmp[i]) / 60;
		else
			dc[i] = t1;
	}
	return Color(dc[0], dc[1], dc[2]);
}

YSL_END_NAMESPACE(Drawing)

YSL_BEGIN_NAMESPACE(Components)

YSL_BEGIN_NAMESPACE(Styles)

Palette::Palette()
	: colors(EndArea)
{
	using Drawing::Color;

	// TODO: use initializer-list;
	yunseq
	(
		colors[Null] = Color(0, 0, 0),
		colors[Desktop] = Color(10, 59, 118),
		colors[Window] = Color(255, 255, 255),
		colors[Panel] = Color(240, 240, 240),
		colors[Track] = Color(237, 237, 237),
		colors[Workspace] = Color(171, 171, 171),
		colors[Shadow] = Color(160, 160, 160),
		colors[DockShadow] = Color(105, 105, 105),
		colors[Light] = Color(227, 227, 227),
		colors[Frame] = Color(100, 100, 100),
		colors[Highlight] = Color(51, 153, 255),
		colors[BorderFill] = Color(158, 62, 255),
		colors[ActiveBorder] = ColorSpace::Aqua,
		colors[InactiveBorder] = Color(180, 180, 180),
	//	colors[ActiveBorder] = Color(180, 180, 180),
	//	colors[InactiveBorder] = Color(244, 247, 252),
		colors[ActiveTitle] = Color(153, 180, 209),
		colors[InactiveTitle] = Color(191, 205, 219),

		colors[HighlightText] = Color(255, 255, 255),
		colors[WindowText] = Color(0, 0, 0),
		colors[PanelText] = Color(0, 0, 0),
		colors[GrayText] = Color(109, 109, 109),
		colors[TitleText] = Color(0, 0, 0),
		colors[InactiveTitleText] = Color(67, 78, 84),
		colors[HotTracking] = Color(0, 102, 204)
	);

	//"GradientActiveTitle"="185 209 234"
	//"GradientInactiveTitle"="215 228 242"
}

pair<Drawing::Color, Drawing::Color>
Palette::GetPair(Palette::ColorListType::size_type n1,
	Palette::ColorListType::size_type n2) const
{
	return make_pair(colors[n1], colors[n2]);
}

YSL_END_NAMESPACE(Styles)

YSL_END_NAMESPACE(Components)

YSL_END
