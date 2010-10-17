﻿// YSLib::Core::YFileSystem by Franksoft 2010
// CodePage = UTF-8;
// CTime = 2010-03-28 00:09:28 + 08:00;
// UTime = 2010-10-16 13:58 + 08:00;
// Version = 0.1836;


#ifndef INCLUDED_YFILESYS_H_
#define INCLUDED_YFILESYS_H_

// YFileSystem ：平台无关的文件处理系统。

#include "ystring.h"
#include "yfunc.hpp"
#include "../Helper/yglobal.h"
#include "../Core/yshell.h" // for HSHL delete procedure;
#include <iterator>
//#include <vector>
//#include <list>

YSL_BEGIN

YSL_BEGIN_NAMESPACE(IO)

//文件系统常量：前缀 FS 表示文件系统 (File System) 。
extern const CPATH FS_Root;
extern const CPATH FS_Seperator;
extern const CPATH FS_Now;
extern const CPATH FS_Parent;


typedef char NativePathCharType; //本机路径字符类型，POSIX 为 char ，Windows 为 wchar_t。
typedef GSStringTemplate<NativePathCharType>::basic_string NativeStringType; //本地字符串类型。

//路径类。
class Path : public ustring
{
public:
	typedef uchar_t ValueType;
	typedef GSStringTemplate<ValueType>::basic_string StringType; //内部字符串类型。
//	typedef std::codecvt<wchar_t, char, std::mbstate_t> codecvt_type;

	static const ValueType Slash;
	static const Path Now;
	static const Path Parent;

public:
	//编码转换。
//	static std::locale imbue(const std::locale&);
//	static const codecvt_type& codecvt();

	//构造函数和析构函数。
	Path();
	Path(const ValueType*);
	Path(const NativePathCharType*);
	Path(const NativeStringType&);
	template<class _tString>
	Path(const _tString&);
	~Path();

	//追加路径。
	Path&
	operator/=(const Path&);

	//查询。
	DefPredicate(Absolute, platform::IsAbsolute(GetNativeString().c_str()))
	DefPredicate(Relative, !IsAbsolute())
	bool
	HasRootName() const;
	bool
	HasRootDirectory() const;
	bool
	HasRootPath() const;
	bool
	HasRelativePath() const;
	bool
	HasParentPath() const;
	bool
	HasFilename() const;
	bool
	HasStem() const;
	bool HasExtension() const;

	//路径分解。
	Path
	GetRootName() const;
	Path
	GetRootDirectory() const;
	Path
	GetRootPath() const;
	Path
	GetRelativePath() const;
	Path
	GetParentPath() const;
	Path
	GetFilename() const;
	Path
	GetStem() const;
	Path
	GetExtension() const;
	DefGetter(NativeStringType, NativeString, Text::StringToMBCS(*this)) //取本地格式和编码的字符串。

	//修改函数。

	Path&
	MakeAbsolute(const Path&);
	Path&
	RemoveFilename();
	Path&
	ReplaceExtension(const Path& = Path());

	//迭代器。
	class iterator : public std::iterator<std::bidirectional_iterator_tag, Path>
	{
	private:
		const value_type* ptr;
		StringType::size_type n;

		iterator();

	public:
		iterator(const value_type&);
		iterator(const iterator&);

		iterator&
		operator++();
		iterator
		operator++(int);

		iterator&
		operator--();
		iterator
		operator--(int);

		bool
		operator==(const iterator&) const;

		bool
		operator!=(const iterator&) const;

		value_type
		operator*() const;

		DefGetter(const value_type*, Ptr, ptr)
		DefGetter(StringType::size_type, Position, n)
	};

	typedef iterator const_iterator;

	iterator begin() const;

	iterator end() const;
};

inline
Path::Path()
	: ustring()
{}
inline
Path::Path(const Path::ValueType* pathstr)
	: ustring(pathstr)
{}
inline
Path::Path(const NativePathCharType* pathstr)
	: ustring(Text::MBCSToString(pathstr))
{}
inline
Path::Path(const NativeStringType& pathstr)
	: ustring(Text::MBCSToString(pathstr))
{}
template<class _tString>
inline
Path::Path(const _tString& pathstr)
	: ustring(pathstr)
{}
inline
Path::~Path()
{}

inline bool
Path::HasRootName() const
{
	return !GetRootName().empty();
}
inline bool
Path::HasRootDirectory() const
{
	return !GetRootDirectory().empty();
}
inline bool
Path::HasRootPath() const
{
	return !GetRootPath().empty();
}
inline bool
Path::HasRelativePath() const
{
	return !GetRelativePath().empty();
}
inline bool
Path::HasParentPath() const
{
	return !GetParentPath().empty();
}
inline bool
Path::HasFilename() const
{
	return !GetFilename().empty();
}
inline bool
Path::HasStem() const
{
	return !GetStem().empty();
}
inline bool
Path::HasExtension() const
{
	return !GetExtension().empty();
}

inline
Path::iterator::iterator()
{}
inline
Path::iterator::iterator(const value_type& p)
	: ptr(&p), n(StringType::npos)
{}
inline
Path::iterator::iterator(const iterator& i)
	: ptr(i.ptr), n(i.n)
{}

inline Path::iterator
Path::iterator::operator++(int)
{
	return ++iterator(*this);
}

inline Path::iterator
Path::iterator::operator--(int)
{
	return --iterator(*this);
}

inline bool
Path::iterator::operator==(const iterator& rhs) const
{
	return ptr == rhs.ptr && n == rhs.n;
}

inline bool
Path::iterator::operator!=(const iterator& rhs) const
{
	return !(*this == rhs);
}

inline Path::iterator
Path::begin() const
{
	return ++iterator(*this);
}

inline Path::iterator
Path::end() const
{
	return iterator(*this);
}


inline bool
operator==(const Path& lhs, const Path& rhs)
{
	return lhs.GetNativeString() == rhs.GetNativeString();
}
inline bool
operator!=(const Path& lhs, const Path& rhs)
{
	return !(lhs == rhs);
}
inline bool
operator<(const Path& lhs, const Path& rhs)
{
	return lhs.GetNativeString() < rhs.GetNativeString();
}
inline bool
operator<=(const Path& lhs, const Path& rhs)
{
	return !(rhs < lhs);
}
inline bool operator>(const Path& lhs, const Path& rhs)
{
	return rhs < lhs;
}
inline bool
operator>=(const Path& lhs, const Path& rhs)
{
	return !(lhs < rhs);
}

inline Path
operator/(const Path& lhs, const Path& rhs)
{
	return Path(lhs) /= rhs;
}

inline void
swap(Path& lhs, Path& rhs)
{
	lhs.swap(rhs);
}
//bool lexicographical_compare(Path::iterator, Path::iterator,
//							 Path::iterator, Path::iterator);


//截取路径末尾的文件名。
const char*
GetFileName(CPATH);
string
GetFileName(const string&);

//截取路径中的目录名并返回字符串。
string
GetDirectoryName(const string&);

//截取路径中的目录名和文件名保存至字符串，并返回最后一个目录分隔符的位置。
string::size_type
SplitPath(const string&, string&, string&);


//截取文件名开头的主文件名（贪婪匹配）。
string
GetStem(const string&, const string&);

//对于两个字符串，判断前者是否是后者的主文件名。
bool
IsStem(const char*, const char*);
bool
IsStem(const string&, const string&);

//判断给定两个文件名的主文件名是否相同（忽略大小写；贪婪匹配）。
bool
HaveSameStems(const char*, const char*);
bool
HaveSameStems(const string&, const string&);

//截取文件名末尾的扩展名（非贪婪匹配）。
const char*
GetExtendName(const char*);
string
GetExtendName(const string&);

//对于两个字符串，判断前者是否是后者的扩展名。
bool
IsExtendName(const char*, const char*);
bool
IsExtendName(const string&, const string&);

//判断给定两个文件名的扩展名是否相同（忽略大小写；非贪婪匹配）。
bool
HaveSameExtendNames(const char*, const char*);
bool
HaveSameExtendNames(const string&, const string&);


//切换路径。
inline int
ChDir(CPATH path)
{
	return platform::chdir(path);
}
int
ChDir(const string&);

//取当前工作目录。
string
GetNowDirectory();

//验证绝对路径有效性。
bool
ValidateDirectory(const string&);
inline bool
ValidateDirectory(const Path& path)
{
	return ValidateDirectory(path.GetNativeString());
}


//文件名过滤器。
typedef bool FNFILTER(const String&);
typedef FNFILTER* PFNFILTER;

struct HFileNameFilter : public GHBase<PFNFILTER>
{
	typedef GHBase<PFNFILTER> ParentType;

	HFileNameFilter(const PFNFILTER pf = NULL)
	: GHBase<PFNFILTER>(pf)
	{}

	bool
	operator()(const String& name) const
	{
		if(GetPtr())
			return GetPtr()(name);
		return -1;
	}
};


//文件列表模块。
class MFileList
{
public:
	typedef String ItemType; //项目名称类型。
	typedef vector<ItemType> ListType; //项目列表类型。

protected:
	Path Directory; //目录的完整路径。
	ListType List; //目录中的项目列表。

public:
	MFileList(CPATH = NULL); //参数为空时为根目录。
	virtual DefEmptyDtor(MFileList)

	//导航至相对路径。
	bool
	operator/=(const string&);
	bool
	operator/=(const String&);

	DefGetter(const Path&, Directory, Directory) //取目录的完整路径。
	DefGetter(const ListType&, List, List) //取项目列表。

	ListType::size_type
	LoadSubItems(); //在目录中取子项目。

	ListType::size_type
	ListItems(); //遍历目录中的项目，更新至列表。
};

inline bool
MFileList::operator/=(const String& s)
{
	return *this /= StringToMBCS(s);
}

YSL_END_NAMESPACE(IO)

YSL_END

#endif

