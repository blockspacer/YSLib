﻿/*
	© 2013-2014 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Host.cpp
\ingroup Helper
\brief 宿主环境。
\version r1420
\author FrankHB <frankhb1989@gmail.com>
\since build 379
\par 创建时间:
	2013-02-08 01:27:29 +0800
\par 修改时间:
	2014-06-28 16:30 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	Helper::Host
*/


#include "Helper/YModules.h"
#include YFM_Helper_Host
#include YFM_Helper_ShellHelper // for YSL_DEBUG_DECL_TIMER;
#include YFM_YCLib_Input // for platform_ex::FetchCursor;
#if YCL_Android
#	include YFM_Android_Helper_AndroidHost // for Android::FetchDefaultWindow;
#endif

namespace YSLib
{

using namespace Drawing;

namespace
{

#if YCL_Win32 && 0
yconstexpr double g_max_free_fps(1000);
std::chrono::nanoseconds host_sleep(u64(1000000000 / g_max_free_fps));
#endif

} // unnamed namespace;


#if YF_Hosted
using namespace Host;

namespace
{

#	if YCL_Win32
::LRESULT CALLBACK
WndProc(::HWND h_wnd, unsigned msg, ::WPARAM w_param, ::LPARAM l_param)
{
	if(const auto p = reinterpret_cast<Window*>(::GetWindowLongPtrW(h_wnd,
		GWLP_USERDATA)))
	{
		YSL_DEBUG_DECL_TIMER(tmr, std::to_string(msg));
		auto& m(p->MessageMap);
		const auto i(m.find(msg));

		if(i != m.cend())
		{
			i->second(w_param, l_param);
			return 0;
		}
	}
	return ::DefWindowProcW(h_wnd, msg, w_param, l_param);
}
#	endif

} // unnamed namespace;


Environment::Environment()
	: wnd_map(), wmap_mtx()
#	if YF_Multithread == 1
	, wnd_thrd_count()
#		if YCL_Win32
	, window_class(WindowClassName, WndProc)
#		endif
#	endif
{
	YCL_Trace(Debug, "Host environment lifetime beginned.");
}
Environment::~Environment()
{
	YCL_Trace(Debug, "Host environment lifetime ended.");

#	if !YCL_Android
	using ystdex::get_value;

	std::for_each(wnd_map.cbegin() | get_value, wnd_map.cend() | get_value,
		[](Window* const& p){
			p->Close();
	});
#	endif
}

Window*
Environment::GetForegroundWindow() const ynothrow
{
#ifdef YCL_Win32
	return FindWindow(::GetForegroundWindow());
#else
	return {};
#endif
}

void
Environment::AddMappedItem(NativeWindowHandle h, Window* p)
{
	std::unique_lock<std::mutex> lck(wmap_mtx);

	wnd_map.emplace(h, p);
}

Window*
Environment::FindWindow(NativeWindowHandle h) const ynothrow
{
	std::unique_lock<std::mutex> lck(wmap_mtx);
	const auto i(wnd_map.find(h));

	return i == wnd_map.end() ? nullptr : i->second;
}

void
Environment::HostLoop()
{
	YTraceDe(Notice, "Host loop beginned.");
#	if YCL_Win32
	while(true)
	{
		::MSG msg{nullptr, 0, 0, 0, 0, {0, 0}}; //!< 本机消息。

		if(::PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE) != 0)
		{
			if(msg.message == WM_QUIT)
				break;
		//	if(!PreTranslateMessage(&msg))
			{
				::TranslateMessage(&msg);
				::DispatchMessageW(&msg);
			}
		//	if(CheckCloseDialog(frm, false))
			//	break;
		}
		else
		//	std::this_thread::yield();
		//	std::this_thread::sleep_for(host_sleep);
			// NOTE: Failure ignored.
			::WaitMessage();
	}
#	endif
	YTraceDe(Notice, "Host loop ended.");
}

#	if YF_Multithread == 1
void
Environment::LeaveWindowThread()
{
	if(--wnd_thrd_count == 0 && ExitOnAllWindowThreadCompleted)
		YSLib::PostQuitMessage(0);
}
#	endif

Point
Environment::MapCursor() const
{
#	if YCL_Win32
	if(const auto p_wnd = GetForegroundWindow())
	{
		::POINT cursor;

		::GetCursorPos(&cursor);
		::ScreenToClient(p_wnd->GetNativeHandle(), &cursor);

		const auto& pr(p_wnd->GetInputBounds());

		if(YB_LIKELY(pr.first.X != pr.second.X && pr.first.Y != pr.second.Y)
			&& (!p_wnd->BoundsLimited
			|| (IsInInterval<::LONG>(cursor.x, pr.first.X, pr.second.X)
			&& IsInInterval<::LONG>(cursor.y, pr.first.Y, pr.second.Y))))
			return {cursor.x - pr.first.X, cursor.y - pr.first.Y};
	}
#	elif YCL_Android
	// TODO: Support floating point coordinates.
	const auto& cursor(platform_ex::FetchCursor());
	// FIXME: For non-DS-hosted applications.
	const pair<Point, Point> pr(Point(0, MainScreenHeight),
		Point(MainScreenWidth, MainScreenHeight << 1));

	if(YB_LIKELY(pr.first.X != pr.second.X && pr.first.Y != pr.second.Y)
		&& (IsInInterval<float>(cursor.first, pr.first.X, pr.second.X)
		&& IsInInterval<float>(cursor.second, pr.first.Y, pr.second.Y)))
		// TODO: Use std::round.
		return {::round(cursor.first - pr.first.X),
			::round(cursor.second - pr.first.Y)};
#	endif
	return Drawing::Point::Invalid;
}

void
Environment::RemoveMappedItem(NativeWindowHandle h) ynothrow
{
	std::unique_lock<std::mutex> lck(wmap_mtx);
	const auto i(wnd_map.find(h));

	if(i != wnd_map.end())
		wnd_map.erase(i);
}

void
Environment::UpdateRenderWindows()
{
	std::unique_lock<std::mutex> lck(wmap_mtx);

	for(const auto& pr : wnd_map)
		if(pr.second)
			pr.second->Refresh();
}

#endif

} // namespace YSLib;

