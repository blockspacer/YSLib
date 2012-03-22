﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file yrender.cpp
\ingroup UI
\brief 样式无关的图形用户界面部件渲染器。
\version r1558;
\author FrankHB<frankhb1989@gmail.com>
\since build 237 。
\par 创建时间:
	2011-09-03 23:46:22 +0800;
\par 修改时间:
	2012-03-14 09:32 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YSLib::UI::YRenderer;
*/


#include "YSLib/UI/yrender.h"
#include "YSLib/UI/ycontrol.h"
#include "YSLib/Service/ydraw.h"

YSL_BEGIN

YSL_BEGIN_NAMESPACE(Components)

Rect
Renderer::Paint(IWidget& wgt, PaintEventArgs&& e)
{
	YAssert(&e.GetSender().GetRenderer() == this,
		"Invalid widget found @ Render::Paint;");

	CallEvent<Components::Paint>(wgt, e);
	return e.ClipArea;
}


bool
BufferedRenderer::RequiresRefresh() const
{
	return !rInvalidated.IsEmpty();
}

void
BufferedRenderer::SetSize(const Size& s)
{
	Buffer.SetSize(s.Width, s.Height);
	static_cast<Size&>(rInvalidated) = s;
}

Rect
BufferedRenderer::CommitInvalidation(const Rect& r)
{
	return rInvalidated = Unite(rInvalidated, r);
}

Rect
BufferedRenderer::Paint(IWidget& wgt, PaintEventArgs&& e)
{
	YAssert(&e.GetSender().GetRenderer() == this,
		"Invalid widget found @ BufferedRenderer::Paint;");

	Rect r(Validate(wgt, e.GetSender(), e));

	UpdateTo(e);
	return r;
}

void
BufferedRenderer::UpdateTo(const PaintContext& pc) const
{
	const auto& g(pc.Target);
	const Rect& r(pc.ClipArea);

	CopyTo(g.GetBufferPtr(), GetContext(), g.GetSize(), r,
		r.GetPoint() - pc.Location, r);
}

Rect
BufferedRenderer::Validate(IWidget& wgt, IWidget& sender,
	const PaintContext& pc)
{
	if(RequiresRefresh())
	{
		const auto& l(GetLocationOf(sender));
		const Rect& clip(Intersect(pc.ClipArea, rInvalidated + l));

		if(!IgnoreBackground && FetchContainerPtr(sender))
		{
			const auto& g(GetContext());

			CopyTo(g.GetBufferPtr(), pc.Target, g.GetSize(),
				clip.GetPoint() - pc.Location, clip, clip);
		}

		PaintEventArgs e(sender, PaintContext(GetContext(), Point::Zero,
			clip - l));

		CallEvent<Components::Paint>(wgt, e);
		//清除无效区域：只设置一个分量为零可能会使 CommitInvalidation 结果错误。
		static_cast<Size&>(rInvalidated) = Size::Zero;
		return e.ClipArea;
	}
	return Rect::Empty;
}

YSL_END_NAMESPACE(Components)

YSL_END
