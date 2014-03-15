﻿/*
	© 2014 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file TextBox.cpp
\ingroup UI
\brief 样式无关的用户界面文本框。
\version r207
\author FrankHB <frankhb1989@gmail.com>
\since build 482
\par 创建时间:
	2014-03-02 16:21:22 +0800
\par 修改时间:
	2014-03-15 11:41 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YSLib::UI::TextBox
*/


#include "YSLib/UI/YModules.h"
#include YFM_YSLib_UI_TextBox
#include YFM_YSLib_UI_Border
#include YFM_YSLib_UI_YGUI
#include YFM_YSLib_Service_TextLayout
#include <ystdex/cast.hpp>

namespace YSLib
{

namespace UI
{

GAnimationSession<InvalidationUpdater> Caret::caret_animation;

Caret::Caret(IWidget& wgt, HBrush caret_brush,
	InvalidationUpdater::Invalidator inv)
	: CaretBrush(caret_brush), CursorInvalidator(inv)
{
	yunseq(
	FetchEvent<Paint>(wgt) += [this](PaintEventArgs&& e){
		if(Check(e.GetSender()))
			CaretBrush(std::move(e));
	},
	FetchEvent<GotFocus>(wgt) += [this](UIEventArgs&& e){
		Restart(caret_animation, e.GetSender(), CursorInvalidator);
	},
	FetchEvent<LostFocus>(wgt) += [this](UIEventArgs&&){
		Stop();
	}
	);
}
Caret::~Caret()
{
	Stop();
}

bool
Caret::Check(IWidget& sender)
{
	if(caret_animation.GetConnectionPtr()
		&& caret_animation.GetConnectionRef().Ready)
	{
		YAssert(IsFocusedCascade(sender), "Wrong focus state found.");

		return IsEnabled(sender)
			&& CaretTimer.RefreshRemainder() < CaretTimer.Interval / 2;
	}
	return {};
}

void
Caret::Stop()
{
	// TODO: Consider possible per-object optimization.
	caret_animation.Reset();
}


TextBox::TextBox(const Rect& r, const Drawing::Font& fnt)
	: Control(r, MakeBlankBrush()), MLabel(fnt),
	Selection(), CursorCaret(*this, std::bind(&TextBox::PaintDefaultCaret, this,
	std::placeholders::_1), InvalidateDefaultCaret), h_offset()
{
	yunseq(
	FetchEvent<TouchDown>(*this) += [this](CursorEventArgs&& e){
		Selection.Range.second = GetCaretPosition(e.Position);
		Selection.Collapse();
	},
	FetchEvent<TouchHeld>(*this) += [this](CursorEventArgs&& e){
		if(&e.GetSender() == this)
		{
			Selection.Range.second = GetCaretPosition(e.Position);
			// XXX: Optimization for block.
			Invalidate(*this);
		}
	},
	FetchEvent<Paint>(*this).Add(BorderBrush(), BackgroundPriority),
	FetchEvent<LostFocus>(*this) += [this](UIEventArgs&&){
		Selection.Collapse();
	}
	);
}

TextSelection::Position
TextBox::GetCaretPosition(const Point& pt)
{
	const SDst max_w(max(pt.X + h_offset - Margin.Left, 0));
	auto pr(FetchStringOffsets(max_w, Font, Text));

	if(pr.first > 0
		&& FetchCharWidth(Font, Text[pr.first - 1]) / 2 < pr.second - max_w)
		--pr.first;
	return {pr.first, 0};
}

void
TextBox::DrawClippedText(const Graphics& g, const Rect& mask, TextState& ts)
{
	auto p(&Text[0]);
	auto x1(Selection.Range.first.X), x2(Selection.Range.second.X);

	if(x1 == x2)
		MLabel::DrawClippedText(g, mask, ts);
	else
	{
		if(x2 < x1)
			std::swap(x1, x2);

		// TODO: Use C++14 lambda initializers to simplify implementation.
		const auto q1(p + x1), q2(p + x2);
		CustomTextRenderer ctr([=, &ts, &p](TextRenderer& tr, ucs4_t c){
			if(IsInInterval(p, q1, q2))
			{
				// TODO: Use colors from %Styles::Palette.
				FillRect(g, tr.ClipArea, Rect(ts.Pen.X,
					ts.Pen.Y - ts.Font.GetAscender(), ts.Font.GetAdvance(c),
					GetTextLineHeightOf(ts)), {51, 153, 255});
				ts.Color = ColorSpace::White;
			}
			else
				ts.Color = ForeColor;
			tr(c);
			++p;
		}, ts, g, mask);

		PutText(AutoWrapLine, ctr, p);
	}
}

void
TextBox::Refresh(PaintEventArgs&& e)
{
	DrawText(GetSizeOf(*this), ForeColor, e);
}

bool
TextBox::InvalidateDefaultCaret(IWidget& wgt)
{
	auto& tb(ystdex::polymorphic_downcast<TextBox&>(wgt));
	const Rect inner_bounds(Rect(GetSizeOf(wgt)) + tb.Margin);
	const auto lh(tb.Font.GetHeight());
	const auto& cur_pos(tb.Selection.Range.second);
	const auto
		x(inner_bounds.X + FetchStringWidth(tb.Font, tb.Text, cur_pos.X));
	const auto y(cur_pos.Y * lh + inner_bounds.X);

	InvalidateVisible(tb, Rect(x, y, 1, lh + GetVerticalOf(tb.Margin)));
	return true;
}

void
TextBox::PaintDefaultCaret(PaintEventArgs&& e)
{
	const Rect inner_bounds(Rect(e.Location, GetSizeOf(*this)) + Margin);
	const auto lh(Font.GetHeight());
	const auto mask(FetchMargin(inner_bounds, e.Target.GetSize()));
	const auto& cur_pos(Selection.Range.second);

	DrawVLineSeg(e.Target, e.ClipArea, inner_bounds.X + FetchStringWidth(Font,
		Text, cur_pos.X) - h_offset, cur_pos.Y * lh + mask.Top,
		cur_pos.Y * lh + lh + mask.Top, ForeColor);
}

} // namespace UI;

} // namespace YSLib;

