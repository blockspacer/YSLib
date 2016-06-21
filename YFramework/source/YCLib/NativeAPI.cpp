﻿/*
	© 2012-2016 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file NativeAPI.cpp
\ingroup YCLib
\brief 通用平台应用程序接口描述。
\version r1018
\author FrankHB <frankhb1989@gmail.com>
\since build 296
\par 创建时间:
	2012-03-26 13:36:28 +0800
\par 修改时间:
	2016-06-21 11:22 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YCLib::NativeAPI
*/


#include "YCLib/YModules.h"
#include YFM_YCLib_NativeAPI // for ::stat, ::lstat;
#if YCL_Win32 || YCL_API_POSIXFileSystem
#	include YFM_YCLib_Debug // for platform::Nonnull;
#endif
#if YCL_DS
#	include YFM_DS_YCLib_DSIO // for ::DISC_INTERFACE, Disc,
//	platform_ex::FAT::Mount;
#	include <arm9/dldi.h> // for ::_io_dldi_stub;


//! \since build 602
extern "C" ::DLDI_INTERFACE _io_dldi_stub;
#elif YCL_MinGW
#	include <ctime> // for std::gmtime;
#elif YCL_Android
#	include <sys/syscall.h> // for ::syscall, __NR_utimensat;
#endif

namespace
{

}


#if YCL_Win32 || YCL_API_POSIXFileSystem
namespace platform_ex
{

YB_NONNULL(2) int
estat(struct ::stat& st, const char* path, bool follow_link) ynothrowv
{
	using platform::Nonnull;

#	if YCL_DS || YCL_Win32
	yunused(follow_link);
	return ::stat(Nonnull(path), &st);
#	else
	return (follow_link ? ::stat : ::lstat)(Nonnull(path), &st);
#	endif
}

} // namespace platform_ex;
#endif


#if YCL_DS
namespace platform_ex
{

//! \since build 602
using namespace platform::Descriptions;

FileSystem::FileSystem(size_t pages)
	: root([pages]() ynothrow -> RootKind{
		const auto init([=](const char* name, const ::DISC_INTERFACE& disc_io)
			ynothrow -> bool{
			// NOTE: %DEFAULT_SECTORS_PAGE is 8 in "common.h" in libfat source.
			//	This is also the minimal value used by the cache. So the shift
			//	is 3.
			TryRet(platform_ex::FAT::Mount(name, disc_io, 0, pages, 3))
			CatchExpr(std::exception& e, YCL_TraceRaw(Warning,
				"FATMount failure[%s]: %s", typeid(e).name(), e.what()))
			CatchExpr(..., YCL_TraceRaw(Emergent,
				"Unknown exception found @ InitializeFileSystem."))
			return {};
		});

		if(init("sd", ::__io_dsisd))
			return RootKind::SD;
		// NOTE: As %::dldiGetInternal.
		if((::_io_dldi_stub.ioInterface.features
			& (FEATURE_SLOT_GBA | FEATURE_SLOT_NDS)) != 0)
			// NOTE: As %::sysSetCardOwner(BUS_OWNER_ARM9), with one less access
			//	to the volatile lvalue.
			REG_EXMEMCNT &= ~ARM7_OWNS_CARD;
		if(init("fat", ::_io_dldi_stub.ioInterface))
			return RootKind::FAT;
		return RootKind::Null;
	}()), init_dev([this]{
		if(root != RootKind::Null)
			return root == RootKind::FAT ? "fat" : "sd";
		// TODO: More descriptive message with underlying reason?
		throw std::runtime_error("Failed initializing file system.");
	}())
{
	// NOTE: No %ARGV_MAGIC here as libnds does.
	::chdir(root == RootKind::FAT ? "fat:/" : "sd:/");
}

FileSystem::~FileSystem()
{
	if(!YB_UNLIKELY(FAT::Unmount(init_dev)))
		YCL_TraceRaw(Err, "Failed uninitializing file system.");
}

} // namespace platform_ex;
#elif YCL_MinGW
extern "C"
{

#	if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
// See FileSystem.cpp.
//! \since build 465
struct ::tm* __cdecl __MINGW_NOTHROW
_gmtime32(const ::__time32_t* p)
{
	return std::gmtime(p);
}
#	endif

} // extern "C";
#elif YCL_Android
// NOTE: For versions where syscalls available, see http://man7.org/linux/man-pages/man2/syscalls.2.html.
// NOTE: Linux kernel version since 2.6.23 should be all OK. For early kernel
//	versions Android used, see http://elinux.org/Android_Kernel_Versions.
//	Also https://en.wikipedia.org/wiki/Android_version_history.
int
linkat(int fd1, const char* path1, int fd2, const char* path2, int flag)
	ynothrow
{
	// NOTE: The 'linkat' syscall was introduced in Linux 2.6.16.
	return ::syscall(__NR_linkat, fd1, path1, fd2, path2, flag);
}

int
futimens(int fd, const ::timespec times[2]) ynothrow
{
	// NOTE: See https://android.googlesource.com/platform/bionic/+/840a114eb12773c5af39c0c97675b27aa6dee78c/libc/bionic/futimens.cpp.
	return ::utimensat(fd, {}, times, 0);
}

int
utimensat(int fd, const char* path, const ::timespec times[2], int flags)
	ynothrow
{
	// NOTE: The 'utimesat' syscall was introduced in Linux 2.6.22.
	// NOTE: See http://stackoverflow.com/questions/19374749/how-to-work-around-absence-of-futimes-in-android-ndk.
	return ::syscall(__NR_utimensat, fd, path, times, flags);
}
#endif

namespace platform
{
}

