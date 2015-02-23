﻿/*
	© 2010-2015 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file YAdaptor.h
\ingroup Adaptor
\brief 外部库关联。
\version r1668
\author FrankHB <frankhb1989@gmail.com>
\since 早于 build 132
\par 创建时间:
	2010-02-22 20:16:21 +0800
\par 修改时间:
	2015-02-19 14:44 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YSLib::Adaptor::YAdaptor
*/


#ifndef YSL_INC_Adaptor_YAdaptor_h_
#define YSL_INC_Adaptor_YAdaptor_h_ 1

#include "YModules.h"

//引入平台设置和存储调试设施。
#include YFM_YSLib_Adaptor_YNew

// 确保包含标准库必要部分。
#include <libdefect/cmath.h>

//包含 YStandardEx 公用部分。
#include <ystdex/algorithm.hpp> // for ystdex::min, ystdex::max;
#include <ystdex/functional.hpp>
#include <ystdex/utility.hpp>
#include <ystdex/string.hpp> // for std::to_string, ystdex::to_string;

//包含 YCLib 公用部分。
#include YFM_YCLib_YCommon
#include YFM_YCLib_Debug
#include YFM_YCLib_Keys
#include YFM_YCLib_Timer
#include YFM_YCLib_FileSystem
#include YFM_YCLib_Video
#include YFM_YCLib_Mutex

/*
!\brief YSLib 命名空间。
\since 早于 build 132
*/
namespace YSLib
{

/*!
\brief 调用分派。
\since build 303
*/
//@{
using ystdex::seq_apply;
using ystdex::unseq_apply;
//@}

/*!
\brief 实用类型。
\since build 209
*/
//@{
using ystdex::noncopyable;
//! \since build 373
using ystdex::nonmovable;
using ystdex::nullptr_t;
//@}

/*!
\brief 数学库函数。
\since build 301
*/
//@{
using std::round;
//@}

/*!
\brief 算法。
\since build 265
*/
//@{
#if __cplusplus >= 201402L
using std::min;
using std::max;
#else
//! \since build 578
using ystdex::min;
//! \since build 578
using ystdex::max;
#endif
//}

//! \brief 助手功能。
//@{
//! \since build 291
using ystdex::arrlen;
//! \since build 308
using std::to_string;
//! \since build 308
using ystdex::to_string;
//@}


/*!
\brief 平台通用数据类型。
\since build 209
*/
//@{
//! \since build 417
using ystdex::byte;
//! \since build 417
using ystdex::octet;
using ystdex::errno_t;
using ystdex::ptrdiff_t;
using ystdex::size_t;
//! \since build 245
using ystdex::wint_t;
//@}


/*!
\brief 并发操作。
\since build 551
*/
using namespace platform::Concurrency;
/*!
\brief 平台公用描述。
\since build 456
*/
using namespace platform::Descriptions;

/*!
\brief 基本实用例程。
\since build 177
*/
//@{
//! \since build 547
using platform::usystem;
//@}

/*!
\brief 日志。
\since build 510
*/
//@{
using platform::Logger;
using platform::FetchCommonLogger;
//@}

/*!
\brief 断言操作。
\pre 允许对操作数使用 ADL 查找同名声明，但要求最终的表达式语义和调用这里的声明等价。
\since build 553
*/
//@{
using platform::Nonnull;
//! \since build 566
using platform::CheckIter;
using platform::Deref;
//@}

/*!
\brief 文件系统例程。
\since build 299
*/
//@{
//! \since build 549
using platform::uaccess;
//! \since build 549
using platform::uopen;
using platform::ufopen;
using platform::ufexists;
//! \since build 566
using platform::upclose;
//! \since build 566
using platform::upopen;
//! \since build 304
using platform::u16getcwd_n;
//! \since build 313
using platform::uchdir;
//! \since build 475
using platform::umkdir;
//! \since build 475
using platform::urmdir;
//! \since build 476
using platform::uunlink;
//! \since build 476
using platform::uremove;
//! \since build 341
using platform::truncate;
//! \since build 547
using platform::GetFileModificationTimeOf;
//! \since build 547
using platform::GetFileSizeOf;
//@}

//系统处理函数。
using platform::terminate;

//基本图形定义。
using platform::SPos;
using platform::SDst;

//基本输出接口。
using platform::InitVideo;

//计时器和时钟。
using platform::GetTicks;
using platform::GetHighResolutionTicks;

//输入类型。
//! \since build 490
using platform::KeyIndex;
//! \since build 486
using platform::KeyBitsetWidth;
using platform::KeyInput;
//! \since build 489
namespace KeyCategory = platform::KeyCategory;
namespace KeyCodes = platform::KeyCodes;

//! \since build 486
using platform::FindFirstKey;
//! \since build 487
using platform::FindNextKey;
//! \since build 487
using platform::MapKeyChar;

//! \brief 图形处理。
namespace Drawing
{

//! \since build 441
using platform::BGRA;
//! \since build 441
using platform::RGBA;
//! \since build 559
using platform::Pixel;
/*!
\since build 297
*/
//@{
using platform::FetchAlpha;
using platform::FetchOpaque;
//@}
//! \since build 417
//@{
using platform::MonoType;
using platform::AlphaType;
//@}
using platform::Color;
namespace ColorSpace = platform::ColorSpace;

} // namespace Drawing;

namespace IO
{
//文件系统抽象。
/*!
\brief 本机路径字符类型。
\since build 286
*/
using platform::NativePathCharType;
//! \since build 402
using platform::CS_Path;

//! \since build 474
using platform::NodeCategory;

//! \since build 411
//@{
using platform::FileOperationFailure;
using platform::DirectorySession;
using platform::HDirectory;
using platform::FileIterator;
//@}
using platform::IsAbsolute;
using platform::GetRootNameLength;

} // namespace IO;

} // namespace YSLib;

#endif

