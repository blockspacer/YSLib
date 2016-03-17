﻿/*
	© 2012-2016 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file ValueNode.h
\ingroup Core
\brief 值类型节点。
\version r2374
\author FrankHB <frankhb1989@gmail.com>
\since build 338
\par 创建时间:
	2012-08-03 23:03:44 +0800
\par 修改时间:
	2016-03-17 16:38 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	YSLib::Core::ValueNode
*/


#ifndef YSL_INC_Core_ValueNode_h_
#define YSL_INC_Core_ValueNode_h_ 1

#include "YModules.h"
#include YFM_YSLib_Core_YObject
#include <ystdex/path.hpp>
#include <ystdex/set.hpp> // for ystdex::mapped_set;
#include <numeric> // for std::accumulate;

namespace YSLib
{

//! \since build 674
//@{
//! \brief 标记列表构造容器。
yconstexpr const struct ListContainerTag{} ListContainer{};

//! \brief 标记不使用容器。
yconstexpr const struct NoContainerTag{} NoContainer{};
//@}


//! \since build 678
//@{
class ValueNode;

inline PDefH(const ValueNode&, AsNode, const ValueNode& node)
	ImplRet(node)
/*!
\brief 传递指定名称和值参数构造值类型节点。
\since build 668
*/
template<typename _tString, typename... _tParams>
inline ValueNode
AsNode(_tString&&, _tParams&&...);
//@}


/*!
\brief 值类型节点。
\warning 非虚析构。
\since build 330

包含名称字符串和值类型对象的对象节点。
*/
class YF_API ValueNode : private ystdex::totally_ordered<ValueNode>,
	private ystdex::totally_ordered<ValueNode, string>
{
public:
	using Container = ystdex::mapped_set<ValueNode, ystdex::less<>>;
	//! \since build 678
	using key_type = typename Container::key_type;
	//! \since build 460
	using iterator = Container::iterator;
	//! \since build 460
	using const_iterator = Container::const_iterator;

private:
	string name{};
	/*!
	\brief 子节点容器。
	\since build 667
	*/
	Container container{};

public:
	//! \since build 667
	ValueObject Value{};

	DefDeCtor(ValueNode)
	/*!
	\brief 构造：使用容器对象。
	\since build 502
	*/
	ValueNode(Container con)
		: container(std::move(con))
	{}
	/*!
	\brief 构造：使用字符串引用和值类型对象构造参数。
	\note 不使用容器。
	\since build 674
	*/
	template<typename _tString, typename... _tParams>
	inline
	ValueNode(NoContainerTag, _tString&& str, _tParams&&... args)
		: name(yforward(str)), Value(yforward(args)...)
	{}
	/*!
	\brief 构造：使用容器对象、字符串引用和值类型对象构造参数。
	\since build 502
	*/
	template<typename _tString, typename... _tParams>
	ValueNode(Container con, _tString&& str, _tParams&&... args)
		: name(yforward(str)), container(std::move(con)),
		Value(yforward(args)...)
	{}
	/*!
	\brief 构造：使用输入迭代器对。
	\since build 337
	*/
	template<typename _tIn>
	inline
	ValueNode(const pair<_tIn, _tIn>& pr)
		: container(pr.first, pr.second)
	{}
	/*!
	\brief 构造：使用输入迭代器对、字符串引用和值参数。
	\since build 340
	*/
	template<typename _tIn, typename _tString>
	inline
	ValueNode(const pair<_tIn, _tIn>& pr, _tString&& str)
		: name(yforward(str)), container(pr.first, pr.second)
	{}
	DefDeCopyMoveCtor(ValueNode)

	/*!
	\brief 合一赋值：使用值参数和交换函数进行复制或转移赋值。
	\since build 502
	*/
	PDefHOp(ValueNode&, =, ValueNode node) ynothrow
		ImplRet(node.swap(*this), *this)

	//! \since build 336
	DefBoolNeg(explicit, bool(Value) || !container.empty())

	//! \since build 667
	//@{
	PDefHOp(const ValueNode&, +=, const ValueNode& node)
		ImplRet(Add(node), *this)
	PDefHOp(const ValueNode&, +=, ValueNode&& node)
		ImplRet(Add(std::move(node)), *this)

	PDefHOp(const ValueNode&, -=, const ValueNode& node)
		ImplRet(Remove(node), *this)
	PDefHOp(const ValueNode&, -=, const string& str)
		ImplRet(Remove(str), *this)
	/*!
	\brief 替换同名子节点。
	\return 自身引用。
	*/
	//@{
	PDefHOp(ValueNode&, /=, const ValueNode& node)
		ImplRet(*this %= node, *this)
	PDefHOp(ValueNode&, /=, ValueNode&& node)
		ImplRet(*this %= std::move(node), *this)
	//@}
	/*!
	\brief 替换同名子节点。
	\return 子节点引用。
	*/
	//@{
	const ValueNode&
	operator%=(const ValueNode&);
	const ValueNode&
	operator%=(const ValueNode&&);
	//@}
	//@}

	//! \since build 673
	friend PDefHOp(bool, ==, const ValueNode& x, const ValueNode& y) ynothrow
		ImplRet(x.name == y.name)

	//! \since build 673
	friend PDefHOp(bool, <, const ValueNode& x, const ValueNode& y) ynothrow
		ImplRet(x.name < y.name)
	//! \since build 678
	friend PDefHOp(bool, <, const ValueNode& x, const string& str) ynothrow
		ImplRet(x.name < str)

	//! \since build 667
	ValueNode&
	operator[](const string&);
	//! \since build 667
	template<class _tCon>
	const ValueNode&
	operator[](const ystdex::path<_tCon>& pth)
	{
		auto p(this);

		for(const auto& n : pth)
			p = &(*p)[n];
		return *p;
	}

	/*!
	\brief 取子节点容器引用。
	\since build 664
	*/
	DefGetter(const ynothrow, const Container&, Container, container)
	/*!
	\brief 取子节点容器引用。
	\since build 667
	*/
	DefGetter(ynothrow, Container&, ContainerRef, container)
	DefGetter(const ynothrow, const string&, Name, name)

	//! \since build 666
	//@{
	//! \brief 设置子节点容器内容。
	PDefH(void, SetChildren, Container con)
		ImplExpr(container = std::move(con))

	PDefH(bool, Add, const ValueNode& node)
		ImplRet(insert(node).second)
	PDefH(bool, Add, ValueNode&& node)
		ImplRet(insert(std::move(node)).second)
	//! \since build 674
	template<typename _tString, typename... _tParams>
	inline bool
	Add(_tString&& str, _tParams&&... args)
	{
		return AddValueTo(container, yforward(str), yforward(args)...);
	}

	//! \since build 674
	template<typename _tString, typename... _tParams>
	static bool
	AddValueTo(Container& con, _tString&& str, _tParams&&... args)
	{
		// TODO: Blocked. Use %string as argument using C++14
		//	heterogeneous %equal_range template.
		const auto pr(con.equal_range(YSLib::AsNode(str)));

		if(pr.first == pr.second)
		{
			con.emplace_hint(pr.first, NoContainer, yforward(str),
				yforward(args)...);
			return true;
		}
		return {};
	}

	/*!
	\brief 清除节点。
	\post <tt>!Value && empty()</tt> 。
	*/
	PDefH(void, Clear, ) ynothrow
		ImplExpr(Value.Clear(), ClearContainer())

	/*!
	\brief 清除节点容器。
	\post \c empty() 。
	*/
	PDefH(void, ClearContainer, ) ynothrow
		ImplExpr(container.clear())

	//! \since build 678
	//@{
	template<typename _type, typename _tString, typename... _tParams>
	_type&
	EmplaceForTypedValue(_tString&& str, _tParams&&... args)
	{
		return EmplaceForTypedValueTo<_type>(GetContainerRef(), yforward(str),
			yforward(args)...);
	}
	template<typename _type, typename _tString, typename... _tParams>
	static _type&
	EmplaceForTypedValueTo(Container& con, _tString&& str, _tParams&&... args)
	{
		return EmplaceTypedValueTo<_type>(con, yforward(str), yforward(args)...)
			.first->Value.template GetObject<_type>();
	}

	template<typename _type, typename _tString, typename... _tParams>
	static std::pair<iterator, bool>
	EmplaceTypedValueTo(Container& con, _tString&& str, _tParams&&... args)
	{
		// TODO: Blocked. Use %string as argument using C++14
		//	heterogeneous %find template.
		auto pr(ystdex::search_map(con, YSLib::AsNode(str)));

		if(pr.second)
			pr.first = EmplaceTypedValueTo<_type>(con, const_iterator(pr.first),
				yforward(str), yforward(args)...);
		return pr;
	}
	template<typename _type, typename _tString, typename... _tParams>
	static inline iterator
	EmplaceTypedValueTo(Container& con, const_iterator hint, _tString&& str,
		_tParams&&... args)
	{
		return EmplaceValueWithHintTo(con, hint, yforward(str),
			InPlaceTag<_type>(), yforward(args)...);
	}
	//@}

	//! \since build 674
	template<typename _tString, typename... _tParams>
	static inline auto
	EmplaceValueTo(Container& con, _tString&& str, _tParams&&... args)
		-> decltype(con.emplace(NoContainer, yforward(str), yforward(args)...))
	{
		return con.emplace(NoContainer, yforward(str), yforward(args)...);
	}

	//! \since build 678
	template<typename _tString, typename... _tParams>
	static inline auto
	EmplaceValueWithHintTo(Container& con, const_iterator i, _tString&& str,
		_tParams&&... args) -> decltype(con.emplace_hint(i, NoContainer,
		yforward(str), yforward(args)...))
	{
		return
			con.emplace_hint(i, NoContainer, yforward(str), yforward(args)...);
	}

	bool
	Remove(const ValueNode&);
	PDefH(bool, Remove, const string& str)
		ImplRet(Remove(AsNode(str)))
	//@}

	/*!
	\brief 复制满足条件的子节点。
	\since build 664
	*/
	template<typename _func>
	Container
	SelectChildren(_func f) const
	{
		Container res;

		ystdex::for_each_if(begin(), end(), f, [&](const ValueNode& nd){
			res.insert(nd);
		});
		return res;
	}

	//! \since build 667
	//@{
	//! \brief 转移满足条件的子节点。
	template<typename _func>
	Container
	SplitChildren(_func f)
	{
		Container res;

		std::for_each(begin(), end(), [&](const ValueNode& nd){
			EmplaceValueTo(res, nd.GetName());
		});
		ystdex::for_each_if(begin(), end(), f, [&, this](const ValueNode& nd){
			const auto& child_name(nd.GetName());

			// TODO: Blocked. Use %string as argument using C++14
			//	heterogeneous %find template.
			Deref(res.find(AsNode(child_name))).Value
				= std::move(nd.Value);
			Remove(child_name);
		});
		return res;
	}

	//! \warning 不检查容器之间的所有权，保持循环引用状态析构导致行为未定义。
	//@{
	//! \brief 交换容器。
	PDefH(void, SwapContainer, ValueNode& node) ynothrow
		ImplExpr(container.swap(node.container))

	//! \brief 交换容器和值。
	void
	SwapContent(ValueNode&) ynothrow;
	//@}
	//@}

	//! \since build 460
	//@{
	PDefH(iterator, begin, )
		ImplRet(GetContainerRef().begin())
	PDefH(const_iterator, begin, ) const
		ImplRet(GetContainer().begin())

	//! \since build 598
	//@{
	DefFwdTmpl(const, pair<iterator YPP_Comma bool>, emplace,
		container.emplace(yforward(args)...))

	//! \since build 667
	DefFwdTmpl(, iterator, emplace_hint,
		container.emplace_hint(yforward(args)...))

	PDefH(bool, empty, ) const ynothrow
		ImplRet(container.empty())
	//@}

	PDefH(iterator, end, )
		ImplRet(GetContainerRef().end())
	PDefH(const_iterator, end, ) const
		ImplRet(GetContainer().end())
	//@}

	//! \since build 667
	DefFwdTmpl(-> decltype(container.insert(yforward(args)...)), auto,
		insert, container.insert(yforward(args)...))

	//! \since build 678
	//@{
	template<typename... _tParams>
	std::pair<iterator, bool>
	try_emplace(const key_type& k, _tParams&&... args)
	{
		return EmplaceValueTo(container, k, yforward(args)...);
	}
	template<typename... _tParams>
	iterator
	try_emplace(const key_type&& k, _tParams&&... args)
	{
		return EmplaceValueWithHintTo(container, k, yforward(args)...);
	}
	template<typename... _tParams>
	std::pair<iterator, bool>
	try_emplace(key_type&& k, _tParams&&... args)
	{
		return EmplaceValueTo(container, std::move(k), yforward(args)...);
	}
	template<typename... _tParams>
	iterator
	try_emplace(key_type&& k, _tParams&&... args)
	{
		return EmplaceValueWithHintTo(container, std::move(k), yforward(args)...);
	}
	//@}

	//! \since build 598
	PDefH(size_t, size, ) const ynothrow
		ImplRet(container.size())

	/*!
	\brief 交换。
	\since build 501
	*/
	void
	swap(ValueNode&) ynothrow;

	/*!
	\sa ystdex::mapped_set
	\sa ystdex::set_value_move
	\since build 671
	*/
	friend ValueNode
	set_value_move(ValueNode&& node)
	{
		return {std::move(node.GetContainerRef()), node.GetName(),
			std::move(node.Value)};
	}
};

//! \relates ValueNode
//@{
//! \since build 666
//@{
/*!
\brief 访问节点的指定类型对象。
\exception std::bad_cast 空实例或类型检查失败 。
*/
//@{
template<typename _type>
inline _type&
Access(ValueNode& node)
{
	return node.Value.Access<_type>();
}
template<typename _type>
inline const _type&
Access(const ValueNode& node)
{
	return node.Value.Access<_type>();
}
//@}

//! \since build 670
//@{
//! \brief 访问节点的指定类型对象指针。
//@{
template<typename _type>
inline observer_ptr<_type>
AccessPtr(ValueNode& node) ynothrow
{
	return node.Value.AccessPtr<_type>();
}
template<typename _type>
inline observer_ptr<const _type>
AccessPtr(const ValueNode& node) ynothrow
{
	return node.Value.AccessPtr<_type>();
}
//@}
//! \brief 访问节点的指定类型对象指针。
//@{
template<typename _type>
inline observer_ptr<_type>
AccessPtr(observer_ptr<ValueNode> p_node) ynothrow
{
	return p_node ? AccessPtr<_type>(*p_node) : nullptr;
}
template<typename _type>
inline observer_ptr<const _type>
AccessPtr(observer_ptr<const ValueNode> p_node) ynothrow
{
	return p_node ? AccessPtr<_type>(*p_node) : nullptr;
}
//@}
//@}
//@}

//! \since build 501
inline DefSwap(ynothrow, ValueNode)
//@}


//! \since build 670
//@{
/*!
\brief 访问节点。
\throw std::out_of_range 未找到对应节点。
*/
//@{
//! \since build 666
YF_API ValueNode&
AccessNode(ValueNode::Container*, const string&);
//! \since build 433
YF_API const ValueNode&
AccessNode(const ValueNode::Container*, const string&);
inline PDefH(ValueNode&, AccessNode, observer_ptr<ValueNode::Container> p_con,
	const string& name)
	ImplRet(AccessNode(p_con.get(), name))
inline PDefH(const ValueNode&, AccessNode,
	observer_ptr<const ValueNode::Container> p_con, const string& name)
	ImplRet(AccessNode(p_con.get(), name))
//! \since build 666
inline PDefH(ValueNode&, AccessNode, ValueNode::Container& con,
	const string& name)
	ImplRet(AccessNode(&con, name))
//! \since build 399
inline PDefH(const ValueNode&, AccessNode, const ValueNode::Container& con,
	const string& name)
	ImplRet(AccessNode(&con, name))
//! \note 时间复杂度 O(n) 。
//@{
YF_API ValueNode&
AccessNode(ValueNode&, size_t);
YF_API const ValueNode&
AccessNode(const ValueNode&, size_t);
//@}
inline PDefH(ValueNode&, AccessNode, ValueNode& node,
	const string& name)
	ImplRet(AccessNode(node.GetContainerRef(), name))
inline PDefH(const ValueNode&, AccessNode, const ValueNode& node,
	const string& name)
	ImplRet(AccessNode(node.GetContainer(), name))
//! \note 使用 ADL \c AccessNode 。
template<class _tNode, typename _tIn>
_tNode&&
AccessNode(_tNode&& node, _tIn first, _tIn last)
{
	return std::accumulate(first, last, ystdex::ref(node),
		[](_tNode&& nd, decltype(*first) c){
		return ystdex::ref(AccessNode(nd, c));
	});
}
//! \note 使用 ADL \c begin 和 \c end 指定范围迭代器。
template<class _tNode, typename _tRange,
	yimpl(typename = typename ystdex::enable_if_t<
	!std::is_constructible<const string&, const _tRange&>::value>)>
inline auto
AccessNode(_tNode&& node, const _tRange& c)
	-> decltype(YSLib::AccessNode(yforward(node), begin(c), end(c)))
{
	return YSLib::AccessNode(yforward(node), begin(c), end(c));
}
//@}

//! \brief 访问节点指针。
//@{
YF_API observer_ptr<ValueNode>
AccessNodePtr(ValueNode::Container&, const string&) ynothrow;
YF_API observer_ptr<const ValueNode>
AccessNodePtr(const ValueNode::Container&, const string&) ynothrow;
inline PDefH(observer_ptr<ValueNode>, AccessNodePtr,
	ValueNode::Container* p_con, const string& name) ynothrow
	ImplRet(p_con ? AccessNodePtr(*p_con, name) : nullptr)
inline PDefH(observer_ptr<const ValueNode>, AccessNodePtr,
	const ValueNode::Container* p_con, const string& name) ynothrow
	ImplRet(p_con ? AccessNodePtr(*p_con, name) : nullptr)
inline PDefH(observer_ptr<ValueNode>, AccessNodePtr,
	observer_ptr<ValueNode::Container> p_con, const string& name) ynothrow
	ImplRet(p_con ? AccessNodePtr(*p_con, name) : nullptr)
inline PDefH(observer_ptr<const ValueNode>, AccessNodePtr,
	observer_ptr<const ValueNode::Container> p_con, const string& name) ynothrow
	ImplRet(p_con ? AccessNodePtr(*p_con, name) : nullptr)
//! \note 时间复杂度 O(n) 。
//@{
YF_API observer_ptr<ValueNode>
AccessNodePtr(ValueNode&, size_t);
YF_API observer_ptr<const ValueNode>
AccessNodePtr(const ValueNode&, size_t);
//@}
inline PDefH(observer_ptr<ValueNode>, AccessNodePtr, ValueNode& node,
	const string& name)
	ImplRet(AccessNodePtr(node.GetContainerRef(), name))
inline PDefH(observer_ptr<const ValueNode>, AccessNodePtr, const ValueNode& node,
	const string& name)
	ImplRet(AccessNodePtr(node.GetContainer(), name))
//! \note 使用 ADL \c AccessNodePtr 。
template<class _tNode, typename _tIn>
auto
AccessNodePtr(_tNode&& node, _tIn first, _tIn last)
	-> decltype(make_obsrever(std::addressof(node)))
{
	// TODO: Simplified using algorithm template?
	for(auto p(make_observer(std::addressof(node))); p && first != last;
		++first)
		p = AccessNodePtr(*p, *first);
	return first;
}
//! \note 使用 ADL \c begin 和 \c end 指定范围迭代器。
template<class _tNode, typename _tRange,
	yimpl(typename = typename ystdex::enable_if_t<
	!std::is_constructible<const string&, const _tRange&>::value>)>
inline auto
AccessNodePtr(_tNode&& node, const _tRange& c)
	-> decltype(YSLib::AccessNodePtr(yforward(node), begin(c), end(c)))
{
	return YSLib::AccessNodePtr(yforward(node), begin(c), end(c));
}

//@}

/*!
\exception std::bad_cast 空实例或类型检查失败 。
\relates ValueNode
*/
//@{
/*!
\brief 访问子节点的指定类型对象。
\note 使用 ADL \c AccessNode 。
*/
//@{
template<typename _type, typename... _tParams>
inline _type&
AccessChild(ValueNode& node, _tParams&&... args)
{
	return Access<_type>(AccessNode(node, yforward(args)...));
}
template<typename _type, typename... _tParams>
inline const _type&
AccessChild(const ValueNode& node, _tParams&&... args)
{
	return Access<_type>(AccessNode(node, yforward(args)...));
}
//@}

//! \brief 访问指定名称的子节点的指定类型对象的指针。
//@{
template<typename _type, typename... _tParams>
inline observer_ptr<_type>
AccessChildPtr(ValueNode& node, _tParams&&... args) ynothrow
{
	return AccessPtr<_type>(
		AccessNodePtr(node.GetContainerRef(), yforward(args)...));
}
template<typename _type, typename... _tParams>
inline observer_ptr<const _type>
AccessChildPtr(const ValueNode& node, _tParams&&... args) ynothrow
{
	return AccessPtr<_type>(
		AccessNodePtr(node.GetContainer(), yforward(args)...));
}
template<typename _type, typename... _tParams>
inline observer_ptr<_type>
AccessChildPtr(ValueNode* p_node, _tParams&&... args) ynothrow
{
	return p_node ? AccessChildPtr<_type>(*p_node, yforward(args)...) : nullptr;
}
template<typename _type, typename... _tParams>
inline observer_ptr<const _type>
AccessChildPtr(const ValueNode* p_node, _tParams&&... args) ynothrow
{
	return p_node ? AccessChildPtr<_type>(*p_node, yforward(args)...) : nullptr;
}
//@}
//@}
//@}


//! \note 结果不含子节点。
//@{
template<typename _tString, typename... _tParams>
inline ValueNode
AsNode(_tString&& str, _tParams&&... args)
{
	return {NoContainer, yforward(str), yforward(args)...};
}

/*!
\brief 传递指定名称和退化值参数构造值类型节点。
\since build 337
*/
template<typename _tString, typename... _tParams>
inline ValueNode
MakeNode(_tString&& str, _tParams&&... args)
{
	return {NoContainer, yforward(str), ystdex::decay_copy(args)...};
}
//@}

/*!
\brief 取指定名称和转换为字符串的值类型节点。
\note 使用非限定 to_string 转换。
\since build 344
*/
template<typename _tString, typename... _tParams>
inline ValueNode
StringifyToNode(_tString&& str, _tParams&&... args)
{
	return {NoContainer, yforward(str), to_string(yforward(args)...)};
}

/*!
\brief 从引用参数取值类型节点：返回自身。
\since build 338
*/
//@{
inline PDefH(const ValueNode&, UnpackToNode, const ValueNode& arg)
	ImplRet(arg)
inline PDefH(ValueNode&&, UnpackToNode, ValueNode&& arg)
	ImplRet(std::move(arg))
//@}
/*!
\brief 从参数取以指定分量为初始化参数的值类型节点。
\note 取分量同 std::get ，但使用 ADL 。仅取前两个分量。
\since build 338
*/
template<class _tPack>
inline ValueNode
UnpackToNode(_tPack&& pk)
{
	return {0, get<0>(yforward(pk)),
		ValueObject(ystdex::decay_copy(get<1>(yforward(pk))))};
}

/*!
\brief 取指定值类型节点为成员的节点容器。
\since build 598
*/
//@{
template<typename _tElem>
inline ValueNode::Container
CollectNodes(std::initializer_list<_tElem> il)
{
	return il;
}
template<typename... _tParams>
inline ValueNode::Container
CollectNodes(_tParams&&... args)
{
	return {yforward(args)...};
}
//@}

/*!
\brief 取以指定分量为参数对应初始化得到的值类型节点为子节点的值类型节点。
\since build 338
*/
template<typename _tString, typename... _tParams>
inline ValueNode
PackNodes(_tString&& name, _tParams&&... args)
{
	return {CollectNodes(UnpackToNode(yforward(args))...), yforward(name)};
}


//! \since build 674
//@{
//! \brief 移除空子节点。
YF_API void
RemoveEmptyChildren(ValueNode::Container&) ynothrow;

/*!
\brief 移除首个子节点。
\pre 断言：节点非空。
*/
//@{
YF_API void
RemoveHead(ValueNode::Container&) ynothrowv;
inline PDefH(void, RemoveHead, ValueNode& term) ynothrowv
	ImplExpr(RemoveHead(term.GetContainerRef()))
//@}
//@}


/*!
\brief 判断字符串是否是一个指定字符和非负整数的组合。
\pre 断言：字符串参数的数据指针非空。
\note 仅测试能被 <tt>unsigned long</tt> 表示的整数。
\since build 659
*/
YF_API bool
IsPrefixedIndex(string_view, char = '$');

/*!
\brief 转换节点大小为新的节点索引值。
\return 保证 4 位十进制数内按字典序递增的字符串。
\throw std::invalid_argument 索引值过大，不能以 4 位十进制数表示。
\note 重复使用作为新节点的名称，可用于插入不重复节点。
\since build 598
*/
//@{
YF_API string
MakeIndex(size_t);
inline PDefH(string, MakeIndex, const ValueNode::Container& con)
	ImplRet(MakeIndex(con.size()))
inline PDefH(string, MakeIndex, const ValueNode& node)
	ImplRet(MakeIndex(node.GetContainer()))
//@}


/*!
\brief 节点序列容器。
\since build 597

除分配器外满足和 std::vector 相同的要求的模板的一个实例，元素为 ValueNode 类型。
*/
using NodeSequence = yimpl(YSLib::vector)<ValueNode>;


/*!
\brief 包装节点的组合字面量。
\since build 598
*/
class YF_API NodeLiteral final
{
private:
	ValueNode node;

public:
	//! \since build 599
	NodeLiteral(const ValueNode& nd)
		: node(nd)
	{}
	//! \since build 599
	NodeLiteral(ValueNode&& nd)
		: node(std::move(nd))
	{}
	NodeLiteral(const string& str)
		: node(NoContainer, str)
	{}
	NodeLiteral(const string& str, string val)
		: node(NoContainer, str, std::move(val))
	{}
	template<typename _tLiteral = NodeLiteral>
	NodeLiteral(const string& str, std::initializer_list<_tLiteral> il)
		: node(NoContainer, str, NodeSequence(il.begin(), il.end()))
	{}
	//! \since build 674
	template<typename _tLiteral = NodeLiteral, class _tString,
		typename... _tParams>
	NodeLiteral(ListContainerTag, _tString&& str,
		std::initializer_list<_tLiteral> il, _tParams&&... args)
		: node(ValueNode::Container(il.begin(), il.end()), yforward(str),
		yforward(args)...)
	{}
	DefDeCopyMoveCtorAssignment(NodeLiteral)

	DefCvt(ynothrow, ValueNode&, node)
	DefCvt(const ynothrow, const ValueNode&, node)
};

/*!
\brief 传递参数构造值类型节点字面量。
\relates NodeLiteral
\since build 668
*/
template<typename _tString, typename _tLiteral = NodeLiteral>
inline NodeLiteral
AsNodeLiteral(_tString&& name, std::initializer_list<_tLiteral> il)
{
	return {ListContainer, yforward(name), il};
}

} // namespace YSLib;

#endif

