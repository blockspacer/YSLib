﻿/*
	© 2009-2016 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file Initialization.cpp
\ingroup Helper
\brief 框架初始化。
\version r3155
\author FrankHB <frankhb1989@gmail.com>
\since 早于 build 132
\par 创建时间:
	2009-10-21 23:15:08 +0800
\par 修改时间:
	2016-08-21 22:41 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	Helper::Initialization
*/


#include "Helper/YModules.h"
#include YFM_Helper_Initialization // for ystdex::replace_cast;
#include YFM_YCLib_MemoryMapping // for MappedFile;
#include YFM_YSLib_Core_YException // for ExtractException, ExtractAndTrace;
#include YFM_CHRLib_MappingEx // for CHRLib::cp113_lkp;
#include YFM_YSLib_Service_TextFile // for Text::BOM_UTF_8, Text::CheckBOM;
#include YFM_NPL_Configuration // for NPL::Configuration;
#include YFM_Helper_Environment // for Environment::AddExitGuard;
#include <ystdex/string.hpp> // for ystdex::write_literal, ystdex::sfmt;
#include <ystdex/scope_guard.hpp> // for ystdex::swap_guard;
#include <cerrno> // for errno;
#include YFM_YSLib_Service_FileSystem // for IO::TraverseChildren;
#include YFM_Helper_GUIApplication // for FetchEnvironment;
//#include <clocale>
#if YCL_Win32
#	include YFM_Win32_YCLib_NLS // for platform_ex::FetchDBCSOffset,
//	platform_ex::WCSToUTF8, platform_ex::UTF8ToWCS;
#endif
#include <ystdex/streambuf.hpp> // for ystdex::membuf;

using namespace ystdex;
using namespace platform;
//! \since build 600
using namespace NPL;

namespace YSLib
{

using namespace Drawing;
using namespace IO;

#if YCL_DS
bool ShowInitializedLog(true);
#endif

namespace
{

//! \since build 551
unique_ptr<ValueNode> p_root;
//! \since build 450
yconstexpr const char TU_MIME[]{u8R"NPLA1(
(application
	(octet-stream "bin" "so")
	(x-msdownload "exe" "dll" "com" "bat" "msi")
	(x-font-ttf "ttf" "ttc")
	(xml "xml")
	(zip "zip")
)
(audio
	(midi "mid" "midi" "rmi")
	(mpeg "mp3")
	(x-mpegurl "m3u")
	(x-wav "wav")
)
(image
	(bmp "bmp")
	(gif "gif")
	(jpeg "jpeg" "jpg")
	(png "png")
	(tif "tif" "tiff")
)
(text
	(html "html" "htm")
	(plain "txt" "conf" "def" "ini" "log" "in")
	(x-c "c" "h" "inl")
	(x-c++ "cc" "cpp" "cxx" "h" "hh" "hpp" "hxx" "inl")
)
)NPLA1"};

#undef CONF_PATH
#undef DATA_DIRECTORY
#undef DEF_FONT_DIRECTORY
#undef DEF_FONT_PATH
#if YCL_Win32 || YCL_Linux
//! \since build 695
const string&
FetchWorkingRoot()
{
	static const struct Init
	{
		string Path;

		Init()
			: Path([]{
#	if YCL_Win32
				IO::Path image(platform::ucast(
					platform_ex::FetchModuleFileName().data()));

				if(!image.empty())
				{
					image.pop_back();

					const auto& dir(image.Verify());

					if(!dir.empty() && dir.back() == FetchSeparator<char16_t>())
						return dir.GetMBCS();
				}
#	elif YCL_Android
				const char*
					sd_paths[]{"/sdcard/", "/mnt/sdcard/", "/storage/sdcard0/"};

				for(const auto& path : sd_paths)
					if(IO::VerifyDirectory(path))
					{
						YTraceDe(Informative, "Successfully found SD card path"
							" '%s' as root path.", path);
						return path;
					}
					else
						YTraceDe(Informative,
							"Failed accessing SD card path '%s'.", path);
#	elif YCL_Linux
				// FIXME: What if link reading failed (i.e. permission denied)?
				// XXX: Link content like 'node_type:[inode]' is not supported.
				// TODO: Use implemnetation for BSD family OS, etc.
				auto image(IO::ResolvePath<ystdex::path<vector<string>,
					IO::PathTraits>>(string_view("/proc/self/exe")));

				if(!image.empty())
				{
					image.pop_back();
				
					const auto& dir(IO::VerifyDirectoryPathTail(
						ystdex::to_string_d(image)));

					if(!dir.empty() && dir.back() == FetchSeparator<char>())
						return dir;
				}
#	else
#		error "Unsupported platform found."
#	endif
				throw GeneralEvent("Failed finding working root path.");
			}())
		{
			YTraceDe(Informative, "Initialized root directory path '%s'.",
				Path.c_str());
		}
	} init;

	return init.Path;
}

// TODO: Reduce overhead?
#	define CONF_PATH (FetchWorkingRoot() + "yconf.txt").c_str()
#endif
#if YCL_DS
#	define DATA_DIRECTORY "/Data/"
#	define DEF_FONT_DIRECTORY "/Font/"
#	define DEF_FONT_PATH "/Font/FZYTK.TTF"
#elif YCL_Win32
#	define DATA_DIRECTORY FetchWorkingRoot()
#	define DEF_FONT_PATH (FetchSystemFontDirectory_Win32() + "SimSun.ttc")
//! \since build 693
inline PDefH(string, FetchSystemFontDirectory_Win32, )
	// NOTE: Hard-coded as Shell32 special path with %CSIDL_FONTS or
	//	%CSIDL_FONTS. See https://msdn.microsoft.com/en-us/library/dd378457.aspx.
	ImplRet(platform_ex::WCSToUTF8(platform_ex::FetchWindowsPath()) + "Fonts\\")
#elif YCL_Android
#	define DATA_DIRECTORY (FetchWorkingRoot() + "Data/")
#	define DEF_FONT_DIRECTORY "/system/fonts/"
#	define DEF_FONT_PATH "/system/fonts/DroidSansFallback.ttf"
#elif YCL_Linux
#	define DATA_DIRECTORY FetchWorkingRoot()
#	define DEF_FONT_PATH "./SimSun.ttc"
#else
#	error "Unsupported platform found."
#endif
#ifndef CONF_PATH
#	define CONF_PATH "yconf.txt"
#endif
#ifndef DATA_DIRECTORY
#	define DATA_DIRECTORY "./"
#endif
#ifndef DEF_FONT_DIRECTORY
#	define DEF_FONT_DIRECTORY DATA_DIRECTORY
#endif

//! \since build 712
MappedFile
LoadMappedModule(const string& path)
{
	TryRet(MappedFile(path))
	CatchExpr(..., std::throw_with_nested(
		GeneralEvent("Loading module '" + path + "' failed.")))
	YAssert(false, "Unreachable control found.");
}

#if YCL_Win32
//! \since build 641
char16_t(*cp113_lkp_backup)(byte, byte);
//! \since build 552
//@{
const unsigned short* p_dbcs_off_936;

void
LoadCP936_NLS()
{
	using namespace platform_ex;

	if((p_dbcs_off_936 = FetchDBCSOffset(936)))
	{
		cp113_lkp_backup = CHRLib::cp113_lkp;
		CHRLib::cp113_lkp = [](byte seq0, byte seq1) ynothrowv -> char16_t{
			return p_dbcs_off_936[p_dbcs_off_936[seq0] + seq1];
		};
	}
}
//@}
#endif

//! \since build 721
void
WriteNPLA1Stream(std::ostream& os, NPL::Configuration&& conf)
{
	ystdex::write_literal(os, Text::BOM_UTF_8) << std::move(conf);
}

//! \since build 693
void
CreateDefaultNPLA1ForStream(std::ostream& os, ValueNode(*creator)())
{
	WriteNPLA1Stream(os, creator());
}

ValueNode
TryReadNPLStream(std::istream& is)
{
	array<char, 3> buf;

	is.read(buf.data(), 3);
	if(Text::CheckBOM(buf.data(), Text::BOM_UTF_8))
	{
		NPL::Configuration conf;

		is >> conf;
		YTraceDe(Debug, "Plain configuration loaded.");
		if(!conf.GetNodeRRef().empty())
			return conf.GetNodeRRef();
		YTraceDe(Warning, "Empty configuration found.");
		throw GeneralEvent("Invalid file found when reading configuration.");
	}
	else
		throw GeneralEvent("Wrong encoding of configuration file.");
}

//! \since build 711
ValueNode
LoadNPLA1InMemory(ValueNode(*creator)())
{
	std::stringstream oss;

	CreateDefaultNPLA1ForStream(oss, creator);
	return TryReadNPLStream(oss);
}

YB_NONNULL(1, 2) bool
CreateDefaultNPLA1File(const char* disp, const char* path,
	ValueNode(*creator)(), bool show_info)
{
	YAssertNonnull(disp),
	YAssertNonnull(path);
	if(show_info)
		YTraceDe(Notice, "Creating %s '%s'...", disp, path);
	if(creator)
	{
		YTraceDe(Debug, "Creator found.");

		ystdex::swap_guard<int, void, decltype(errno)&> gd(errno, 0);

		// XXX: Failed on race condition detected.
		UniqueFile ufile(uopen(path, omode_conv(std::ios_base::out
			| std::ios_base::trunc | platform::ios_noreplace)));
		// TODO: Use shared locking.
		auto fd(ufile.get());
		unique_lock<FileDescriptor> lck(fd);

		if(ofstream ofs{std::move(ufile)})
		{
			CreateDefaultNPLA1ForStream(ofs, creator);
			YTraceDe(Debug, "Created configuration.");
			lck.release();
			return {};
		}
		YTraceDe(Warning, "Cannot create file, possible error (from errno)"
			" = %d: %s.", errno, std::strerror(errno));
	}
	return true;
}

} // unnamed namespace;

void
ExtractInitException(const std::exception& e, string& res) ynothrow
{
	ExtractException(
		[&](const string& str, RecordLevel, size_t level){
		res += string(level, ' ') + "ERROR: " + str + '\n';
	}, e);
}

void
TraceForOutermost(const std::exception& e, RecordLevel lv) ynothrow
{
#if YCL_DS
	ShowInitializedLog = {};
#endif
	if(const auto p = dynamic_cast<const YSLib::FatalError*>(&e))
	{
#if YCL_Win32
		using platform_ex::UTF8ToWCS;

		FilterExceptions([&]{
			::MessageBoxW({}, UTF8ToWCS(p->GetContent()).c_str(),
				UTF8ToWCS(p->GetTitle()).data(), MB_ICONERROR);
		});
#endif
		YTraceDe(Emergent, "%s\n\n%s", p->GetTitle(), p->GetContent().data());
		terminate();
	}
	else
		ExtractAndTrace(e, lv);
}

ValueNode
LoadNPLA1File(const char* disp, const char* path, ValueNode(*creator)(),
	bool show_info)
{
	if(!ufexists(path))
	{
		YTraceDe(Debug, "Path '%s' access failed.", path);
		if(CreateDefaultNPLA1File(disp, path, creator, show_info))
		{
			YTraceDe(Warning, "Creating default file failed,"
				" trying fallback in memory...");
			return LoadNPLA1InMemory(creator);
		}
	}
	if(show_info)
		YTraceDe(Notice, "Found %s '%s'.", Nonnull(disp), path);

	// XXX: Race condition may cause failure, though file would not be
	//	corrupted now.
	auto res(TryInvoke([&]() -> ValueNode{
		MappedFile mfile(path);
		// TODO: Use shared locking.
		auto fd(mfile.GetFile());
		lock_guard<FileDescriptor> lck(fd);

		ystdex::membuf mbuf(ystdex::replace_cast<const char*>(mfile.GetPtr()),
			mfile.GetSize());
		std::istream is(&mbuf);

		if(is)
		{
			YTraceDe(Debug, "Found accessible configuration file.");
			return TryReadNPLStream(is);
		}
		return {};
	}));

	if(res)
		return res;
	YTraceDe(Warning, "Newly created configuration corrupted,"
		" trying fallback in memory...");
	return LoadNPLA1InMemory(creator);
}

void
LoadComponents(Application& app, const ValueNode& node)
{
	const auto& data_dir(AccessChild<string>(node, "DataDirectory"));
	const auto& font_path(AccessChild<string>(node, "FontFile"));
	const auto& font_dir(AccessChild<string>(node, "FontDirectory"));

	if(!data_dir.empty() && !font_path.empty() && !font_dir.empty())
		YTraceDe(Notice, "Loaded default directory:\n%s\nLoaded default"
			" font path:\n%s\nLoaded default font directory:\n%s",
			data_dir.c_str(), font_path.c_str(), font_dir.c_str());
	else
		throw GeneralEvent("Empty path loaded.");
#if !CHRLib_NoDynamicMapping

	static MappedFile mapped;
	const string mapping_name(data_dir + "cp113.bin");

	YTraceDe(Notice, "Loading character mapping file '%s' ...",
		mapping_name.c_str());
	try
	{
		mapped = LoadMappedModule(data_dir + "cp113.bin");
		if(mapped.GetSize() != 0)
			CHRLib::cp113 = mapped.GetPtr();
		else
			throw GeneralEvent("Failed loading CHRMapEx.");
		YTraceDe(Notice, "CHRMapEx loaded successfully.");
	}
	catch(std::exception& e)
	{
		YTraceDe(Notice, "Module cp113.bin loading failed, error: %s",
			e.what());
#	if YCL_Win32
		LoadCP936_NLS();
		if(p_dbcs_off_936)
			YTraceDe(Notice, "NLS CP936 used as fallback.");
		else
#	endif
		{
			CHRLib::cp113_lkp = [](byte, byte) YB_ATTR(noreturn) -> char16_t{
				throw LoggedEvent("Failed calling conversion for CHRMapEx.");
			};
			YTraceDe(Warning, "CHRMapEx conversion calls would lead to"
				" exception thrown.");
		}
	}
	app.AddExitGuard([]() ynothrow{
#	if YCL_Win32
		if(cp113_lkp_backup)
		{
			CHRLib::cp113_lkp = cp113_lkp_backup;
			cp113_lkp_backup = {};
		}
#	endif
		mapped = MappedFile();
		YTraceDe(Notice, "Character mapping deleted.");
	});
#endif
	YTraceDe(Notice, "Trying entering directory '%s' ...", data_dir.c_str());
	if(!IO::VerifyDirectory(data_dir))
		throw GeneralEvent("Invalid default data directory found.");
	if(!(ufexists(font_path.c_str()) || IO::VerifyDirectory(font_dir)))
		throw GeneralEvent("Invalid default font file path found.");
}

ValueNode
LoadConfiguration(bool show_info)
{
	return LoadNPLA1File("configuration file", CONF_PATH, []{
		return ValueNode(NodeLiteral{"YFramework",
			{{"DataDirectory", DATA_DIRECTORY}, {"FontFile", DEF_FONT_PATH},
			{"FontDirectory", DEF_FONT_DIRECTORY}}});
	}, show_info);
}

void
SaveConfiguration(const ValueNode& node)
{
	UniqueFile ufile(uopen(CONF_PATH,
		omode_conv(std::ios_base::out | std::ios_base::trunc)));
	auto fd(ufile.get());
	// TODO: Use shared locking.
	unique_lock<FileDescriptor> lck(fd);

	if(ofstream ofs{std::move(ufile)})
	{
		YTraceDe(Debug, "Writing configuration...");
		WriteNPLA1Stream(ofs, ValueNode(node.GetContainer()));
		lck.release();
	}
	else
		throw GeneralEvent("Invalid file found when writing configuration.");
	YTraceDe(Debug, "Writing configuration done.");
}


void
InitializeSystemFontCache(FontCache& fc, const string& font_file,
	const string& font_dir)
{
	string res;

	YTraceDe(Notice, "Loading font files...");
	try
	{
		size_t loaded(fc.LoadTypefaces(font_file) != 0 ? 1 : 0);

		if(!font_dir.empty())
			try
			{
				IO::TraverseChildren(font_dir,
					[&](NodeCategory c, NativePathView npv){
					if(!bool(c & NodeCategory::Directory))
					{
						const FontPath path(font_dir + String(npv).GetMBCS());

						if(path != font_file && fc.LoadTypefaces(path) != 0)
							++loaded;
					}
				});
			}
			CatchExpr(std::system_error& e, YTraceDe(Warning,
				"Failed loading font directory."), ExtractAndTrace(e, Warning))
		fc.InitializeDefaultTypeface();

		const auto faces(fc.GetFaces().size());

		if(faces != 0)
			YTraceDe(Notice, "%zu face(s) in %zu font file(s)"
				" are loaded\nsuccessfully.\n", faces, loaded);
		else
			throw GeneralEvent("No fonts found.");
		YTraceDe(Notice, "Setting default font face...");
		if(const auto pf = fc.GetDefaultTypefacePtr())
			YTraceDe(Notice, "\"%s\":\"%s\",\nsuccessfully.",
				pf->GetFamilyName().c_str(), pf->GetStyleName().c_str());
		else
			throw GeneralEvent("Setting default font face failed.");
		return;
	}
	CatchExpr(std::exception& e, ExtractInitException(e, res))
	CatchExpr(..., res += "Unknown exception @ InitializeSystemFontCache.\n")
	throw FatalError("      Font Caching Failure      ",
		" Please make sure the fonts are\n"
		" stored in correct path.\n" + res);
}


ValueNode&
FetchRoot() ynothrow
{
	return FetchEnvironment().Root;
}

Drawing::FontCache&
FetchDefaultFontCache()
{
	static unique_ptr<Drawing::FontCache> p_font_cache;

	if(YB_UNLIKELY(!p_font_cache))
	{
		p_font_cache.reset(new Drawing::FontCache());
		FetchAppInstance().AddExitGuard([&]() ynothrow{
			p_font_cache.reset();
		});

		const auto& node(FetchEnvironment().Root["YFramework"]);

		InitializeSystemFontCache(*p_font_cache,
			AccessChild<string>(node, "FontFile"),
			AccessChild<string>(node, "FontDirectory"));
	}
#if YCL_DS
	// XXX: Actually this should be set after %InitVideo call.
	ShowInitializedLog = {};
#endif
	return *p_font_cache;
}

MIMEBiMapping&
FetchMIMEBiMapping()
{
	static unique_ptr<MIMEBiMapping> p_mapping;

	if(YB_UNLIKELY(!p_mapping))
	{
		using namespace NPL;

		p_mapping.reset(new MIMEBiMapping());
		AddMIMEItems(*p_mapping, LoadNPLA1File("MIME database",
			(AccessChild<string>(FetchEnvironment().Root["YFramework"],
			"DataDirectory") + "MIMEExtMap.txt").c_str(), []{
				return A1::LoadNode(SContext::Analyze(NPL::Session(TU_MIME)));
			}, true));
		FetchAppInstance().AddExitGuard([&]() ynothrow{
			p_mapping.reset();
		});
	}
	return *p_mapping;
}

} // namespace YSLib;

