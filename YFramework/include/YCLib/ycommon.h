﻿/*
	Copyright (C) by Franksoft 2009 - 2012.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\defgroup YCLib YCommonLib
\brief YSLib 基础库。
*/

/*!	\file ycommon.h
\ingroup YCLib
\brief 平台相关的公共组件无关函数与宏定义集合。
\version r2874;
\author FrankHB<frankhb1989@gmail.com>
\since 早于 build 132 。
\par 创建时间:
	2009-11-12 22:14:28 +0800;
\par 修改时间:
	2012-03-08 15:57 +0800;
\par 文本编码:
	UTF-8;
\par 模块名称:
	YCLib::YCommon;
*/


#ifndef YCL_INC_YCOMMON_H_
#define YCL_INC_YCOMMON_H_

//平台定义。
#include "Platform.h"

//平台无关部分。
#include <ydef.h>
#include <ystdex/cstdio.h>
#include <cstdlib>

//平台相关部分。
#include "NativeAPI.h"

//#define HEAP_SIZE (mallinfo().uordblks)

//! \brief 默认平台命名空间。
namespace platform
{
	//外部平台库名称引用。
	using ::swiWaitForVBlank;

	using ::lcdMainOnTop;
	using ::lcdMainOnBottom;
	using ::lcdSwap;
	using ::videoSetMode;
	using ::videoSetModeSub;

	using ::touchRead;

	//平台相关的全局常量。

	#define YCL_MAX_FILENAME_LENGTH MAXPATHLEN //!< 最大路径长度。
	#define YCL_MAX_PATH_LENGTH YCL_MAX_FILENAME_LENGTH

	const char DEF_PATH_DELIMITER = '/'; //!< 文件路径分隔符。
	const char* const DEF_PATH_SEPERATOR = "/"; //!< 文件路径分隔字符串。
	#define DEF_PATH_ROOT DEF_PATH_SEPERATOR

	//类型定义。
	/*!
	\brief 本机路径字符类型。
	
	POSIX 为 char ，Windows 为 wchar_t。
	\since build 286 。
	*/
	typedef char NativePathCharType;
	/*!
	\brief 本机路径字符串类型。
	\since build 286 。
	*/
	typedef NativePathCharType PATHSTR[YCL_MAX_PATH_LENGTH];
	/*!
	\brief 本机文件名类型。
	\since build 286 。
	*/
	typedef NativePathCharType FILENAMESTR[YCL_MAX_FILENAME_LENGTH];

	// using ystdex;
	using ystdex::const_path_t;
	using ystdex::path_t;

	/*!
	\brief 主内存块设置。

	满足条件时使用平台特定算法覆盖内存区域的每个字节，否则使用 std::memset 。
	\note 参数和返回值语义同 std::memset 。
	\warning 仅进行内存区域检查，不进行空指针或其它检查。
	*/
	void*
	mmbset(void*, int, std::size_t);

	/*!
	\brief 主内存块复制。

	满足条件时使用平台特定算法复制内存区域，否则使用 std::memcpy 。
	\note 参数和返回值语义同 std::memcpy 。
	\warning 仅进行内存区域检查，不进行空指针或其它检查。
	*/
	void*
	mmbcpy(void*, const void*, std::size_t);

	/*!
	\brief 当 buf 非空时取当前工作目录复制至 buf 起始的长为 t 的缓冲区中。
	\return buf 。
	*/
	char*
	getcwd_n(char* buf, std::size_t);

	/*!
	\brief 判断指定目录是否存在。
	*/
	bool
	direxists(const_path_t);

	using ::mkdir;
	using ::chdir;

	/*!
	\brief 按路径新建一个或多个目录。
	*/
	bool
	mkdirs(const_path_t);


	/*!
	\brief 异常终止函数。
	*/
	void
	terminate();

	typedef s16 SPos; //!< 屏幕坐标度量。
	typedef u16 SDst; //!< 屏幕坐标距离。
	typedef u16 PixelType; //!< 像素。
	typedef PixelType* BitmapPtr;
	typedef const PixelType* ConstBitmapPtr;
	typedef PixelType ScreenBufferType[SCREEN_WIDTH * SCREEN_HEIGHT]; \
		//!< 主显示屏缓冲区。
	#define BITALPHA BIT(15) //!<  Alpha 位。


	//! \brief 系统默认颜色空间。
	namespace ColorSpace
	{
	//	#define DefColorA(r, g, b, name) name = ARGB16(1, r, g, b),
		#define	HexAdd0x(hex) 0x##hex
		#define DefColorH_(hex, name) name = \
			RGB8(((hex >> 16) & 0xFF), ((hex >> 8) & 0xFF), (hex & 0xFF)) \
			| BITALPHA
		#define DefColorH(hex_, name) DefColorH_(HexAdd0x(hex_), name)

		//参考：http://www.w3schools.com/html/html_colornames.asp 。

		typedef enum
		{
			DefColorH(00FFFF, Aqua),
			DefColorH(000000, Black),
			DefColorH(0000FF, Blue),
			DefColorH(FF00FF, Fuchsia),
			DefColorH(808080, Gray),
			DefColorH(008000, Green),
			DefColorH(00FF00, Lime),
			DefColorH(800000, Maroon),
			DefColorH(000080, Navy),
			DefColorH(808000, Olive),
			DefColorH(800080, Purple),
			DefColorH(FF0000, Red),
			DefColorH(C0C0C0, Silver),
			DefColorH(008080, Teal),
			DefColorH(FFFFFF, White),
			DefColorH(FFFF00, Yellow)
		} ColorSet;

		#undef DefColorH
		#undef DefColorH_
		#undef HexAdd0x
	}

	//! \brief 颜色。
	class Color
	{
	public:
		typedef ColorSpace::ColorSet ColorSet;
		typedef u8 MonoType;
		typedef u8 AlphaType;

	private:
		/*!
		\brief RGB 分量。
		\since build 276 。
		*/
		MonoType r, g, b;
		/*!
		\brief Alpha 分量。
		\since build 276 。
		*/
		AlphaType a;

	public:
		/*!
		\brief 构造：使用本机颜色对象。
		*/
		yconstfn
		Color(PixelType = 0);
		/*!
		\brief 使用 RGB 值和 alpha 位构造 Color 对象。
		*/
		yconstfn
		Color(MonoType, MonoType, MonoType, AlphaType = true);

		/*!
		\brief 转换：本机颜色对象。
		*/
		yconstfn
		operator PixelType() const;

		/*!
		\brief 取红色分量。
		*/
		yconstfn MonoType
		GetR() const;
		/*!
		\brief 取绿色分量。
		*/
		yconstfn MonoType
		GetG() const;
		/*!
		\brief 取蓝色分量。
		*/
		yconstfn MonoType
		GetB() const;
		/*!
		\brief 取 alpha 分量。
		*/
		yconstfn AlphaType
		GetA() const;
	};

	yconstfn
	Color::Color(PixelType px)
		: r(px << 3 & 248), g(px >> 2 & 248), b(px >> 7 & 248),
		a(px & BITALPHA ? 0xFF : 0x00)
	{}
	yconstfn
	Color::Color(MonoType r_, MonoType g_, MonoType b_, AlphaType a_)
		: r(r_), g(g_), b(b_), a(a_)
	{}

	yconstfn
	Color::operator PixelType() const
	{
		return ARGB16(int(a != 0), r >> 3, g >> 3, b >> 3);
	}

	yconstfn Color::MonoType
	Color::GetR() const
	{
		return r;
	}
	yconstfn Color::MonoType
	Color::GetG() const
	{
		return g;
	}
	yconstfn Color::MonoType
	Color::GetB() const
	{
		return b;
	}
	yconstfn Color::AlphaType
	Color::GetA() const
	{
		return a;
	}

	//! \brief 本机按键空间。
	namespace KeySpace
	{
		//按键集合。
		typedef enum
		{
			Empty	= 0,
			A		= KEY_A,
			B		= KEY_B,
			Select	= KEY_SELECT,
			Start	= KEY_START,
			Right	= KEY_RIGHT,
			Left	= KEY_LEFT,
			Up		= KEY_UP,
			Down	= KEY_DOWN,
			R		= KEY_R,
			L		= KEY_L,
			X		= KEY_X,
			Y		= KEY_Y,
			Touch	= KEY_TOUCH,
			Lid		= KEY_LID
		} KeySet;

		//按键别名。
		const KeySet
			Enter = A,
			Esc = B,
			PgUp = L,
			PgDn = R;
	}


	//! \brief 本机按键代码。
	class KeyCode
	{
	public:
		typedef u32 InputType; //!< 本机按键输入类型。

	private:
		InputType _value;

	public:
		/*!
		\brief 构造：使用本机按键输入对象。
		*/
		yconstfn
		KeyCode(InputType = 0);

		/*!
		\brief 转换：本机按键输入对象。
		*/
		yconstfn
		operator InputType() const;
	};

	yconstfn
	KeyCode::KeyCode(InputType i)
	: _value(i)
	{}

	yconstfn
	KeyCode::operator KeyCode::InputType() const
	{
		return _value;
	}


	//! \brief 按键信息。
	typedef struct KeysInfo
	{
		KeyCode Up, Down, Held;
	} KeysInfo;


	//! \brief 屏幕光标信息。
	typedef struct CursorInfo : public ::touchPosition
	{
		/*!
		\brief 取横坐标。
		*/
		SDst
		GetX() const;
		/*!
		\brief 取纵坐标。
		*/
		SDst
		GetY() const;
	} CursorInfo;

	inline SDst
	CursorInfo::GetX() const
	{
		return px;
	}
	inline SDst
	CursorInfo::GetY() const
	{
		return py;
	}


	/*!
	\brief 调试模式：设置状态。
	\note 当且仅当状态为 true 时，
		以下除 YDebugGetStatus 外的调试模式函数有效。
	*/
	void
	YDebugSetStatus(bool = true);

	/*!
	\brief 调试模式：取得状态。
	*/
	bool
	YDebugGetStatus();

	/*!
	\brief 调试模式：显示控制台（fc 为前景色，bc 为背景色）。
	*/
	void
	YDebugBegin(Color fc = ColorSpace::White, Color bc = ColorSpace::Blue);

	/*!
	\brief 调试模式：按键继续。
	*/
	void
	YDebug();
	/*!
	\brief 调试模式：显示控制台字符串，按键继续。
	*/
	void
	YDebug(const char*);

	/*!
	\brief 调试模式 printf ：显示控制台格式化输出 ，按键继续。
	*/
	int
	yprintf(const char*, ...)
		_ATTRIBUTE ((format (printf, 1, 2)));

	//断言。
	#ifdef YCL_USE_YASSERT

	#undef YAssert

	/*!
	\brief YCLib 默认断言函数。
	\note 当定义 YCL_USE_YASSERT 宏时，宏 YAssert 操作由此函数实现。
	*/
	void
	yassert(bool, const char*, const char*, int, const char*);

	#define YAssert(exp, message) \
		platform::yassert(exp, #exp, message, __LINE__, __FILE__)

	#else

	#	include <cassert>
	#	define YAssert(exp, message) assert(exp)

	#endif


	//定长路径字符串类型。
	typedef char PATHSTR[MAXPATHLEN];


	//目录迭代器。
	class HDirectory
	{
	public:
		typedef ::DIR* IteratorType; //!< 本机迭代器类型。

		static PATHSTR Name; //!< 节点名称。
		static struct ::stat Stat; //!< 节点状态信息。
		static int LastError; //!< 上一次操作结果，0 为无错误。

	private:
		IteratorType dir;

	public:
		/*!
		\brief 构造：使用路径字符串。
		*/
		explicit
		HDirectory(const_path_t = nullptr);

	private:
		/*!
		\brief 构造：使用本机迭代器。
		*/
		yconstfn
		HDirectory(IteratorType&);

	public:
		/*!
		\brief 复制构造。
		\note 浅复制。
		*/
		yconstfn
		HDirectory(const HDirectory&);
		/*!
		\brief 析构。
		*/
		~HDirectory();

		/*!
		\brief 迭代：向后遍历。
		*/
		HDirectory&
		operator++();
		/*!
		\brief 迭代：向前遍历。
		*/
		HDirectory
		operator++(int);

		/*!
		\brief 判断文件系统节点有效性。
		*/
		bool
		IsValid() const;
		/*!

		\brief 从节点状态信息判断是否为目录。
		*/
		static bool
		IsDirectory();

		/*!
		\brief 打开。
		*/
		void
		Open(const_path_t);

		/*!
		\brief 关闭。
		*/
		void
		Close();

		/*!
		\brief 复位。
		*/
		void
		Reset();
	};

	inline
	HDirectory::HDirectory(const_path_t path)
		: dir()
	{
		Open(path);
	}
	yconstfn
	HDirectory::HDirectory(const HDirectory& h)
		: dir(h.dir)
	{}
	yconstfn
	HDirectory::HDirectory(IteratorType& d)
		: dir(d)
	{}
	inline
	HDirectory::~HDirectory()
	{
		Close();
	}

	inline HDirectory
	HDirectory::operator++(int)
	{
		return ++HDirectory(*this);
	}

	inline bool
	HDirectory::IsValid() const
	{
		return dir;
	}


	/*!
	\brief 判断指定路径字符串是否表示一个绝对路径。
	*/
	bool
	IsAbsolute(const_path_t);

	/*!
	\brief 取指定路径的文件系统根节点名称的长度。
	*/
	std::size_t
	GetRootNameLength(const_path_t);


	/*!
	\brief 设置允许设备进入睡眠的标识状态。
	\return 旧状态。
	\note 默认状态为 true 。
	\since build 278 。
	*/
	bool
	AllowSleep(bool);

	/*!
	\brief 快速刷新缓存映像到显示屏缓冲区。
	\note 第一参数为显示屏缓冲区，第二参数为源缓冲区。
	*/
	void
	ScreenSynchronize(PixelType*, const PixelType*);

	/*!
	\brief 复位屏幕显示模式。
	*/
	void
	ResetVideo();

	/*!
	\brief 启动控制台。
	\note fc 为前景色，bc为背景色。
	*/
	void
	YConsoleInit(u8 dspIndex,
		Color fc = ColorSpace::White, Color bc = ColorSpace::Black);


	/*!
	\brief 等待任意按键。
	*/
	void
	WaitForInput();

	/*!
	\brief 等待 mask 包含的按键。
	*/
	void
	WaitForKey(u32 mask);

	/*!
	\brief 等待任意按键（除触摸屏、翻盖外）。
	*/
	inline void
	WaitForKeypad()
	{
		WaitForKey(KEY_A | KEY_B | KEY_X | KEY_Y | KEY_L | KEY_R
			| KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN
			| KEY_START | KEY_SELECT);
	}

	/*!
	\brief 等待任意按键（除 L 、 R 和翻盖外）。
	*/
	inline void
	WaitForFrontKey()
	{
		WaitForKey(KEY_TOUCH | KEY_A | KEY_B | KEY_X | KEY_Y
			| KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN
			| KEY_START | KEY_SELECT);
	}

	/*!
	\brief 等待任意按键（除 L 、 R 、触摸屏和翻盖外）。
	*/
	inline void
	WaitForFrontKeypad()
	{
		WaitForKey(KEY_A | KEY_B | KEY_X | KEY_Y
			| KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN
			|KEY_START | KEY_SELECT);
	}

	/*!
	\brief 等待方向键。
	*/
	inline void
	WaitForArrowKey()
	{
		WaitForKey(KEY_LEFT | KEY_RIGHT | KEY_UP | KEY_DOWN);
	}

	/*!
	\brief 等待按键 A 、 B 、 X 、 Y 键。
	*/
	inline void
	WaitForABXY()
	{
		WaitForKey(KEY_A | KEY_B | KEY_X | KEY_Y);
	}

	/*
	void trimString(char* string);
	void unescapeString(char* string);
	u32 versionStringToInt(char* string);
	void versionIntToString(char* out, u32 version);
	*/

	/*!
	\brief 写入当前按键信息。
	\since build 272 。
	*/
	void
	WriteKeys(KeysInfo&);

	/*!
	\brief 写入当前指针设备信息。
	\since build 272 。
	*/
	void
	WriteCursor(CursorInfo&);


	/*!
	\brief 开始 tick 计时。
	*/
	void
	StartTicks();

	/*!
	\brief 取 tick 数。
	\note 单位为毫秒。
	*/
	u32
	GetTicks();

	/*!
	\brief 取高精度 tick 数。
	\note 单位为纳秒。
	\since build 291 。
	*/
	u64
	GetHighResolutionTicks();

	/*!
	\brief 初始化视频输出。
	*/
	bool
	InitVideo();
}

namespace platform_ex
{
	/*!
	\brief 默认上屏初始化函数。
	*/
	platform::BitmapPtr
	InitScrUp(int&);

	/*!
	\brief 默认下屏初始化函数。
	*/
	platform::BitmapPtr
	InitScrDown(int&);
}

#endif
