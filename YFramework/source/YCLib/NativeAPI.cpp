﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file NativeAPI.cpp
\ingroup YCLib
\brief 通用平台应用程序接口描述。
\version r1356;
\author FrankHB<frankhb1989@gmail.com>
\since build 296 。
\par 创建时间:
	2012-03-26 13:36:28 +0800;
\par 修改时间:
	2012-04-03 13:53 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YCLib::YCommon;
*/


#include "YCLib/NativeAPI.h"
#include <ydef.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>

namespace
{
}

#ifdef YCL_DS

static int t; //object file place holder;

#elif defined(YCL_MINGW32)

extern "C"
{

DIR*
opendir(const char* name)
{
	yassume(name);
//	yassume(name[std::strlen(name) - 1] != '\\');

	const auto r(::GetFileAttributesA(name));

	if(r != INVALID_FILE_ATTRIBUTES && r & FILE_ATTRIBUTE_DIRECTORY)
	{
		const auto dir(static_cast<DIR*>(std::malloc(sizeof(DIR))));

		yassume(std::strlen(name) + 2 < sizeof(dir->Name));

		dir->WinDir = {};
		yunseq(std::sprintf(dir->Name, "%s\\*", name), dir->hNode = NULL,
			dir->POSIXDir.lpWinDir = &dir->WinDir);
		return dir;
	}
	return nullptr;
}

dirent*
readdir(DIR* dir)
{
	if(dir->hNode == 0)
	{
		// NOTE: see MSDN "FindFirstFile function";
		yconstraint(dir->Name);
		yconstraint(*dir->Name != '\0');
		yconstraint(dir->Name[std::strlen(dir->Name) - 1] != '\\');

		if((dir->hNode = ::FindFirstFileA(dir->Name, &dir->WinDir))
			== reinterpret_cast<void*>(0xFFFFFFFF))
			dir->hNode = NULL;
	}
	else if(!::FindNextFileA(dir->hNode, &dir->WinDir))
		dir->hNode = NULL;
	if(dir->hNode && dir->hNode != reinterpret_cast<void*>(0xFFFFFFFF))
	{
		yassume(dir->WinDir.cFileName);
		// NOTE: see http://pubs.opengroup.org/onlinepubs/009695399/basedefs/
//dirent.hNode.html for details;
		yassume(std::strlen(dir->WinDir.cFileName)
			< sizeof(dir->POSIXDir.d_name));

		std::strcpy(dir->POSIXDir.d_name, dir->WinDir.cFileName);
		dir->POSIXDir.d_reclen = std::strlen(dir->POSIXDir.d_name);
		yunseq(dir->POSIXDir.d_off = 0, dir->POSIXDir.d_ino = 0);
	}
	return dir->hNode == NULL ? 0 : &dir->POSIXDir;
}

void
rewinddir(DIR* dir)
{
	if(dir->hNode)
	{
		::FindClose(dir->hNode);
		dir->hNode = NULL;
	}
}

int
closedir(DIR* dir)
{
	if(dir)
	{
		if(dir->hNode)
			::FindClose(dir->hNode);
		std::free(dir);
		return 0;
	}
	return -1;
}

} // extern "C";

#endif

namespace platform
{
}

