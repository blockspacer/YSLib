﻿/*
	© 2009-2015 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file CharacterProcessing.h
\ingroup CHRLib
\brief 字符编码处理。
\version r1121
\author FrankHB <frankhb1989@gmail.com>
\since 早于 build 132
\par 创建时间:
	2009-11-17 17:52:35 +0800
\par 修改时间:
	2015-01-11 04:39 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	CHRLib::CharacterProcessing
*/


#ifndef INC_CHRLib_CharacterProcessing_h_
#define INC_CHRLib_CharacterProcessing_h_ 1

#include "YModules.h"
#include YFM_CHRLib_CharacterMapping
#include <cstdio> // for std::FILE;
#include <memory> // for std::move;
#include <ystdex/string.hpp> // for ystdex::string_traits;
#include <algorithm> // for std::copy_n;
#include <ystdex/cstring.h> // for ystdex::ntctslen;

namespace CHRLib
{

/*!
\brief 判断整数类型字符在 ASCII 字符取值范围内。
\note 截取低 7 位。
*/
template<typename _tChar>
yconstfn bool
IsASCII(_tChar c)
{
	return !(c & ~0x7F);
}

/*!
\brief 任意整数类型字符转换为 ASCII 取值范围兼容的字符。
\note 截取低 7 位。
*/
template<typename _tChar>
yconstfn char
ToASCII(_tChar c)
{
	static_assert(std::is_integral<_tChar>::value, "Invalid type found.");

	return c & 0x7F;
}


/*!
\brief 按指定编码和转换状态转换字符串中字符为 UCS-2 字符，返回转换的字节数。
\since build 291
*/
//@{
YF_API ConversionResult
MBCToUC(ucs2_t&, const char*&, Encoding, ConversionState&& = {});
inline PDefH(ConversionResult, MBCToUC, ucs2_t& uc, const char*& c,
	Encoding enc, ConversionState& st)
	ImplRet(MBCToUC(uc, c, enc, std::move(st)))
//@}
/*!
\brief 按指定编码和转换状态转换字符流中字符为 UCS-2 字符，返回转换的字节数。
\pre 断言：指针参数非空。
\since build 291
*/
//@{
YF_API ConversionResult
MBCToUC(ucs2_t&, std::FILE*, Encoding, ConversionState&& = {});
inline PDefH(ConversionResult, MBCToUC, ucs2_t& uc, std::FILE* fp, Encoding enc,
	ConversionState& st)
	ImplRet(MBCToUC(uc, fp, enc, std::move(st)))
//@}
/*!
\brief 按指定编码和转换状态返回转换字符为 UCS-2 字符的字节数。
\since build 291
*/
//@{
YF_API ConversionResult
MBCToUC(const char*&, Encoding, ConversionState&& = {});
inline PDefH(ConversionResult, MBCToUC, const char*& c, Encoding enc,
	ConversionState& st)
	ImplRet(MBCToUC(c, enc, std::move(st)))
//! \pre 断言：指针参数非空。
//@{
YF_API ConversionResult
MBCToUC(std::FILE*, Encoding, ConversionState&& = {});
inline PDefH(ConversionResult, MBCToUC, std::FILE* fp, Encoding enc,
	ConversionState& st)
	ImplRet(MBCToUC(fp, enc, std::move(st)))
//@}
//@}

/*!
\brief 按指定编码转换 UCS-2 字符为字符串表示的多字节字符，返回转换的字节数。
\pre 断言：指针参数非空 。
\since build 305
*/
YF_API size_t
UCToMBC(char*, const ucs2_t&, Encoding);


//! \note 编码字节序同实现的 ucs2_t 存储字节序。
//@{
/*
\pre 断言：指针参数非空 。
\return 转换的串长。
*/
//@{
/*!
\brief 按指定编码转换 MBCS 字符串为 UCS-2 字符串。
\since build 291
*/
YF_API size_t
MBCSToUCS2(ucs2_t*, const char*, Encoding = CS_Default);

/*!
\brief 按指定编码转换 UCS-2 字符串为 MBCS 字符串。
\since build 291
*/
YF_API size_t
UCS2ToMBCS(char*, const ucs2_t*, Encoding = CS_Default);

//! \brief 转换 UCS-4 字符串为 UCS-2 字符串。
YF_API size_t
UCS4ToUCS2(ucs2_t*, const ucs4_t*);
//@}

/*!
\brief 取 UCS-2 字符串转换的指定编码的多字节字符串。
\since build 305
*/
template<class _tDst, class _tSrc>
_tDst
GetMBCSOf(const _tSrc& src, Encoding enc = CS_Default)
{
	_tDst str(src.length() * FetchMaxCharWidth(enc),
		typename ystdex::string_traits<_tDst>::value_type());

	str.resize(UCS2ToMBCS(&str[0], src.c_str(), enc));
	return str;
}


/*!
\pre 输入字符串的每个字符不超过 <tt>sizeof(ucsint_t)</tt> 字节。
\pre 间接断言：指针参数非空。
\since build 544
*/
//@{
//! \brief 转换指定编码的多字节字符串为指定类型的 UCS-2 字符串。
template<class _tDst = std::basic_string<ucs2_t>>
_tDst
MakeUCS2LE(const char* s, Encoding enc = CS_Default)
{
	_tDst str(ystdex::ntctslen(s),
		typename ystdex::string_traits<_tDst>::value_type());

	str.resize(MBCSToUCS2(&str[0], s, enc));
	return str;
}
//! \brief 构造指定类型的 UCS-2 字符串。
template<class _tDst = std::basic_string<ucs2_t>>
inline _tDst
MakeUCS2LE(const ucs2_t* s, Encoding = CharSet::ISO_10646_UCS_2)
{
	yconstraint(s);

	// FIXME: Correct conversion for encoding other than UCS2-LE.
	return s;
}
//! \brief 转换 UCS-4 字符串为指定类型的 UCS-2 字符串。
template<class _tDst = std::basic_string<ucs2_t>>
_tDst
MakeUCS2LE(const ucs4_t* s, Encoding = CharSet::ISO_10646_UCS_4)
{
	yconstraint(s);

	_tDst str(ystdex::ntctslen(s),
		typename ystdex::string_traits<_tDst>::value_type());

	str.resize(UCS4ToUCS2(&str[0], s));
	return str;
}
//! \note 转换指定类型的 UCS2-LE 字符串：仅当源类型参数不可直接构造目标类型时有效。
template<class _tString, class _tDst = std::basic_string<ucs2_t>,
	yimpl(typename = ystdex::enable_for_string_class_t<_tString>, typename
	= ystdex::enable_if_t<!std::is_constructible<_tDst, _tString>::value>)>
inline _tDst
MakeUCS2LE(const _tString& str, Encoding enc = CS_Default)
{
	return CHRLib::MakeUCS2LE<_tDst>(str.c_str(), str.length(), enc);
}
//! \note 传递指定类型的 UCS2-LE 字符串：仅当源类型参数可直接构造目标类型时有效。
template<class _tString, class _tDst = std::basic_string<ucs2_t>,
	yimpl(typename = ystdex::enable_for_string_class_t<_tString>, typename
	= ystdex::enable_if_t<std::is_constructible<_tDst, _tString>::value>)>
inline _tDst
MakeUCS2LE(_tString&& str)
{
	return std::forward<_tDst>(str);
}

//! \brief 构造多字节字符串。
template<class _tDst = std::string>
inline _tDst
MakeMBCS(const char* s)
{
	yconstraint(s);

	return _tDst(s);
}
//! \brief 转换 UTF-8 字符串为指定编码的多字节字符串。
template<class _tDst = std::string>
inline _tDst
MakeMBCS(const char* s, Encoding enc)
{
	return enc = CS_Default ? MakeMBCS<_tDst>(s)
		: MakeMBCS<_tDst>(MakeUCS2LE(s, CS_Default), enc);
}
//! \brief 转换 UCS-2LE 字符串为指定编码的多字节字符串。
template<class _tDst = std::string>
_tDst
MakeMBCS(const ucs2_t* s, Encoding enc = CS_Default)
{
	yconstraint(s);

	const auto w(FetchMaxVariantCharWidth(enc));
	_tDst str(ystdex::ntctslen(s) * (w == 0 ? sizeof(ucsint_t) : w) + 1,
		typename ystdex::string_traits<_tDst>::value_type());

	str.resize(UCS2ToMBCS(&str[0], s, enc));
	return str;
}
//! \note 转换指定类型的多字节字符串：仅当源类型参数不可直接构造目标类型时有效。
template<class _tString, class _tDst = std::basic_string<ucs2_t>,
	yimpl(typename = ystdex::enable_for_string_class_t<_tString>, typename
	= ystdex::enable_if_t<!std::is_constructible<_tDst, _tString>::value>)>
inline _tDst
MakeMBCS(const _tString& str, Encoding enc = CS_Default)
{
	return CHRLib::MakeUCS2LE<_tDst>(str.c_str(), str.length(), enc);
}
//! \note 传递指定类型的多字节字符串：仅当源类型参数可直接构造目标类型时有效。
template<class _tString, class _tDst = std::basic_string<ucs2_t>,
	yimpl(typename = ystdex::enable_for_string_class_t<_tString>, typename
	= ystdex::enable_if_t<std::is_constructible<_tDst, _tString>::value>)>
inline _tDst
MakeMBCS(_tString&& str)
{
	return std::forward<_tDst>(str);
}
//@}

} // namespace CHRLib;

#endif
