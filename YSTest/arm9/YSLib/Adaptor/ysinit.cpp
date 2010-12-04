﻿/*
	Copyright (C) by Franksoft 2009 - 2010.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ysinit.cpp
\ingroup Service
\brief 程序启动时的通用初始化。
\version 0.1725;
\author FrankHB<frankhb1989@gmail.com>
\par 创建时间:
	2009-10-21 23:15:08 + 08:00;
\par 修改时间:
	2010-12-03 23:14 + 08:00;
\par 字符集:
	UTF-8;
\par 模块名称:
	YSLib::Adaptor::YShellInitialization;
*/


#include "ysinit.h"
#include "yfont.h"
#include "../Core/yapp.h"

using namespace stdex;
using namespace platform;
using std::puts;

YSL_BEGIN

void
InitYSConsole()
{
	YConsoleInit(true, ColorSpace::Lime);
}

void
EpicFail()
{
	YDebugSetStatus();
	YDebugBegin();
}

namespace
{
	inline void
	fatalError()
	{
		terminate();
	}

	void
	printFailInfo(const char* t, const char* s)
	{
		EpicFail();

		const char* line =
			"--------------------------------";

		iprintf("%s%s%s\n%s\n%s", line, t, line, s, line);
	}

	void
	defFontFail()
	{
		const char* title =
			"    Fontface Caching Failure    ";
		const char* warning =
			" Please make sure the fonts are "
			" stored in correct directory.   ";

		printFailInfo(title, warning);
		fatalError();
	}

	void
	installFail(const char* str)
	{
		const char* title =
			"      Invalid Installation      ";
		const char* warning =
			" Please make sure the data is   "
			" stored in correct directory.   ";

		printFailInfo(title, warning);
		iprintf(" %s\n cannot be found!\n", str);
		fatalError();
	}
}

void
LibfatFail()
{
	const char* title =
		"         LibFAT Failure         ";
	const char* warning =
		" An error is preventing the     "
		" program from accessing         "
		" external files.                "
		"                                "
		" If you're using an emulator,   "
		" make sure it supports DLDI     "
		" and that it's activated.       "
		"                                "
		" In case you're seeing this     "
		" screen on a real DS, make sure "
		" you've applied the correct     "
		" DLDI patch (most modern        "
		" flashcards do this             "
		" automatically).                "
		"                                "
		" Note: Some cards only          "
		" autopatch .nds files stored in "
		" the root folder of the card.   ";

	printFailInfo(title, warning);
	fatalError();
}


void
InitializeSystemFontCache()
{
	puts("Loading font files...");
	CreateFontCache(theApp.pFontCache, DEF_FONT_PATH);
	if(theApp.pFontCache && DEF_FONT_DIRECTORY)
		theApp.pFontCache->LoadFontFileDirectory(DEF_FONT_DIRECTORY);
	CheckSystemFontCache();
	iprintf("%u font file(s) are loaded\nsuccessfully.\n",
		theApp.pFontCache->GetFilesN());
	puts("Setting default font face...");

	const Drawing::Typeface* const
		pf(theApp.pFontCache->GetDefaultTypefacePtr());

	if(pf && Drawing::Font::InitializeDefault())
		iprintf("\"%s\":\"%s\",\nsuccessfully.\n", pf->GetFamilyName(),
			pf->GetStyleName());
	else
	{
		puts("failed.");
		defFontFail();
	}
}

void
DestroySystemFontCache()
{
	Drawing::DestroyFontCache(theApp.pFontCache);
	Drawing::Font::ReleaseDefault();
}

void
CheckInstall()
{
	puts("Checking installation...");
	if(!direxists(Text::StringToMBCS(YApplication::CommonAppDataPath).c_str()))
		installFail("Default data directory");
	if(!(stdex::fexists(DEF_FONT_PATH)
		|| direxists(DEF_FONT_DIRECTORY)))
		installFail("Default font");
	puts("OK!");
}

void
CheckSystemFontCache()
{
	if(!(theApp.pFontCache && theApp.pFontCache->GetTypesN() > 0))
		defFontFail();
}

YSL_END
