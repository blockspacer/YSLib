﻿/*
	© 2013-2016, 2018-2019 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Environment.cpp
\ingroup Helper
\brief 环境。
\version r1908
\author FrankHB <frankhb1989@gmail.com>
\since build 379
\par 创建时间:
	2013-02-08 01:27:29 +0800
\par 修改时间:
	2019-11-29 03:35 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	Helper::Environment
*/


#include "Helper/YModules.h"
#include YFM_Helper_Environment
#include YFM_Helper_Initialization // for InitializeEnvironment,
//	ShowInitializedLog, InitializeKeyModule, LoadComponents;
#if YCL_DS
#	include YFM_YCLib_NativeAPI // for ::powerOn, ::defaultExceptionHandler,
//	platform_ex::InitializeFileSystem;
#	include YFM_DS_YCLib_DSVideo // for platform_ex::DSConsoleInit;
#	include YFM_YSLib_Core_YGDIBase // for Drawing::ColorSpace;
#elif YCL_Win32
#	include YFM_Win32_YCLib_Consoles // for platform_ex::FixConsoleHandler;
#	include YFM_Win32_YCLib_MinGW32 // for platform_ex::Win32Exception;
#	include YFM_YSLib_Core_YCoreUtilities // for FetchEnvironmentVariable;
#	include YFM_YCLib_Debug // for platform_ex::SendDebugString;
#endif

namespace YSLib
{

using namespace platform_ex;
#if YF_Hosted
using namespace Host;
#endif

Environment::Environment(Application& app)
{
#if !YF_Hosted
	// NOTE: This only effects freestanding implementations now, which may need
	//	different behavior than default implementation. Hosted implemenations
	//	are expected to have termination handlers friendly to debug, which can
	//	be overriden before the construction of %Environment if needed.
	std::set_terminate(terminate);
#endif
#if YCL_DS

	using namespace platform;
	namespace ColorSpace = Drawing::ColorSpace;

	::powerOn(POWER_ALL);
	::defaultExceptionHandler();
	DSConsoleInit(true, ColorSpace::Lime, ColorSpace::Black);
	FetchCommonLogger().SetSender([&](Logger::Level lv, Logger&,
		const char* str) YB_ATTR_LAMBDA_QUAL(ynothrowv, YB_NONNULL(4)){
		if(ShowInitializedLog || lv <= Descriptions::Alert)
		{
			if(!ShowInitializedLog)
			{
				static const struct Init
				{
					Init()
					{
						DSConsoleInit({}, ColorSpace::White, ColorSpace::Blue);
					}
				} init;
			}
			std::fprintf(stderr, "%s\n", Nonnull(str));
			std::fflush(stderr);
		}
	});
	InitializeKeyModule([&]{
		app.AddExit(ystdex::any(ystdex::in_place_type<FileSystem>));
	}, yfsig, "         LibFAT Failure         ",
		" An error is preventing the\n"
		" program from accessing\n"
		" external files.\n"
		"\n"
		" If you're using an emulator,\n"
		" make sure it supports DLDI\n"
		" and that it's activated.\n"
		"\n"
		" In case you're seeing this\n"
		" screen on a real DS, make sure\n"
		" you've applied the correct\n"
		" DLDI patch (most modern\n"
		" flashcards do this\n"
		" automatically).\n"
		"\n"
		" Note: Some cards only\n"
		" autopatch .nds files stored in\n"
		" the root folder of the card.\n");
#elif YCL_Win32
	TryExpr(FixConsoleHandler())
	CatchExpr(Win32Exception&,
		YTraceDe(Warning, "Console handler setup failed."))

	string env_str;

	// TODO: Extract as %YCoreUtilities functions?
	if(FetchEnvironmentVariable(env_str, "YF_DEBUG_OUTPUT"))
		FilterExceptions([&]{
			if(env_str == "1")
				FetchCommonLogger().SetSender(SendDebugString);
		});
#endif
#if false
	// TODO: Review locale APIs compatibility.
	static yconstexpr const char locale_str[]{"zh_CN.GBK"};

	if(!std::setlocale(LC_ALL, locale_str))
		throw GeneralEvent("Call of std::setlocale() with %s failed.\n",
			locale_str);
#endif
	// NOTE: Ensure root node is initialized before lifetime of environment
	//	begins.
	string res;

	YTraceDe(Notice, "Checking installation...");
	InitializeKeyModule([&]{
		Root = LoadConfiguration(true);
		if(Root.GetName() == "YFramework")
			Root = PackNodes(string(), std::move(Root));
		LoadComponents(app, AccessNode(Root, "YFramework"));
		YTraceDe(Notice, "Check of installation succeeded.");
	}, yfsig, "      Invalid Installation      ",
		" Please make sure the data is\n stored in correct directory.\n");
	YF_Trace(Debug, "Environment lifetime began.");
}
Environment::~Environment()
{
	YF_Trace(Debug, "Environment lifetime ended.");
}

} // namespace YSLib;

