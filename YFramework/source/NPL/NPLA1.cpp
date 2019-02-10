﻿/*
	© 2014-2019 FrankHB.

	This file is part of the YSLib project, and may only be used,
	modified, and distributed under the terms of the YSLib project
	license, LICENSE.TXT.  By continuing to use, modify, or distribute
	this file you indicate that you have read the license and
	understand and accept it fully.
*/

/*!	\file NPLA1.cpp
\ingroup NPL
\brief NPLA1 公共接口。
\version r9975
\author FrankHB <frankhb1989@gmail.com>
\since build 473
\par 创建时间:
	2014-02-02 18:02:47 +0800
\par 修改时间:
	2019-02-10 14:28 +0800
\par 文本编码:
	UTF-8
\par 模块名称:
	NPL::NPLA1
*/


#include "NPL/YModules.h"
#include YFM_NPL_NPLA1 // for YSLib, ystdex::bind1, std::make_move_iterator,
//	ystdex::value_or, shared_ptr, tuple, list, lref, vector, observer_ptr, set,
//	owner_less, ystdex::erase_all, ystdex::as_const, NPL::make_observer,
//	std::find_if, NPL::AllocateEnvironment, unordered_map, AccessPtr,
//	YSLib::allocate_shared, ystdex::id, ystdex::cast_mutable, pair, share_move,
//	ystdex::equality_comparable, std::allocator_arg, NoContainer,
//	ystdex::exchange, NPL::SwitchToFreshEnvironment, ystdex::invoke_value_or,
//	ystdex::call_value_or, ystdex::make_transform, in_place_type,
//	ystdex::try_emplace, NPL::Access;
#include <ystdex/scope_guard.hpp> // for ystdex::guard, ystdex::swap_guard,
//	ystdex::unique_guard;
#include YFM_NPL_SContext // for Session;

using namespace YSLib;

namespace NPL
{

// NOTE: The following options provide documented alternative implementations.
//	They are for exposition only. The code without TCO or thunk works without
//	several guarantees in the specification, so it is not conforming.

#define YF_Impl_NPLA1_Enable_TCO true
#define YF_Impl_NPLA1_Enable_TailRewriting true
#define YF_Impl_NPLA1_Enable_Thunked true

// NOTE: This is only useful for TCO frame compression and disabled by default.
#define YF_Impl_NPLA1_Enable_WeakExternalRoots false

// NOTE: Use %RelaySwitched instead of %RelayNext as possible to be more
//	friendly to TCO.

//! \since build 820
static_assert(!YF_Impl_NPLA1_Enable_TCO || YF_Impl_NPLA1_Enable_Thunked,
	"Invalid combination of build options found.");
//! \since build 851
static_assert(!YF_Impl_NPLA1_Enable_TailRewriting || YF_Impl_NPLA1_Enable_TCO,
	"Invalid combination of build options found.");

namespace A1
{

string
to_string(ValueToken vt)
{
	switch(vt)
	{
	case ValueToken::Null:
		return "null";
	case ValueToken::Undefined:
		return "undefined";
	case ValueToken::Unspecified:
		return "unspecified";
	case ValueToken::GroupingAnchor:
		return "grouping";
	case ValueToken::OrderedAnchor:
		return "ordered";
	}
	throw std::invalid_argument("Invalid value token found.");
}

void
InsertChild(ValueNode&& node, ValueNode::Container& con)
{
	con.insert(node.GetName().empty() ? YSLib::AsNode(node.get_allocator(),
		'$' + MakeIndex(con.size()), std::move(node.Value)) : std::move(node));
}

ValueNode
TransformNode(const TermNode& term, NodeMapper mapper, NodeMapper map_leaf_node,
	TermNodeToString term_to_str, NodeInserter insert_child)
{
	auto s(term.size());

	if(s == 0)
		return map_leaf_node(term);

	auto i(term.begin());
	const auto nested_call(ystdex::bind1(TransformNode, mapper, map_leaf_node,
		term_to_str, insert_child));

	if(s == 1)
		return nested_call(*i);

	const auto& name(term_to_str(*i));

	if(!name.empty())
		yunseq(++i, --s);
	if(s == 1)
	{
		auto&& nd(nested_call(*i));

		if(nd.GetName().empty())
			return
				YSLib::AsNode(term.get_allocator(), name, std::move(nd.Value));
		return
			{ValueNode::Container({std::move(nd)}, term.get_allocator()), name};
	}

	ValueNode::Container node_con;

	std::for_each(i, term.end(), [&](const TermNode& tm){
		insert_child(mapper ? mapper(tm) : nested_call(tm), node_con);
	});
	return
		{std::allocator_arg, term.get_allocator(), std::move(node_con), name};
}


//! \since build 685
namespace
{

//! \since build 852
//@{
template<typename _func, class _tTerm>
YB_ATTR_nodiscard TermNode
TransformForSeparatorCore(_func trans, _tTerm&& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	using namespace std::placeholders;
	using it_t = decltype(std::make_move_iterator(term.end()));
	auto res(AsTermNode(term.get_allocator(), MakeIndex(term.size()),
		yforward(term).Value));

	if(IsBranch(term))
	{
		// NOTE: Explicit type 'TermNode' is intended.
		res.Add(TermNode(AsIndexNode(term.get_allocator(), res.size(), pfx)));
		ystdex::split(std::make_move_iterator(term.begin()),
			std::make_move_iterator(term.end()), ystdex::bind1(
			HasValue<TokenValue>, std::ref(delim)), [&](it_t b, it_t e){
				const auto add([&](TermNode& node, it_t i){
					NPL::AddChildToTail(node, trans(Deref(i)));
				});

				// XXX: The implementation is depended on the fact that
				//	%TermNode is simply an alias of %ValueNode currently. It
				//	should be optimized.
				if(std::distance(b, e) == 1)
					// XXX: This guarantees a single element is converted with
					//	no redundant parentheses according to NPLA1 syntax,
					//	consistent to the trivial reduction for term with one
					//	subnode in %ReduceOnce.
					add(res, b);
				else
				{
					// NOTE: Explicit type 'TermNode' is intended.
					TermNode
						child(AsIndexNode(res.get_allocator(), res.size()));

					while(b != e)
						add(child, b++);
					res.Add(std::move(child));
				}
			});
	}
	return res;
}

//! \since build 852
template<class _tTerm>
YB_ATTR_nodiscard TermNode
TransformForSeparatorTmpl(_tTerm&& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	return TransformForSeparatorCore([&](_tTerm&& tm) ynothrow{
		return yforward(tm);
	}, yforward(term), pfx, delim);
}

//! \since build 852
template<class _tTerm>
YB_ATTR_nodiscard TermNode
TransformForSeparatorRecursiveTmpl(_tTerm&& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	return TransformForSeparatorCore([&](_tTerm&& tm){
		return TransformForSeparatorRecursiveTmpl(yforward(tm), pfx, delim);
	}, yforward(term), pfx, delim);
}
//@}


//! \since build 842
void
SetupNextTerm(ContextNode& ctx, TermNode& term)
{
	ContextState::Access(ctx).SetNextTermRef(term);
}

//! \since build 820
using EnvironmentGuard = ystdex::guard<EnvironmentSwitcher>;

// NOTE: See $2018-09 @ %Documentation::Workflow::Annual2018 for rationale of
//	implementation.
// XXX: First-class continuations are not implemented yet, due to lack of term
//	replacement mechanism in captured continuation. Kernel-style continuation
//	interception is also unsupported because no parantage is maintained in the
//	continuation object now.

#if YF_Impl_NPLA1_Enable_Thunked
#	if false
//! \since build 821
YB_ATTR_nodiscard inline
#		if true
PDefH(ReductionStatus, RelayTail, ContextNode& ctx, Reducer& cur)
	ImplRet(RelaySwitched(ctx, std::move(cur)))
#		else
// NOTE: For exposition only. This does not hold guarantee of TCO in unbounded
//	recursive cases.
PDefH(ReductionStatus, RelayTail, ContextNode&, const Reducer& cur)
	ImplRet(cur())
#		endif
#	endif

// NOTE: For continuation not capturable in the object language, it does not
//	have to be of a %Continuation.
//! \since build 842
ReductionStatus
RecoverNextTerm(TermNode& term, ContextNode& ctx)
{
	SetupNextTerm(ctx, term);
	return ctx.LastStatus;
}

//! \since build 841
void
PushActionsRange(EvaluationPasses::const_iterator first,
	EvaluationPasses::const_iterator last, TermNode& term, ContextNode& ctx)
{
	if(first != last)
	{
		const auto& f(first->second);

		++first;
		// NOTE: Retrying is recognized from the result since 1st administrative
		//	pass is reduced. The non-skipped administractive reductions are
		//	reduced in the tail context, otherwise they are dropped by no-op.
		if(ctx.LastStatus != ReductionStatus::Retrying)
			RelaySwitched(ctx, [=, &f, &term, &ctx]{
				PushActionsRange(first, last, term, ctx);

				const auto res(f(term, ctx));

				if(res != ReductionStatus::Partial)
					ctx.LastStatus
						= CombineSequenceReductionResult(ctx.LastStatus, res);
				return ctx.LastStatus;
			});
	}
	else
		ctx.LastStatus = ReductionStatus::Retained;
}

//! \since build 842
ReductionStatus
ExtractAdministratives(const EvaluationPasses& passes, TermNode& term,
	ContextNode& ctx)
{
	SetupNextTerm(ctx, term);
	// XXX: Be cautious with overflow risks in call of %ContextNode::ApplyTail
	//	when TCO is not enabled.
	PushActionsRange(passes.cbegin(), passes.cend(), term, ctx);
	return ReductionStatus::Partial;
}

//! \since build 841
ReductionStatus
ReduceForClosureResultInContext(TermNode& term, const ContextNode& ctx)
{
	return ReduceForClosureResult(term, ctx.LastStatus);
}

#	if YF_Impl_NPLA1_Enable_TCO
/*!
\brief 帧记录索引。
\note 顺序和的
\since build 842
*/
enum RecordFrameIndex : size_t
{
	ActiveCombiner,
	ActiveEnvironmentPtr,
	TemporaryEnvironmentPtr
};

/*!
\brief 帧记录。
\note 成员顺序和 RecordFrameIndex 中的项对应。
\since build 842
\sa RecordFrameIndex
*/
using FrameRecord = tuple<ContextHandler, shared_ptr<Environment>,
	shared_ptr<Environment>>;

/*!
\brief 帧记录列表。
\sa FrameRecord
\since build 827
*/
using FrameRecordList = yimpl(list)<FrameRecord>;

// NOTE: The concrete name is insignificant here.
//! \since build 829
yconstexpr const auto OperandName(yimpl("."));

//! \since build 818
class TCOAction final
{
public:
	lref<TermNode> Term;
	//! \since build 821
	mutable Reducer Next;
	//! \since build 820
	//@{
	bool ReduceNestedAsync = {};
	bool LiftCallResult = {};

private:
	//! \since build 827
	mutable list<ContextHandler> xgds{};

public:
	// See $2018-06 @ %Documentation::Workflow::Annual2018 for details.
	//! \since build 825
	mutable observer_ptr<const ContextHandler> LastFunction{};
	//! \since build 829
	mutable shared_ptr<Environment> TemporaryPtr{};
	mutable EnvironmentGuard EnvGuard;
	//! \since build 825
	mutable FrameRecordList RecordList;
	bool ReduceCombined = {};
	//@}
#		if YF_Impl_NPLA1_Enable_WeakExternalRoots
	//! \since build 827
	mutable set<weak_ptr<Environment>, owner_less<weak_ptr<Environment>>>
		WeakEnvs;
#		endif
 
	//! \since build 819
	TCOAction(ContextNode& ctx, TermNode& term, bool lift)
		: Term(term), Next(ctx.Switch()), LiftCallResult(lift),
		xgds(ctx.get_allocator()), EnvGuard(ctx),
		RecordList(ctx.get_allocator())
#		if YF_Impl_NPLA1_Enable_WeakExternalRoots
		, WeakEnvs(ctx.get_allocator())
#		endif
	{}
	// XXX: Not used, but provided for well-formness.
	//! \since build 819
	TCOAction(const TCOAction& a)
		// XXX: Some members are moved. This is only safe when newly constructed
		//	object always live longer than the older one.
		: Term(a.Term), Next(a.Next), ReduceNestedAsync(a.ReduceNestedAsync),
		LiftCallResult(a.LiftCallResult), xgds(std::move(a.xgds)),
		EnvGuard(std::move(a.EnvGuard)), ReduceCombined(a.ReduceCombined)
#		if YF_Impl_NPLA1_Enable_WeakExternalRoots
		, WeakEnvs(std::move(a.WeakEnvs))
#		endif
	{}
	DefDeMoveCtor(TCOAction)

	DefDeMoveAssignment(TCOAction)

	ReductionStatus
	operator()() const
	{
		auto& ctx(EnvGuard.func.Context.get());

		RelaySwitched(ctx, std::move(Next));

		// NOTE: Lifting is optional. See also $2018-02
		//	@ %Documentation::Workflow::Annual2018.
		const auto res(LiftCallResult
			? ReduceForClosureResultInContext(Term, ctx) : ctx.LastStatus);

		// NOTE: The order here is significant. The environment in the guards
		//	should be hold until lifting is completed.
		while(!RecordList.empty())
			RecordList.pop_front();
		{
			const auto egd(std::move(EnvGuard));
		}
		while(!xgds.empty())
			xgds.pop_back();
		if(ReduceCombined)
			RegularizeTerm(Term, res);
		return ReduceNestedAsync ? ReductionStatus::Clean : res;
	}

	//! \since build 825
	YB_ATTR_nodiscard lref<const ContextHandler>
	AttachFunction(ContextHandler&& h)
	{
		// NOTE: This scans guards to hold function prvalues, which are safe to
		//	be removed as per the equivalence (hopefully, of beta reduction)
		//	defined by %operator== of the handler, no new instance is to be
		//	added.
		ystdex::erase_all(xgds, h);
		xgds.emplace_back();
		// NOTE: Strong exception guarantee is kept here.
		swap(xgds.back(), h);
		return ystdex::as_const(xgds.back());
	}

	//! \since build 842
	void
	CleanupOrphanTemporary()
	{
		if(!RecordList.empty())
		{
			auto& frame(RecordList.front());
			auto& p_temp_env(get<TemporaryEnvironmentPtr>(frame));

			if(p_temp_env.use_count() == 1)
				p_temp_env = {};
		//	get<ActiveCombiner>(frame) = ContextHandler();
		}
	}

	//! \since build 825
	YB_ATTR_nodiscard ContextHandler
	MoveFunction()
	{
		ContextHandler res;
		const auto i(std::find_if(xgds.rbegin(), xgds.rend(),
			[this](const ContextHandler& h) ynothrow{
			return NPL::make_observer(&h) == LastFunction;
		}));

		if(i != xgds.rend())
		{
			res = std::move(*i);
			xgds.erase(std::next(i).base());
		}
		LastFunction = {};
		return res;
	}

	//! \since build 844
	void
	UpdateTemporaryPtr(TermNode& term, ContextNode& ctx)
	{
		// XXX: Old %TemporaryPtr value would be overwritten. It should be saved
		//	before the next call, or the operand saved held by %TemporaryPtr
		//	would be lost.
		YAssert(!TemporaryPtr,
			"The temporary stored for TCO context is overwritten.");
		// NOTE: An environment object is introduced to make it collectable.
		Deref(TemporaryPtr = AllocateEnvironment(term, ctx))[
			OperandName].SetContent(std::move(term));
	}
};

//! \since build 830
YB_ATTR_nodiscard YB_PURE inline
	PDefH(TCOAction*, AccessTCOAction, ContextNode& ctx)
	ImplRet(ctx.Current.target<TCOAction>())
// NOTE: There is no need to check term like 'if(&p->Term.get() == &term)'. It
//	should be same to saved enclosing term currently.

//! \since build 840
template<typename _func>
void
SetupTailAction(ContextNode& ctx, _func&& act)
{
	// XXX: Avoid direct use of %ContextNode::SetupTail even though it is safe
	//	because %Current is already saved in %act?
	ctx.SetupTail(std::move(act));
}

//! \since build 840
YB_ATTR_nodiscard TCOAction&
EnsureTCOAction(ContextNode& ctx, TermNode& term)
{
	auto p(AccessTCOAction(ctx));

	if(!p)
	{
		SetupTailAction(ctx, TCOAction(ctx, term, {}));
		p = AccessTCOAction(ctx);
	}
	return Deref(p);
}

//! \since build 840
YB_ATTR_nodiscard inline PDefH(TCOAction&, RefTCOAction, ContextNode& ctx)
	// NOTE: The TCO action should have been created by a previous call of
	//	%CombinerReturnThunk.
	ImplRet(Deref(AccessTCOAction(ctx)))
#	endif

//! \since build 810
ReductionStatus
ReduceChildrenOrderedAsync(TNIter first, TNIter last, ContextNode& ctx)
{
	if(first != last)
	{
		auto& subterm(*first);

		++first;
		return RelaySwitched(ctx, [&, first, last]{
			RelaySwitched(ctx, std::bind(ReduceChildrenOrderedAsync, first,
				last, std::ref(ctx)));
			return ReduceChecked(subterm, ctx);
		});
	}
	return ReductionStatus::Clean;
}
#else
//! \since build 841
ReductionStatus
ReduceCheckedSync(TermNode& term, ContextNode& ctx)
{
	return CheckedReduceWith(Reduce, term, ctx);
}
#endif

//! \since build 851
ReductionStatus
DoAdministratives(const EvaluationPasses& passes, TermNode& term,
	ContextNode& ctx)
{
#if YF_Impl_NPLA1_Enable_Thunked
	return ExtractAdministratives(passes, term, ctx);
#else
	return passes(term, ctx);
#endif
}

//! \since build 821
template<typename _fCurrent>
ReductionStatus
ReduceSubsequent(TermNode& term, ContextNode& ctx, _fCurrent&& next)
{
#if YF_Impl_NPLA1_Enable_Thunked
	SetupNextTerm(ctx, term);
	return RelayNext(ctx, Continuation(ReduceChecked, ctx),
		ComposeActions(ctx, yforward(next), ctx.Switch()));
#else
	// NOTE: This does not support PTC.
	ReduceCheckedSync(term, ctx);
	return next();
#endif
}

//! \since build 824
ReductionStatus
ReduceAgainLifted(TermNode& term, ContextNode& ctx, TermNode& tm)
{
	LiftTerm(term, tm);
	return ReduceAgain(term, ctx);
}

#if YF_Impl_NPLA1_Enable_TailRewriting
//! \since build 823
ReductionStatus
ReduceSequenceOrderedAsync(TermNode& term, ContextNode& ctx, TNIter i)
{
	YAssert(i != term.end(), "Invalid iterator found for sequence reduction.");
	return std::next(i) == term.end() ? ReduceAgainLifted(term, ctx, *i)
		: ReduceSubsequent(*i, ctx, [&, i]{
		auto& act(EnsureTCOAction(ctx, term));

		act.UpdateTemporaryPtr(*i, ctx);
		// NOTE: The sequence combiner is permanent and not stateful.
		act.RecordList.emplace_front(ContextHandler(),
			shared_ptr<Environment>(), std::move(act.TemporaryPtr));
		return ReduceSequenceOrderedAsync(term, ctx, term.erase(i));
	});
}
#endif


//! \since build 808
YB_ATTR_nodiscard YB_PURE bool
ExtractBool(TermNode& term, bool is_and) ynothrow
{
	return ystdex::value_or(AccessTermPtr<bool>(ReferenceTerm(term)), is_and)
		== is_and;
}

//! \since build 753
ReductionStatus
AndOr(TermNode& term, ContextNode& ctx, bool is_and)
{
	Forms::Retain(term);

	auto i(term.begin());

	if(++i != term.end())
		return std::next(i) == term.end() ? ReduceAgainLifted(term, ctx, *i)
			: ReduceSubsequent(*i, ctx, [&, i, is_and]{
			if(ExtractBool(*i, is_and))
				term.Remove(i);
			else
			{
				term.Value = !is_and;
				return ReductionStatus::Clean;
			}
			return ReduceAgain(term, ctx);
		});
	term.Value = is_and;
	return ReductionStatus::Clean;
}

//! \since build 804
//@{
template<typename _fComp, typename _func>
void
EqualTerm(TermNode& term, _fComp f, _func g)
{
	Forms::RetainN(term, 2);

	auto i(term.begin());
	const auto& x(Deref(++i));

	term.Value = f(g(x), g(ystdex::as_const(Deref(++i))));
}

template<typename _func>
void
EqualTermValue(TermNode& term, _func f)
{
	EqualTerm(term, f, [](const TermNode& node) -> const ValueObject&{
		return ReferenceTerm(node).Value;
	});
}

template<typename _func>
void
EqualTermReference(TermNode& term, _func f)
{
	EqualTerm(term, f, [](const TermNode& node) -> const TermNode&{
		return ReferenceTerm(node);
	});
}
//@}


//! \since build 782
class RecursiveThunk final
{
private:
	//! \since build 784
	//@{
	using shared_ptr_t = shared_ptr<ContextHandler>;
	unordered_map<string, shared_ptr_t> store{};
	//! \since build 847
	shared_ptr_t p_default;
	//@}

public:
	//! \since build 840
	lref<Environment> Record;
	//! \since build 840
	lref<const TermNode> Term;

	//! \since build 787
	//@{
	RecursiveThunk(Environment& env, const TermNode& t)
		: p_default(YSLib::allocate_shared<ContextHandler>(t.get_allocator(),
		ThrowInvalidCyclicReference)), Record(env), Term(t)
	{
		Fix(Record, Term, p_default);
	}
	//! \since build 841
	DefDeMoveCtor(RecursiveThunk)

	//! \since build 841
	DefDeMoveAssignment(RecursiveThunk)

private:
	void
	Fix(Environment& env, const TermNode& t, const shared_ptr_t& p_d)
	{
		if(IsBranch(t))
			for(const auto& tm : t)
				Fix(env, tm, p_d);
		else if(const auto p = AccessPtr<TokenValue>(t))
		{
			const auto& n(*p);

			// XXX: This is served as addtional static environment.
			Forms::CheckParameterLeafToken(n, [&]{
				// XXX: The symbol can be rebound.
				env[n].SetContent(TermNode::Container(t.get_allocator()),
					ValueObject(any_ops::use_holder,
					in_place_type<HolderFromPointer<weak_ptr<ContextHandler>>>,
					store[n] = p_d));
			});
		}
	}

	void
	Restore(Environment& env, const TermNode& t)
	{
		if(IsBranch(t))
			for(const auto& tm : t)
				Restore(env, tm);
		else if(const auto p = AccessPtr<TokenValue>(t))
			FilterExceptions([&]{
				const auto& n(*p);
				auto& v(env[n].Value);

				if(v.type() == ystdex::type_id<ContextHandler>())
				{
					// XXX: The element should exist.
					auto& p_strong(store.at(n));

					Deref(p_strong) = std::move(v.GetObject<ContextHandler>());
					v = ValueObject(any_ops::use_holder,
						in_place_type<HolderFromPointer<shared_ptr_t>>,
						std::move(p_strong));
				}
			});
	}
	//@}

	//! \since build 780
	YB_NORETURN static ReductionStatus
	ThrowInvalidCyclicReference(TermNode&, ContextNode&)
	{
		throw InvalidReference("Invalid cyclic reference found.");
	}

public:
	PDefH(void, Commit, )
		ImplExpr(Restore(Record, Term))
};


//! \since build 841
template<typename _func>
void
DoDefine(TermNode& term, _func f)
{
	Forms::Retain(term);
	if(term.size() > 2)
	{
		RemoveHead(term);

		auto formals(std::move(Deref(term.begin())));

		RemoveHead(term);
		LiftToSelf(formals);
		f(formals);
	}
	else
		throw InvalidSyntax("Insufficient terms in definition.");
}

//! \since build 841
ReductionStatus
DoDefineReturn(TermNode& term) ynothrow
{
	term.Value = ValueToken::Unspecified;
	return ReductionStatus::Clean;
}


//! \since build 799
YB_NONNULL(2) void
CheckVauSymbol(const TokenValue& s, const char* target)
{
	if(s != "#ignore" && !IsNPLASymbol(s))
		throw InvalidSyntax(ystdex::sfmt("Token '%s' is not a symbol or"
			" '#ignore' expected for %s.", s.data(), target));
}

//! \since build 799
YB_NORETURN YB_NONNULL(2) void
ThrowInvalidSymbolType(const TermNode& term, const char* n)
{
	throw InvalidSyntax(ystdex::sfmt("Invalid %s type '%s' found.", n,
		term.Value.type().name()));
}

//! \since build 781
YB_ATTR_nodiscard YB_PURE string
CheckEnvFormal(const TermNode& eterm)
{
	const auto& term(ReferenceTerm(eterm));

	if(const auto p = TermToNamePtr(term))
	{
		CheckVauSymbol(*p, "environment formal parameter");
		return *p;
	}
	else
		ThrowInvalidSymbolType(term, "environment formal parameter");
	return {};
}

//! \since build 822
//@{
#if YF_Impl_NPLA1_Enable_TCO
//! \since build 825
bool
DeduplicateBindings(Environment& dst, const Environment& src)
{
	// NOTE: Make the frames reused as possible.
	// XXX: Assuming objects are all pinned.
	return Environment::Deduplicate(dst.GetMapRef(), src.GetMapRef());
}

//! \since build 827
struct RecordCompressor final
{
	using Tracer = function<bool(Environment&, Environment&)>;
	using RecordInfo = map<lref<Environment>, size_t, ystdex::get_less<>>;
	using ReferenceSet = set<lref<Environment>, ystdex::get_less<>>;

	// XXX: The order of destruction is unspecified.
	vector<shared_ptr<Environment>> Collected;
	lref<Environment> Root;
	ReferenceSet Reachable, NewlyReachable, WeakRoots;
	RecordInfo Universe;

	RecordCompressor(Environment& root)
		: RecordCompressor(root, root.Bindings.get_allocator())
	{}
	//! \since build 851
	RecordCompressor(Environment& root, Environment::allocator_type a)
		: Collected(a), Root(root), Reachable({root}, a), NewlyReachable(a),
		WeakRoots(a), Universe(a)
	{}

	// NOTE: All checks rely on recursive calls do not support PTC currently.
	void
	Add(Environment& e)
	{
		AddForRoot(e);
		Universe.emplace(e, CountReferences(e));
		// XXX: There shall be exact one (strong or weak) reference share %e
		//	externally.
		WeakRoots.insert(e);
	}

	void
	AddForRoot(Environment& e)
	{
		Traverse(e, e.Parent, [this](Environment& dst, Environment&) -> bool{
			return Universe.emplace(dst, CountReferences(dst)).second;
		});
	}

#	if YF_Impl_NPLA1_Enable_WeakExternalRoots
	void
	AddWeakRoot(const weak_ptr<Environment>& p)
	{
		if(const auto p_env = p.lock())
			Add(*p_env);
	}
#	endif

	void
	Compress()
	{
		// NOTE: This is need to keep the root as external reference.
		const auto p_root(Root.get().shared_from_this());

		// NOTE: Trace.
		for(auto& pr : Universe)
		{
			auto& e(pr.first.get());

			Traverse(e, e.Parent,
				[this](Environment& dst, Environment&) -> bool{
				auto& count(Universe.at(dst));

				YAssert(count > 0, "Invalid count found in trace record.");
				--count;
				return {};
			});
		}
		for(const auto& e : WeakRoots)
		{
			auto& count(Universe.at(e));

			if(count > 0)
				--count;
		}
		for(auto i(Universe.cbegin()); i != Universe.cend(); )
			if(i->second > 0)
			{
				NewlyReachable.insert(i->first);
				i = Universe.erase(i);
			}
			else
				++i;
		while(!NewlyReachable.empty())
		{
			ReferenceSet rs;

			for(const auto& e : NewlyReachable)
				Traverse(e, e.get().Parent,
					[&](Environment& dst, Environment&) ynothrow -> bool{
					rs.insert(dst);
					Universe.erase(dst);
					return {};
				});
			Reachable.insert(std::make_move_iterator(NewlyReachable.begin()),
				std::make_move_iterator(NewlyReachable.end()));
			for(auto i(rs.cbegin()); i != rs.cend(); )
				if(ystdex::exists(Reachable, *i))
					i = rs.erase(i);
				else
					++i;
			NewlyReachable = std::move(rs);
		}
		// NOTE: Move to collect.
		for(const auto& pr : Universe)
			// XXX: The envionment would be finally collected after the last
			//	reference is destroyed.
			Collected.push_back(pr.first.get().shared_from_this());

		// TODO: Fully support of PTC by direct DFS traverse.
		ReferenceSet accessed;

		ystdex::retry_on_cond(ystdex::id<>(), [&]() -> bool{
			size_t collected(0);

			Traverse(Root, Root.get().Parent,
				[&](Environment& dst, Environment& src) -> bool{
				if(accessed.insert(src).second)
				{
					if(!ystdex::exists(Universe, ystdex::ref(dst)))
						return true;
					src.Parent = dst.Parent;
					++collected;
				}
				return {};
			});
			return collected != 0;
		});
	}

	static size_t
	CountReferences(const Environment& e) ynothrowv
	{
		const auto acnt(e.GetAnchorCount());

		YAssert(acnt > 0, "Zero anchor count found for environment.");
		// TODO: Use C++17 %weak_from_this to get more efficient
		//	implementation?
		return CountStrong(e.shared_from_this()) + size_t(acnt) - 2;
	}

	static size_t
	CountStrong(const shared_ptr<const Environment>& p) ynothrowv
	{
		const long scnt(p.use_count());

		YAssert(scnt > 0, "Zero shared count found for environment.");
		return size_t(scnt);
	}

	static void
	Traverse(Environment& e, const ValueObject& parent, Tracer trace)
	{
		const auto& tp(parent.type());

		if(tp == ystdex::type_id<EnvironmentList>())
		{
			for(const auto& vo : parent.GetObject<EnvironmentList>())
				Traverse(e, vo, trace);
		}
		else if(tp == ystdex::type_id<EnvironmentReference>())
		{
			// XXX: The shared pointer should not be locked to ensure it neutral
			//	to nested call levels.
			if(const auto p
				= parent.GetObject<EnvironmentReference>().Lock().get())
				if(trace(*p, e))
					Traverse(*p, p->Parent, trace);
		}
		else if(tp == ystdex::type_id<shared_ptr<Environment>>())
		{
			// NOTE: The reference count should not be effected here.
			if(const auto p = parent.GetObject<shared_ptr<Environment>>().get())
				if(trace(*p, e))
					Traverse(*p, p->Parent, trace);
		}
	}
};

// NOTE: See $2018-06 @ %Documentation::Workflow::Annual2018 for details.
//! \since build 842
//@{
void
UpdateFusedAction(TCOAction& fused_act, EnvironmentGuard& gd)
{
	fused_act.EnvGuard = std::move(gd);
}

void
CompressTCOFramesForSavedEnvironment(ContextNode& ctx, TCOAction& act,
	Environment& saved)
{
	auto& record_list(act.RecordList);
	auto i(record_list.cbegin());
	const auto erase_frame([&]{
		i = record_list.erase(i);
	});
	RecordCompressor compressor(ctx.GetRecordRef());

	compressor.AddForRoot(ctx.GetRecordRef());
	compressor.Add(saved);
	// XXX: This is usually not empty.
	if(act.TemporaryPtr)
		compressor.Add(*act.TemporaryPtr);
#	if YF_Impl_NPLA1_Enable_WeakExternalRoots
	for(const auto& p_weak : act.WeakEnvs)
		compressor.AddWeakRoot(p_weak);
#	endif
	// NOTE: This does not support PTC currently.
	ystdex::retry_on_cond(ystdex::id<>(), [&]() -> bool{
		const auto orig_size(record_list.size());

		// NOTE: The following code searches the frames to be removed, in the
		//	order from new to old. After merging, the guard slot %EnvGuard owns
		//	the resources of the expression (and its enclosed subexpressions)
		//	being TCO'd.
		i = record_list.cbegin();
		while(i != record_list.cend())
		{
			auto& p_frame_env_ref(get<ActiveEnvironmentPtr>(
				*ystdex::cast_mutable(record_list, i)));

			if(p_frame_env_ref.use_count() != 1)
			{
				const auto& p_frame_operand(get<TemporaryEnvironmentPtr>(*i));

				if(p_frame_operand.use_count() != 1)
					// NOTE: The whole frame is to be removed. The function
					//	prvalue is expected to live only in the subexpression
					//	evaluation. This has equivalent effects of evlis tail
					//	recursion.
					erase_frame();
				else
				{
					// NOTE: The frame is alive to ensure proper lifetime of
					//	the operand, but the frame environment should be
					//	compressed away.
					p_frame_env_ref.reset();
					++i;
				}
			}
			else
			{
				auto& frame_env(Deref(p_frame_env_ref));

				if(frame_env.IsOrphan())
					erase_frame();
				else
				{
					compressor.Add(frame_env);
					// XXX: This does not fit for the situation that the same
					//	parent is referenced in multiple inactive frames.
					//	However, they should be already compressed.
					if(frame_env.GetAnchorCount() == 2
						&& [](Environment& dst, const Environment& src) -> bool{
						if(const auto p_env
							= AccessPtr<EnvironmentReference>(dst.Parent))
							return p_env->Lock().get() == &src
								&& DeduplicateBindings(dst, src);
						return {};
					}(frame_env, saved))
					{
						saved.Parent = std::move(frame_env.Parent);
						erase_frame();
					}
					else
						++i;
				}
			}
		}
		return record_list.size() != orig_size;
	});
	act.CleanupOrphanTemporary();
	compressor.Compress();
}
//@}
#elif YF_Impl_NPLA1_Enable_Thunked
/*!
\since build 851
\warning 非虚析构。
\warning 复制构造转移成员。
*/
//@{
struct ThunkedActionSaver
{
	// NOTE: Lambda is not used to avoid unspecified destruction order of
	//	captured component and better performance (compared to the case of
	//	%pair used to keep the order and %shared_ptr to hold the guard).
	mutable TermNode::Container OperandList;
	mutable ValueObject OperandValue;

	ThunkedActionSaver(TermNode& term)
		: OperandList(std::move(term.GetContainerRef())),
		OperandValue(std::move(term.Value))
	{}
	// XXX: The copy constructor is not used, but provided for well-formness.
	ThunkedActionSaver(const ThunkedActionSaver& a)
		// XXX: Several members are moved in the copy constructors. It is only
		//	safe when newly constructed object always live longer than the older
		//	one.
		: OperandList(std::move(a.OperandList)),
		OperandValue(std::move(a.OperandValue))
	{}
	DefDeMoveCtor(ThunkedActionSaver)

	DefDeMoveAssignment(ThunkedActionSaver)
};

struct EvalAction
{
	// NOTE: The destruction order of following captured component is
	//	significant. See %ThunkedActionSaver.
	mutable Reducer Next;
	mutable EnvironmentGuard Guard;

	EvalAction(ContextNode& ctx, EnvironmentGuard& egd)
		: Next(ctx.Switch()), Guard(std::move(egd))
	{}
	// NOTE: See comment to the copy constructor of class %ThunkedActionSaver.
	EvalAction(const EvalAction& a)
		: Next(std::move(a.Next)), Guard(std::move(Guard))
	{}
	DefDeMoveCtor(EvalAction)

	DefDeMoveAssignment(EvalAction)

	ReductionStatus
	operator()() const
	{
		{
			const auto egd(std::move(Guard));
		}
		return Next();
	}
};


template<class _tAction>
ReductionStatus
RelayForAction(_tAction&& a, ContextNode& ctx, TermNode& term, bool no_lift)
{
	// XXX: Term reused. Call of %RecoverNextTerm is not needed as the next
	//	term is guaranteed not changed.
	return RelayNext(ctx, no_lift ? Continuation(ReduceChecked, ctx)
		: Continuation([&]{
		return RelaySwitchedUnchecked(ctx, ComposeActions(ctx, Continuation(
			ReduceChecked, ctx), Continuation([&](TermNode&, ContextNode& c){
				return ReduceForClosureResultInContext(term, c);
		}, ctx)));
	}, ctx), std::move(a));
}
//@}
#endif

//! \since build 851
//@{
void
SetExpressionToReduce(TermNode& term, TermNode& expr, bool move)
{
	if(move)
		LiftTerm(term, expr);
	else
		term.SetContent(expr);
}

/*!
\pre 对 TCO 实现，存在 TCOAction 当前动作且操作数已被保存为 TemporaryPtr 。
\note 对 TCO 实现利用 TCOAction 以尾上下文进行规约。
\note 第三参数指定是否通过转移构造而不保留原项。
\note 第四参数用于替换第二参数，可能是前者的子项。
\note 第六参数指定是否避免使用 ReduceForClosureResult 保证规约后提升结果。
*/
//@{
//! \brief 用于直接求值的异步调用规约。
ReductionStatus
RelayForEval(ContextNode& ctx, TermNode& term, bool move, TermNode& expr,
	EnvironmentGuard&& gd, bool no_lift)
{
	const auto set_expr([&, move]{
		SetExpressionToReduce(term, expr, move);
	});

	// XXX: For thunked code, there should be a single continuation before being
	//	captured and it is not capturable here. No %SetupNextTerm or
	//	%RecoverNextTerm needs to be called.
#if YF_Impl_NPLA1_Enable_TCO
	auto& act(RefTCOAction(ctx));

	[&]{
		// NOTE: The tempoaray function, the environment and the
		//	temporary operand are saved in the frame record list as a new
		//	entry.
		const auto add_rec([&](shared_ptr<Environment>&& p_env){
			act.RecordList.emplace_front(act.MoveFunction(),
				std::move(p_env), std::move(act.TemporaryPtr));
		});

		// NOTE: If there is no environment set in %act.EnvGuard yet, there is
		//	ideally no need to save the components to the frame record list
		//	for recursive calls, except that %TCOAction::UpdateTemporaryPtr is
		//	called elsewhere. In such case, each operation making potentionally
		//	overwriting of %act.LastFunction and %act.TemporaryPtr will always
		//	get into this call and that time %act.EnvGuard should be set. This
		//	is not true when %TCOAction::UpdateTemporaryPtr is actually called
		//	in %ReduceSequenceOrderedAsync.
		if(act.EnvGuard.func.SavedPtr)
		{
			// NOTE: Operand saving is performed whether the frame compression
			//	is needed, once there is a saved environment set.
			if(auto& p_saved = gd.func.SavedPtr)
			{
				CompressTCOFramesForSavedEnvironment(ctx, act, *p_saved);
				add_rec(std::move(p_saved));
				return;
			}
			// XXX: Normally this should not occur, but this is allowed by the
			//	interface (for an object %EnvironmentSwitcher initialized without
			//	an environment).
		}
		else
			UpdateFusedAction(act, gd);
		add_rec({});
	}();
	set_expr();
	if(!no_lift)
	{
		if(act.LiftCallResult)
			ReduceForClosureResultInContext(term, ctx);
		else
			act.LiftCallResult = true;
	}
	return RelaySwitchedUnchecked(ctx, Continuation(ReduceChecked, ctx));
#elif YF_Impl_NPLA1_Enable_Thunked
	set_expr();
	return RelayForAction(EvalAction(ctx, gd), ctx, term, no_lift);
#else
	yunused(gd);
	// NOTE: This does not support PTC.
	set_expr();

	const auto res(ReduceCheckedSync(term, ctx));

	return no_lift ? res : ReduceForClosureResult(term, res);
#endif
}

/*!
\brief 用于函数调用（ β 规约）的异步调用规约。
\pre 若为非 TCO 的函数调用，第二参数的子项已绑定变量。
\pre 临时函数不存在或已通过 CombinerReturnThunk 调用被适当保存。
\note 第二参数在必要的保存操作后被设置为第四参数覆盖，其时机未指定。
*/
ReductionStatus
RelayForCall(ContextNode& ctx, TermNode& term, bool move, TermNode& closure,
	EnvironmentGuard&& gd, bool no_lift)
{
	// NOTE: The operand in the parameter %term is to be saved before setting
	//	the term content to the closure (so the closure should only be
	//	called after saving the term) except those already saved in
	//	%TCOAction::TemporaryPtr for the TCO implementation before this call. It
	//	is then safe to be cleared and replaced by the next term being
	//	evaluated.
#if YF_Impl_NPLA1_Enable_Thunked && !YF_Impl_NPLA1_Enable_TCO
	struct CallAction : ThunkedActionSaver, EvalAction
	{
		CallAction(TermNode& term, ContextNode& ctx, EnvironmentGuard& egd)
			: ThunkedActionSaver(term), EvalAction(ctx, egd)
		{}
		DefDeCopyMoveCtorAssignment(CallAction)
	};
	CallAction a(term, ctx, gd);

	SetExpressionToReduce(term, closure, move);
	return RelayForAction(a, ctx, term, no_lift);
#else
#	if !YF_Impl_NPLA1_Enable_TCO
	// NOTE: It is necessary to keep the operand alive for function calls.
	const auto operand_l(std::move(term.GetContainerRef()));
	const auto operand_v(std::move(term.Value));
#	endif

	return RelayForEval(ctx, term, move, closure, std::move(gd), no_lift);
#endif
}
//@}
//@}

//! \since build 835
ReductionStatus
EvalImplUnchecked(TermNode& term, ContextNode& ctx, bool no_lift)
{
	auto i(std::next(term.begin()));
	const auto term_args([](TermNode& expr) -> pair<bool, lref<TermNode>>{
		if(const auto p = AccessPtr<const TermReference>(expr))
			return {{}, p->get()};
		return {true, expr};
	}(Deref(i)));
	auto p_env(ResolveEnvironment(Deref(++i)).first);

	// NOTE: This is necessary to cleanup reference count (if any).
	i->Value = {};
	// XXX: Support more environment types?
	return RelayForEval(ctx, term, term_args.first, term_args.second,
		EnvironmentGuard(ctx, ctx.SwitchEnvironment(std::move(p_env))),
		no_lift);
}

ReductionStatus
EvalImpl(TermNode& term, ContextNode& ctx, bool no_lift)
{
	Forms::RetainN(term, 2);
	return EvalImplUnchecked(term, ctx, no_lift);
}
//@}

//! \since build 835
ReductionStatus
EvalStringImpl(TermNode& term, ContextNode& ctx, bool no_lift)
{
	Forms::RetainN(term, 2);

	auto& expr(Deref(std::next(term.begin())));
	auto unit(SContext::Analyze(Session(AccessTerm<const string>(expr)),
		term.get_allocator()));

	unit.SwapContainer(expr);
	return EvalImplUnchecked(term, ctx, no_lift);
}


//! \since build 851
inline PDefH(shared_ptr<TermNode>, ShareMoveTerm, TermNode& term)
	ImplRet(share_move(term.get_allocator(), term))
//! \since build 851
inline PDefH(shared_ptr<TermNode>, ShareMoveTerm, TermNode&& term)
	ImplRet(share_move(term.get_allocator(), term))


//! \since build 767
class VauHandler final : private ystdex::equality_comparable<VauHandler>
{
private:
	/*!
	\brief 动态上下文名称。
	\since build 769
	*/
	string eformal{};
	/*!
	\brief 形式参数对象。
	\invariant \c bool(p_formals) 。
	\since build 771
	*/
	shared_ptr<TermNode> p_formals;
	/*!
	\note 共享所有权用于检查循环引用。
	\since build 823
	*/
	//@{
	//! \brief 捕获静态环境作为父环境的引用，包含引入抽象时的静态环境。
	EnvironmentReference parent;
	bool owning_static;
	//! \brief 可选保持所有权的静态环境指针或父环境锚对象指针。
	shared_ptr<const void> p_static;
	//@}
	//! \brief 闭包求值构造。
	shared_ptr<TermNode> p_closure;
	/*!
	\brief 调用指针。
	\since build 847
	*/
	ReductionStatus(VauHandler::*p_call)(TermNode&, ContextNode&) const;

public:
	//! \brief 返回时不提升项以允许返回引用。
	bool NoLifting = {};

	/*!
	\pre 形式参数对象指针非空。
	\since build 842
	*/
	VauHandler(string&& ename, shared_ptr<TermNode>&& p_fm,
		shared_ptr<Environment>&& p_env, bool owning, TermNode& term, bool nl)
		: eformal(std::move(ename)), p_formals((LiftToSelf(Deref(p_fm)),
		CheckParameterTree(*p_fm), std::move(p_fm))), parent(p_env),
		// XXX: Optimize with region inference?
		owning_static(owning), p_static(owning ? std::move(p_env) : nullptr),
		p_closure(ShareMoveTerm(ystdex::exchange(term, TermNode(
		std::allocator_arg, term.get_allocator(), NoContainer,
		term.GetName())))), p_call(eformal.empty() ? &VauHandler::CallStatic
		: &VauHandler::CallDynamic), NoLifting(nl)
	{}

	//! \since build 824
	friend bool
	operator==(const VauHandler& x, const VauHandler& y)
	{
		return x.eformal == y.eformal && x.p_formals == y.p_formals
			&& x.parent == y.parent && x.owning_static == y.owning_static
			&& x.p_static == y.p_static && x.NoLifting == y.NoLifting;
	}

	//! \since build 772
	ReductionStatus
	operator()(TermNode& term, ContextNode& ctx) const
	{
		if(IsBranch(term))
			return (this->*p_call)(term, ctx);
		else
			throw LoggedEvent("Invalid composition found.", Alert);
	}

	//! \since build 799
	static void
	CheckParameterTree(const TermNode& term)
	{
		for(const auto& child : term)
			CheckParameterTree(child);
		if(term.Value)
		{
			if(const auto p = TermToNamePtr(term))
				CheckVauSymbol(*p, "parameter in a parameter tree");
			else
				ThrowInvalidSymbolType(term, "parameter tree node");
		}
	}

private:
	//! \since build 847
	//@{
	void
	BindEnvironment(ContextNode& ctx, ValueObject&& vo) const
	{
		YAssert(!eformal.empty(), "Empty dynamic environment name found.");
		// NOTE: Bound dynamic environment.
 		ctx.GetRecordRef().AddValue(eformal, std::move(vo));
		// NOTE: The dynamic environment is either out of TCO action or
		//	referenced by other environments already in TCO action, so there
		//	is no need to treat as root.
	}

	ReductionStatus
	CallDynamic(TermNode& term, ContextNode& ctx) const
	{
		// NOTE: Evaluation in the local context: using the activation
		//	record frame with outer scope bindings.
		auto wenv(ctx.WeakenRecord());
		// XXX: Reuse of frame cannot be done here unless it can be proved all
		//	bindings would behave as in the old environment, which is too
		//	expensive for direct execution of programs with first-class
		//	environments.
		EnvironmentGuard gd(ctx, NPL::SwitchToFreshEnvironment(ctx));

		BindEnvironment(ctx, std::move(wenv));
		// XXX: Referencing escaped variables (now only parameters need to be
		//	cared) form the context would cause undefined behavior (e.g.
		//	returning a reference to automatic object in the host language). The
		//	lifting of call result is enabled to prevent this, unless
		//	%NoLifting is %true. See also %Forms::BindParameter.
		return DoCall(term, ctx, gd);
	}

	ReductionStatus
	CallStatic(TermNode& term, ContextNode& ctx) const
	{
		// NOTE: See above.
		EnvironmentGuard gd(ctx, NPL::SwitchToFreshEnvironment(ctx));

		return DoCall(term, ctx, gd);
	}

	ReductionStatus
	DoCall(TermNode& term, ContextNode& ctx, EnvironmentGuard& gd) const
	{
		// NOTE: Since first term is expected to be saved (e.g. by
		//	%ReduceCombined), it is safe to be removed directly.
		RemoveHead(term);
		// NOTE: Forming beta-reducible terms using parameter binding, to
		//	substitute them as arguments for later closure reduction.
		// XXX: Do not lift terms if provable to be safe?
		Forms::BindParameter(ctx, Deref(p_formals), [&]() -> TermNode&{
#if YF_Impl_NPLA1_Enable_TCO
			auto& act(RefTCOAction(ctx));

			// NOTE: The arguments in the operand held by the term have to be
			//	saved for extension of lifetime of bound reference parameters.
			act.UpdateTemporaryPtr(term, ctx);
			return Deref(act.TemporaryPtr)[OperandName];
#else
			return term;
#endif
		}());
#ifndef NDEBUG
		ctx.Trace.Log(Debug, [&]{
			return sfmt("Function called, with %ld shared term(s), %ld"
				" %s shared static environment(s), %zu parameter(s).",
				p_closure.use_count(), parent.GetPtr().use_count(),
				owning_static ? "owning" : "nonowning", p_formals->size());
		});
#endif
		// NOTE: Static environment is bound as base of local environment by
		//	setting parent environment pointer.
		ctx.GetRecordRef().Parent = parent;
		// XXX: Implement accurate lifetime analysis depending on
		//	'p_closure.unique()'?
		return RelayForCall(ctx, term, {}, *p_closure, std::move(gd),
			NoLifting);
	}
	//@}
};


//! \since build 821
template<typename... _tParams>
ReductionStatus
CombinerReturnThunk(const ContextHandler& h, TermNode& term, ContextNode& ctx,
	_tParams&&... args)
{
	static_assert(sizeof...(args) < 2, "Unsupported owner arguments found.");

#if YF_Impl_NPLA1_Enable_TCO
	auto& fused_act(EnsureTCOAction(ctx, term));
	auto& lf(fused_act.LastFunction);

	lf = {};
	// XXX: Blocked. 'yforward' cause G++ 5.3 crash: internal compiler
	//	error: Segmentation fault.
	yunseq(0, (lf = NPL::make_observer(
		&fused_act.AttachFunction(std::forward<_tParams>(args)).get()), 0)...);
	fused_act.ReduceCombined = true;
	SetupNextTerm(ctx, term);
	return RelaySwitchedUnchecked(ctx,
		Continuation(std::ref(lf ? *lf : h), ctx));
#elif YF_Impl_NPLA1_Enable_Thunked
	SetupNextTerm(ctx, term);
	// TODO: Blocked. Use C++14 lambda initializers to simplify implementation.
	return RelayNext(ctx, Continuation(std::ref(h), ctx),
		std::bind([&](Reducer& act, const _tParams&...){
		// NOTE: Captured argument pack is only needed when %h actually shares.
		RelaySwitched(ctx, std::move(act));
		return RegularizeTerm(term, ctx.LastStatus);
	}, ctx.Switch(), std::move(args)...));
#else
	yunseq(0, args...);
	// NOTE: This does not support PTC.
	return RegularizeTerm(term, h(term, ctx));
#endif
}

//! \since build 822
ReductionStatus
ConsImpl(TermNode& term, bool by_val)
{
	using namespace Forms;

	RetainN(term, 2);

	const auto i(std::next(term.begin(), 2));
	auto& item(Deref(i));

	return ResolveTerm([&](TermNode& nd, bool has_ref) -> ReductionStatus{
		const auto get_ins_idx([&, i]{
			term.erase(i);
			// TODO: Simplify without relying on equivalence between
			//	%TermNode::Container and %ValueNode::Container.
			return GetLastIndexOf(term.GetContainer());
		});
		const auto ret([&]{
			RemoveHead(term);
			if(by_val)
				LiftSubtermsToReturn(term);
			return ReductionStatus::Retained;
		});

		if(has_ref)
		{
			const auto& tail(nd);

			if(IsList(tail))
			{
				auto idx(get_ins_idx());

				for(const auto& subnode : tail)
					NPL::AddChildToTail(++idx, term, subnode);
				return ret();
			}
		}
		else if(IsList(nd))
		{
			auto tail(std::move(nd));
			auto idx(get_ins_idx());

			for(auto& subnode : tail)
				NPL::AddChildToTail(++idx, term, std::move(subnode));
			return ret();
		}
		throw ListTypeError(
			ystdex::sfmt("Expected a list for the 2nd argument, got '%s'.",
			TermToStringWithReferenceMark(item, has_ref).c_str()));
	}, item);
}

//! \since build 834
//@{
template<typename _func>
void
SetFirstRest(_func f, TermNode& term)
{
	Forms::RetainN(term, 2);

	auto i(term.begin());

	ResolveTerm([&](TermNode& nd, bool has_ref){
		if(has_ref)
		{
			if(IsBranch(nd))
				f(nd, Deref(++i));
			else
				throw ListTypeError(ystdex::sfmt("Expected a non-empty list for"
					" the 1st argument, got '%s'.",
					TermToStringWithReferenceMark(nd, true).c_str()));
		}
		else
			throw ValueCategoryMismatch(ystdex::sfmt("Expected a lvalue for the"
				" 1st argument, got '%s'.", TermToString(nd).c_str()));
	}, Deref(++i));
	term.Value = ValueToken::Unspecified;
}

void
SetFirstImpl(TermNode& term, bool by_val)
{
	SetFirstRest([by_val](TermNode& node, TermNode& nd){
		auto& head(Deref(node.begin()));

		// XXX: How to simplify? Merge with %BindParameterObject?
		if([&]() -> bool{
			// XXX: Assume value representation of %nd is regular.
			if(const auto p = AccessPtr<const TermReference>(nd))
			{
				if(by_val)
					head.SetContent(p->get());
				else
					// XXX: No cyclic reference check.
					head.SetContent(TermNode::Container(head.get_allocator()),
						*p);
				return {};
			}
			return true;
		}())
			head.SetContent(std::move(nd));
	}, term);
}

void
SetRestImpl(TermNode& term, bool by_val)
{
	SetFirstRest([by_val](TermNode& node, TermNode& tm){
		ResolveTerm([&](TermNode& nd, bool has_ref){
			// XXX: How to simplify? Merge with %BindParameterObject?
			if(IsList(nd))
			{
				const auto a(node.get_allocator());
				TermNode nd_new(std::allocator_arg, a, {TermNode(a)},
					node.GetName());
				size_t idx(0);

				if(has_ref)
					for(const auto& subnode : nd)
						NPL::AddChildToTail(idx, nd_new, subnode);
				else
					for(const auto& subnode : nd)
						// XXX: No cyclic reference check.
						NPL::AddChildToTail(idx, nd_new, std::move(subnode));
				if(by_val)
					LiftSubtermsToReturn(nd_new);
				// XXX: The order is significant.
				Deref(nd_new.begin()) = std::move(Deref(node.begin()));
				swap(node, nd_new);
			}
			else
				throw ListTypeError(ystdex::sfmt("Expected a list for"
					" the 2nd argument, got '%s'.",
					TermToStringWithReferenceMark(nd, has_ref).c_str()));
		}, tm);
	}, term);
}
//@}

//! \since build 840
//@{
template<typename _func>
ReductionStatus
CreateFunction(TermNode& term, _func f, size_t n)
{
	Forms::Retain(term);
	if(term.size() > n)
		return f();
	throw InvalidSyntax("Insufficient terms in function abstraction.");
}

ReductionStatus
LambdaImpl(TermNode& term, ContextNode& ctx, bool no_lift)
{
	return CreateFunction(term, [&, no_lift]{
		auto p_env(ctx.ShareRecord());
		auto i(term.begin());
		auto formals(ShareMoveTerm(Deref(++i)));

		term.erase(term.begin(), ++i);
		// NOTE: %StrictContextHandler implies strict evaluation of arguments in
		//	%StrictContextHandler::operator().
		term.Value = ContextHandler(StrictContextHandler(VauHandler({},
			std::move(formals), std::move(p_env), {}, term, no_lift)));
		return ReductionStatus::Clean;
	}, 1);
}

//! \since build 842
ContextHandler
CreateVau(TermNode& term, bool no_lift, TermNode::iterator i,
	shared_ptr<Environment>&& p_env, bool owning)
{
	auto formals(ShareMoveTerm(Deref(++i)));
	auto eformal(CheckEnvFormal(Deref(++i)));

	term.erase(term.begin(), ++i);
	return FormContextHandler(VauHandler(std::move(eformal), std::move(formals),
		std::move(p_env), owning, term, no_lift));
}

ReductionStatus
VauImpl(TermNode& term, ContextNode& ctx, bool no_lift)
{
	return CreateFunction(term, [&, no_lift]{
		term.Value = CreateVau(term, no_lift, term.begin(), ctx.ShareRecord(),
			{});
		return ReductionStatus::Clean;
	}, 2);
}

ReductionStatus
VauWithEnvironmentImpl(TermNode& term, ContextNode& ctx, bool no_lift)
{
	return CreateFunction(term, [&, no_lift]{
		auto i(term.begin());
		auto& t(Deref(++i));

		return ReduceSubsequent(t, ctx, [&, i, no_lift]{
			// XXX: List components are ignored.
			auto p_env_pr(ResolveEnvironment(t));

			term.Value = CreateVau(term, no_lift, i,
				std::move(p_env_pr.first), p_env_pr.second);
			return ReductionStatus::Clean;
		});
	}, 3);
}
//@}

//! \since build 828
struct BindParameterObject
{
	//! \since build 847
	template<typename _fCopy, typename _fMove>
	void
	operator()(char sigil, bool copy, TermNode& b, _fCopy cp, _fMove mv,
		const AnchorPtr& p_anchor) const
	{
		// NOTE: The operand should have been evaluated. Subnodes in arguments
		//	retained are also transferred.
		// TODO: Support xvalues as currently rvalue references are not
		//	distinguished here.
		if(const auto p = AccessPtr<const TermReference>(b))
		{
			if(sigil == char())
				// NOTE: Since it is passed by value copy, direct destructive
				//	lifting cannot be used.
				cp(p->get().GetContainer(), p->get().Value);
			else
				mv(TermNode::Container(b.get_allocator()), *p);
		}
		else if(copy)
		{
			// NOTE: Binding on list rvalue is always unsafe.
			if(sigil == '&' && !IsList(b))
				// XXX: Always move because the value object is newly created.
				mv(TermNode::Container(b.get_allocator()),
					TermReference({}, b, p_anchor));
			else
				cp(b.GetContainer(), b.Value);
		}
		else
			// XXX: Moved. This is copy elision in object language.
			mv(std::move(b.GetContainerRef()), std::move(b.Value));
	}
};


//! \since build 834
//@{
class EncapsulationBase
{
private:
	// XXX: Is it possible to support %TermReference safety check here?
	// TODO: Add naming scheme and persistence interoperations?
	shared_ptr<void> p_type;

public:
	//! \since build 849
	EncapsulationBase(shared_ptr<void> p) ynothrow
		: p_type(std::move(p))
	{}
	DefDeCopyMoveCtorAssignment(EncapsulationBase)

	YB_ATTR_nodiscard YB_PURE friend PDefHOp(bool, ==,
		const EncapsulationBase& x, const EncapsulationBase& y) ynothrow
		ImplRet(x.p_type == y.p_type)

	DefGetter(const ynothrow, const EncapsulationBase&, , *this)
	DefGetter(ynothrow, EncapsulationBase&, Ref, *this)
	DefGetter(const ynothrow, const shared_ptr<void>&, Type, p_type)
};


class Encapsulation final : private EncapsulationBase
{
public:
	mutable TermNode Term;

	Encapsulation(shared_ptr<void> p, TermNode term)
		: EncapsulationBase(std::move(p)), Term(std::move(term))
	{}
	DefDeCopyMoveCtorAssignment(Encapsulation)

	friend PDefHOp(bool, ==, const Encapsulation& x, const Encapsulation& y)
		ynothrow
		ImplRet(x.Get() == y.Get())

	using EncapsulationBase::Get;
	using EncapsulationBase::GetType;
};


class Encapsulate final : private EncapsulationBase
{
public:
	Encapsulate(shared_ptr<void> p)
		: EncapsulationBase(std::move(p))
	{}
	DefDeCopyMoveCtorAssignment(Encapsulate)

	friend PDefHOp(bool, ==, const Encapsulate& x, const Encapsulate& y)
		ynothrow
		ImplRet(x.Get() == y.Get())

	void
	operator()(TermNode& term) const
	{
		Forms::CallUnary([this](TermNode& tm) YB_PURE{
			return Encapsulation(GetType(),
				ystdex::invoke_value_or(&TermReference::get,
				AccessTermPtr<const TermReference>(tm), std::move(tm)));
		}, term);
	}
};


class Encapsulated final : private EncapsulationBase
{
public:
	Encapsulated(shared_ptr<void> p)
		: EncapsulationBase(std::move(p))
	{}
	DefDeCopyMoveCtorAssignment(Encapsulated)

	YB_ATTR_nodiscard YB_PURE friend
		PDefHOp(bool, ==, const Encapsulated& x, const Encapsulated& y) ynothrow
		ImplRet(x.Get() == y.Get())

	void
	operator()(TermNode& term) const
	{
		Forms::CallUnary([this](TermNode& tm) YB_PURE ynothrow -> bool{
			return ystdex::call_value_or(
				[this](const Encapsulation& enc) YB_PURE ynothrow{
				return Get() == enc.Get();
			}, AccessTermPtr<Encapsulation>(tm));
		}, term);
	}
};


class Decapsulate final : private EncapsulationBase
{
public:
	Decapsulate(shared_ptr<void> p)
		: EncapsulationBase(std::move(p))
	{}
	DefDeCopyMoveCtorAssignment(Decapsulate)

	YB_ATTR_nodiscard YB_PURE friend
		PDefHOp(bool, ==, const Decapsulate& x, const Decapsulate& y) ynothrow
		ImplRet(x.Get() == y.Get())

	void
	operator()(TermNode& term) const
	{
		Forms::CallUnaryAs<const Encapsulation>(
			[](const Encapsulation& enc) YB_PURE ynothrow{
			// XXX: No environment is captured as the owner is shared.
			return TermReference(enc.Term);
		}, term);
	}
};
//@}

} // unnamed namespace;


ContextState::ContextState(YSLib::pmr::memory_resource& rsrc)
	: ContextNode(rsrc)
{}
ContextState::ContextState(const ContextState& ctx)
	: ContextNode(ctx),
	EvaluateLeaf(ctx.EvaluateLeaf), EvaluateList(ctx.EvaluateList),
	EvaluateLiteral(ctx.EvaluateLiteral), Guard(ctx.Guard)
{}
ContextState::ContextState(ContextState&& ctx)
	: ContextNode(std::move(ctx)),
	EvaluateLeaf(ctx.EvaluateLeaf), EvaluateList(ctx.EvaluateList),
	EvaluateLiteral(ctx.EvaluateLiteral), Guard(ctx.Guard)
{
	swap(next_term_ptr, ctx.next_term_ptr);
}
ContextState::ImplDeDtor(ContextState)

TermNode&
ContextState::GetNextTermRef() const
{
	if(const auto p = next_term_ptr)
		return *p;
	// NOTE: This should not occur unless there exists some invalid low-level
	//	interoperations on the next term pointer in the context.
	throw NPLException("No next term found to evaluation.");
}

void
ContextState::SetNextTermRef(TermNode& term)
{
	next_term_ptr = NPL::make_observer(&term);
}

ReductionStatus
ContextState::RewriteGuarded(TermNode& term, Reducer reduce)
{
	const auto gd(Guard(term, *this));

	return Rewrite(reduce);
}


ReductionStatus
Reduce(TermNode& term, ContextNode& ctx)
{
#if YF_Impl_NPLA1_Enable_Thunked
	// TODO: Support other states?
	ystdex::swap_guard<Reducer> gd(true, ctx.Current);

#endif
	SetupNextTerm(ctx, term);
	return ystdex::polymorphic_downcast<ContextState&>(ctx).RewriteGuarded(term,
		Continuation(ReduceOnce, ctx));
}

ReductionStatus
ReduceAgain(TermNode& term, ContextNode& ctx)
{
#if YF_Impl_NPLA1_Enable_Thunked
	Continuation reduce_again(ReduceOnce, ctx);

	SetupNextTerm(ctx, term);
#	if YF_Impl_NPLA1_Enable_TCO

	auto& act(EnsureTCOAction(ctx, term));

	act.ReduceNestedAsync = true,
	act.CleanupOrphanTemporary();
	return RelaySwitchedUnchecked(ctx, std::move(reduce_again));
#	else
	return RelayNext(ctx, std::move(reduce_again),
		ComposeActions(ctx, []() ynothrow{
		return ReductionStatus::Retrying;
	}, ctx.Switch()));
#	endif
#else
	return Reduce(term, ctx);
#endif
}

void
ReduceArguments(TNIter first, TNIter last, ContextNode& ctx)
{
	if(first != last)
		// NOTE: This is neutral to thunks.
		// NOTE: The order of evaluation is unspecified by the language
		//	specification. It should not be depended on.
		ReduceChildren(++first, last, ctx);
	else
		throw InvalidSyntax("Invalid function application found.");
}

ReductionStatus
ReduceChecked(TermNode& term, ContextNode& ctx)
{
#if YF_Impl_NPLA1_Enable_Thunked
	// XXX: Assume it is always reducing the same term and the next actions are
	//	safe to be dropped.
#	if YF_Impl_NPLA1_Enable_TCO
	RelaySwitched(ctx, [&]{
		if(ctx.LastStatus == ReductionStatus::Retrying)
		{
			SetupNextTerm(ctx, term);
			return RelaySwitched(ctx, Continuation(ReduceOnce, ctx));
		}
		return ctx.LastStatus;
	});
	return ReductionStatus::Retrying;
#	else
	SetupNextTerm(ctx, term);
	return RelayNext(ctx, Continuation(ReduceOnce, ctx),
		std::bind([&](Reducer& act){
		RelaySwitched(ctx, std::move(act));
		return ReductionStatus::Retrying;
	}, ctx.Switch()));
#	endif
#else
	// NOTE: This does not support PTC.
	ReduceCheckedSync(term, ctx);
	return CheckNorm(term);
#endif
}

void
ReduceChildren(TNIter first, TNIter last, ContextNode& ctx)
{
	// NOTE: Separators or other sequence constructs are not handled here. The
	//	evaluation can be potentionally parallel, though the simplest one is
	//	left-to-right.
	// XXX: Use execution policies to be evaluated concurrently?
	// NOTE: This does not support PTC, but allow it to be called
	//	in routines which expect proper tail actions, given the guarnatee that
	//	the precondition of %Reduce is not violated.
	// XXX: The remained tail action would be dropped.
#if YF_Impl_NPLA1_Enable_Thunked
	ReduceChildrenOrderedAsync(first, last, ctx);
#else
	std::for_each(first, last, ystdex::bind1(ReduceCheckedSync, std::ref(ctx)));
#endif
}

ReductionStatus
ReduceChildrenOrdered(TNIter first, TNIter last, ContextNode& ctx)
{
#if YF_Impl_NPLA1_Enable_Thunked
	return ReduceChildrenOrderedAsync(first, last, ctx);
#else
	// NOTE: This does not support PTC.
	const auto tr([&](TNIter iter){
		return ystdex::make_transform(iter, [&](TNIter i){
			ReduceCheckedSync(*i, ctx);
			return CheckNorm(*i);
		});
	});

	return ystdex::default_last_value<ReductionStatus>()(tr(first), tr(last),
		ReductionStatus::Clean);
#endif
}

ReductionStatus
ReduceFirst(TermNode& term, ContextNode& ctx)
{
	// NOTE: This is neutral to thunks.
	return IsBranch(term) ? ReduceOnce(Deref(term.begin()), ctx)
		: ReductionStatus::Clean;
}

ReductionStatus
ReduceOnce(TermNode& term, ContextNode& cb)
{
	auto& ctx(ystdex::polymorphic_downcast<ContextState&>(cb));

	if(IsBranch(term))
	{
		YAssert(term.size() != 0, "Invalid node found.");
		if(term.size() != 1)
			// NOTE: List evaluation.
			return DoAdministratives(ctx.EvaluateList, term, ctx);
		else
		{
			// NOTE: List with single element shall be reduced as the
			//	element.
			LiftFirst(term);
			return ReduceAgain(term, ctx);
		}
	}

	const auto& tp(term.Value.type());

	// NOTE: Empty list or special value token has no-op to do with.
	// XXX: Add logic to directly handle special value tokens here?
	// NOTE: The reduction relies on proper handling of reduction status and
	//	proper tail action for the thunked implementations.
	return tp != ystdex::type_id<void>() && tp != ystdex::type_id<ValueToken>()
		? DoAdministratives(ctx.EvaluateLeaf, term, ctx)
		: ReductionStatus::Clean;
}

ReductionStatus
ReduceOrdered(TermNode& term, ContextNode& ctx)
{
#if YF_Impl_NPLA1_Enable_Thunked
#	if YF_Impl_NPLA1_Enable_TailRewriting
	if(!term.empty())
		return ReduceSequenceOrderedAsync(term, ctx, term.begin());
	term.Value = ValueToken::Unspecified;
	return ReductionStatus::Retained;
#	else
	SetupNextTerm(ctx, term);
	// TODO: Blocked. Use C++14 lambda initializers to simplify implementation.
	return RelayNext(ctx, Continuation(static_cast<ReductionStatus(&)(TermNode&,
		ContextNode&)>(ReduceChildrenOrdered), ctx),
		std::bind([&](Reducer& act){
		RelaySwitched(ctx, std::move(act));
		if(!term.empty())
			LiftTerm(term, *term.rbegin());
		else
			term.Value = ValueToken::Unspecified;
		return CheckNorm(term);
	}, ctx.Switch()));
#	endif
#else
	// NOTE: This does not support PTC.
	const auto res(ReduceChildrenOrdered(term, ctx));

	if(!term.empty())
		LiftTerm(term, *term.rbegin());
	else
		term.Value = ValueToken::Unspecified;
	return res;
#endif
}

ReductionStatus
ReduceTail(TermNode& term, ContextNode& ctx, TNIter i)
{
	term.erase(term.begin(), i);
	// NOTE: This is neutral to thunks.
	return ReduceAgain(term, ctx);
}


void
SetupTraceDepth(ContextState& root, const string& name)
{
	yunseq(
	ystdex::try_emplace(root.GetBindingsRef(), name, NoContainer, name,
		in_place_type<size_t>),
	root.Guard = [name](TermNode& term, ContextNode& ctx){
		if(const auto p = ctx.GetRecordRef().LookupName(name))
		{
			using ystdex::pvoid;
			auto& depth(NPL::Access<size_t>(*p));

			YTraceDe(Informative, "Depth = %zu, context = %p, semantics = %p.",
				depth, pvoid(&ctx), pvoid(&term));
			++depth;
			return ystdex::unique_guard([&]() ynothrow{
				--depth;
			});
		}
		else
			ValueNode::ThrowWrongNameFound(name);
	}
	);
}


TermNode
TransformForSeparator(const TermNode& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	return TransformForSeparatorTmpl(term, pfx, delim);
}
TermNode
TransformForSeparator(TermNode&& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	return TransformForSeparatorTmpl(std::move(term), pfx, delim);
}

TermNode
TransformForSeparatorRecursive(const TermNode& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	return TransformForSeparatorRecursiveTmpl(term, pfx, delim);
}
TermNode
TransformForSeparatorRecursive(TermNode&& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	return TransformForSeparatorRecursiveTmpl(std::move(term), pfx, delim);
}

ReductionStatus
ReplaceSeparatedChildren(TermNode& term, const ValueObject& pfx,
	const TokenValue& delim)
{
	if(std::find_if(term.begin(), term.end(),
		ystdex::bind1(HasValue<TokenValue>, std::ref(delim))) != term.end())
		term = TransformForSeparator(std::move(term), pfx, delim);
	return ReductionStatus::Clean;
}


ReductionStatus
FormContextHandler::operator()(TermNode& term, ContextNode& ctx) const
{
	auto cont([this](TermNode& t, ContextNode& c) -> ReductionStatus{
		// XXX: Is it worth matching specific builtin special forms here?
		// FIXME: Exception filtering does not work well with thunks.
		try
		{
			if(!Check || Check(t))
				return Handler(t, c);
			// XXX: Use more specific exception type?
			throw std::invalid_argument("Term check failed.");
		}
		CatchExpr(NPLException&, throw)
		// TODO: Use semantic exceptions.
		CatchThrow(bad_any_cast& e, LoggedEvent(ystdex::sfmt(
			"Mismatched types ('%s', '%s') found.", e.from(), e.to()), Warning))
		// TODO: Use nested exceptions?
		CatchThrow(std::exception& e, LoggedEvent(e.what(), Err))
		// XXX: Use distinct status for failure?
		return ReductionStatus::Clean;
	});

#if YF_Impl_NPLA1_Enable_Thunked
	SetupNextTerm(ctx, term);
	return RelaySwitched(ctx, Continuation(cont, ctx));
#else
	// NOTE: This does not support PTC.
	return cont(term, ctx);
#endif
}


ReductionStatus
StrictContextHandler::operator()(TermNode& term, ContextNode& ctx) const
{
	// NOTE: This implementes arguments evaluation in applicative order.
#if YF_Impl_NPLA1_Enable_Thunked
	// TODO: Optimize for cases with no argument.
	SetupNextTerm(ctx, term);
	// TODO: Blocked. Use C++14 lambda initializers to simplify implementation.
	return RelayNext(ctx, Continuation([&](TermNode& t, ContextNode& c){
		ReduceArguments(t, c);
		return ReductionStatus::Partial;
	}, ctx), ComposeActions(ctx,
		// NOTE: Call of %RecoverNextTerm can also be composed here, however it
		//	is inefficient.
		Continuation([&](TermNode&, ContextNode& c){
		RecoverNextTerm(term, ctx);
		return Handler(term, c);
	}, ctx), ctx.Switch()));
#else
	// NOTE: This does not support PTC.
	ReduceArguments(term, ctx);
	return Handler(term, ctx);
#endif
}


void
RegisterSequenceContextTransformer(EvaluationPasses& passes,
	const TokenValue& delim, bool ordered)
{
	passes += ystdex::bind1(ReplaceSeparatedChildren,
		ordered ? ContextHandler(Forms::Sequence)
		: ContextHandler(StrictContextHandler(ReduceBranchToList)), delim);
}


ReductionStatus
EvaluateDelayed(TermNode& term)
{
	return ystdex::call_value_or([&](DelayedTerm& delayed){
		return EvaluateDelayed(term, delayed);
	}, AccessPtr<DelayedTerm>(term), ReductionStatus::Clean);
}
ReductionStatus
EvaluateDelayed(TermNode& term, DelayedTerm& delayed)
{
	// NOTE: This does not rely on tail action.
	// NOTE: The referenced term is lived through the envaluation, which is
	//	guaranteed by the evaluated parent term.
	LiftDelayed(term, delayed);
	// NOTE: To make it work with %DetectReducible.
	return ReductionStatus::Partial;
}

ReductionStatus
EvaluateIdentifier(TermNode& term, const ContextNode& ctx, string_view id)
{
	// NOTE: This is conversion of lvalue in object to value of expression.
	//	The referenced term is lived through the evaluation, which is guaranteed
	//	by the context. This is necessary since the ownership of objects which
	//	are not temporaries in evaluated terms needs to be always in the
	//	environment, rather than in the tree. It would be safe if not passed
	//	directly and without rebinding. Note access of objects denoted by
	//	invalid reference after rebinding would cause undefined behavior in the
	//	object language.
	auto t_pr(ResolveIdentifier(ctx, id));

	term.Value = TermReference(t_pr.second, std::move(t_pr.first));
	// NOTE: This is not guaranteed to be saved as %ContextHandler in
	//	%ReduceCombined.
	if(const auto p_handler = AccessTermPtr<LiteralHandler>(term))
		return (*p_handler)(ctx);
	// NOTE: Unevaluated term shall be detected and evaluated. See also
	//	$2017-05 @ %Documentation::Workflow::Annual2017.
	return IsLeaf(term) ? (term.Value.type()
		!= ystdex::type_id<TokenValue>() ? EvaluateDelayed(term)
		: ReductionStatus::Clean) : ReductionStatus::Retained;
}

ReductionStatus
EvaluateLeafToken(TermNode& term, ContextNode& cb, string_view id)
{
	auto& ctx(ystdex::polymorphic_downcast<ContextState&>(cb));

	YAssertNonnull(id.data());
	// NOTE: Only string node of identifier is tested.
	if(!id.empty())
	{
		// NOTE: The term would be normalized by %ReduceCombined.
		//	If necessary, there can be inserted some additional cleanup to
		//	remove empty tokens, returning %ReductionStatus::Partial.
		//	Separators should have been handled in appropriate preprocessing
		//	passes.
		const auto lcat(CategorizeBasicLexeme(id));

		switch(lcat)
		{
		case LexemeCategory::Code:
			// XXX: When do code literals need to be evaluated?
			id = DeliteralizeUnchecked(id);
			if(YB_UNLIKELY(id.empty()))
				break;
			YB_ATTR_fallthrough;
		case LexemeCategory::Symbol:
			YAssertNonnull(id.data());
			// XXX: Asynchronous reduction is currently not supported.
			return CheckReducible(ctx.EvaluateLiteral(term, ctx, id))
				? EvaluateIdentifier(term, ctx, id) : ReductionStatus::Clean;
			// XXX: Empty token is ignored.
			// XXX: Remained reducible?
		case LexemeCategory::Data:
			// XXX: This should be prevented being passed to second pass in
			//	%TermToNamePtr normally. This is guarded by normal form handling
			//	in the loop in %ContextNode::Rewrite with %ReduceOnce.
			term.Value.emplace<string>(Deliteralize(id));
			YB_ATTR_fallthrough;
		default:
			break;
			// XXX: Handle other categories of literal?
		}
	}
	return ReductionStatus::Clean;
}

ReductionStatus
ReduceCombined(TermNode& term, ContextNode& ctx)
{
	if(IsBranch(term))
	{
		auto& fm(Deref(term.begin()));

		// NOTE: This is neutral to thunks.
		if(const auto p_handler = AccessPtr<ContextHandler>(fm))
#if YF_Impl_NPLA1_Enable_Thunked
		{
#	if YF_Impl_NPLA1_Enable_TCO
			return CombinerReturnThunk(*p_handler, term, ctx,
				std::move(*p_handler));
#	else
			// XXX: Optimize for performance using context-dependent store?
			// XXX: This should ideally be a member of handler. However, it
			//	makes no sense before allowing %ContextHandler overload for
			//	ref-qualifier '&&'.
			auto p(share_move(ctx.get_allocator(), *p_handler));

			return CombinerReturnThunk(*p, term, ctx, std::move(p));
#	endif
		}
#else
			return CombinerReturnThunk(ContextHandler(std::move(*p_handler)),
				term, ctx);
#endif
		if(const auto p_handler = AccessTermPtr<ContextHandler>(fm))
			return CombinerReturnThunk(*p_handler, term, ctx);
		ResolveTerm(
			[&](const TermNode& nd, bool has_ref) YB_ATTR(noreturn){
			// TODO: Capture contextual information in error.
			// TODO: Extract general form information extractor function.
			throw ListReductionFailure(ystdex::sfmt("No matching combiner '%s'"
				" for operand with %zu argument(s) found.",
				TermToStringWithReferenceMark(nd, has_ref).c_str(),
				FetchArgumentN(term)));
		}, fm);
	}
	return ReductionStatus::Clean;
}

ReductionStatus
ReduceLeafToken(TermNode& term, ContextNode& ctx)
{
	const auto res(ystdex::call_value_or([&](string_view id){
		return EvaluateLeafToken(term, ctx, id);
	// XXX: A term without token is ignored.
	}, TermToNamePtr(term), ReductionStatus::Clean));

	return CheckReducible(res) ? ReduceAgain(term, ctx) : res;
}


void
SetupDefaultInterpretation(ContextState& root, EvaluationPasses passes)
{
	passes += ReduceHeadEmptyList;
	passes += ReduceFirst;
	// TODO: Insert more optional optimized lifted form evaluation passes.
	// XXX: This should be the last of list pass for current TCO
	//	implementation, assumed by TCO action.
	passes += ReduceCombined;
	root.EvaluateList = std::move(passes);
	root.EvaluateLeaf = ReduceLeafToken;
}


REPLContext::REPLContext(bool trace, YSLib::pmr::memory_resource& rsrc)
	: Allocator(&rsrc), Root(rsrc)
{
	using namespace std::placeholders;

	SetupDefaultInterpretation(Root,
		std::bind(std::ref(ListTermPreprocess), _1, _2));
	if(trace)
		SetupTraceDepth(Root);
}

TermNode
REPLContext::Perform(string_view unit, ContextNode& ctx)
{
	YAssertNonnull(unit.data());
	if(!unit.empty())
		return Process(Session(unit), ctx);
	throw LoggedEvent("Empty token list found.", Alert);
}

void
REPLContext::Prepare(TermNode& term) const
{
	TokenizeTerm(term);
	Preprocess(term);
}
TermNode
REPLContext::Prepare(const TokenList& token_list) const
{
	auto term(SContext::Analyze(token_list));

	Prepare(term);
	return term;
}
TermNode
REPLContext::Prepare(const Session& session) const
{
	auto term(SContext::Analyze(session, Allocator));

	Prepare(term);
	return term;
}

void
REPLContext::Process(TermNode& term, ContextNode& ctx) const
{
	Prepare(term);
	Reduce(term, ctx);
}

TermNode
REPLContext::ReadFrom(std::istream& is) const
{
	if(is)
	{
		if(const auto p = is.rdbuf())
			return ReadFrom(*p);
		else
			throw std::invalid_argument("Invalid stream buffer found.");
	}
	else
		throw std::invalid_argument("Invalid stream found.");
}
TermNode
REPLContext::ReadFrom(std::streambuf& buf) const
{
	using s_it_t = std::istreambuf_iterator<char>;

	return Prepare(Session(s_it_t(&buf), s_it_t()));
}


namespace Forms
{

bool
IsSymbol(const string& id) ynothrow
{
	return IsNPLASymbol(id);
}

TokenValue
StringToSymbol(const string& s)
{
	return s;
}

const string&
SymbolToString(const TokenValue& s) ynothrow
{
	return s;
}


size_t
RetainN(const TermNode& term, size_t m)
{
	const auto n(FetchArgumentN(term));

	if(n != m)
		throw ArityMismatch(m, n);
	return n;
}


void
BindParameter(ContextNode& ctx, const TermNode& t, TermNode& o)
{
	auto& env(ctx.GetRecordRef());
	const auto check_sigil([&](string_view& id){
		char sigil(id.front());

		if(sigil != '&' && sigil != '%')
			sigil = char();
		else
			id.remove_prefix(1);
		return sigil;
	});
	const auto p_anchor([&]() -> AnchorPtr{
#if YF_Impl_NPLA1_Enable_TCO
		return ystdex::call_value_or([&](TCOAction& a){
			return ystdex::call_value_or(
				std::mem_fn(&Environment::Anchor), a.TemporaryPtr);
		}, AccessTCOAction(ctx));
#endif
		return {};
	}());

	// NOTE: No duplication check here. Symbols can be rebound.
	// TODO: Support xvalues. See %BindParameterObject.
	// XXX: Additional ownership check?
	MatchParameter(t, o,
		[&, check_sigil](TNIter first, TNIter last, string_view id, bool copy){
		YAssert(ystdex::begins_with(id, "."), "Invalid symbol found.");
		id.remove_prefix(1);
		if(!id.empty())
		{
			const char sigil(check_sigil(id));

			if(!id.empty())
			{
				TermNode::Container con(t.get_allocator());

				for(; first != last; ++first)
					BindParameterObject()(sigil, copy, Deref(first), [&](const
						TermNode::Container& c, const ValueObject& vo){
						con.emplace(c, MakeIndex(con.size()), vo);
					}, [&](TermNode::Container&& c, ValueObject&& vo){
						con.emplace(std::move(c), MakeIndex(con.size()),
							std::move(vo));
					}, p_anchor);

				auto& term(env[id]);

				// XXX: This relies on the assumption that %ValueNode has same
				//	container type with %TermNode.
				term.SetContent(ValueNode(std::move(con)));
			}
		}
	}, [&](const TokenValue& n, TermNode& b, bool copy){
		CheckParameterLeafToken(n, [&]{
			if(!n.empty())
			{
				string_view id(n);
				const char sigil(check_sigil(id));

				if(!id.empty())
					BindParameterObject()(sigil, copy, b, [&](const
						TermNode::Container& c, const ValueObject& vo){
						env[id].SetContent(c, vo);
					}, [&](TermNode::Container&& c, ValueObject&& vo){
						env[id].SetContent(std::move(c), std::move(vo));
					}, p_anchor);
			}
		});
	}, {});
}

void
MatchParameter(const TermNode& t, TermNode& o, function<void(TNIter,
	TNIter, const TokenValue&, bool)> bind_trailing_seq,
	function<void(const TokenValue&, TermNode&, bool)> bind_value, bool o_copy)
{
	if(IsBranch(t))
	{
		const auto n_p(t.size());
		auto last(t.end());

		if(n_p > 0)
		{
			const auto& back(Deref(std::prev(last)));

			if(IsLeaf(back) && ReferenceTerm(back))
			{
				if(const auto p = AccessPtr<TokenValue>(back))
				{
					if(!p->empty() && p->front() == '.')
						--last;
				}
				else
					throw ParameterMismatch(ystdex::sfmt(
						"Invalid term '%s' found for symbol parameter.",
						TermToString(back).c_str()));
			}
		}

		const auto match_branch(
			[&, n_p](TermNode& a, bool ellipsis, bool copy) -> bool{
			const auto n_o(a.size());

			if(n_p == n_o || (ellipsis && n_o >= n_p - 1))
			{
				auto j(a.begin());

				for(auto i(t.begin()); i != last; yunseq(++i, ++j))
				{
					YAssert(j != a.end(), "Invalid state of operand found.");
					MatchParameter(Deref(i), Deref(j), bind_trailing_seq,
						bind_value, o_copy || copy);
				}
				if(ellipsis)
				{
					const auto& lastv(Deref(last).Value);

					YAssert(lastv.type() == ystdex::type_id<TokenValue>(),
						"Invalid ellipsis sequence token found.");
					bind_trailing_seq(j, a.end(),
						lastv.GetObject<TokenValue>(), copy);
					return true;
				}
				return {};
			}
			else if(!ellipsis)
				throw ArityMismatch(n_p, n_o);
			else
				throw ParameterMismatch(
					"Insufficient term found for list parameter.");
		});
		const auto match_operand_branch([&](TermNode& a, bool copy){
			if(match_branch(a, last != t.end(), copy))
				YAssert(++last == t.end(), "Invalid state found.");
		});
		if(const auto p = AccessPtr<const TermReference>(o))
			// XXX: There is only one level of indirection. It should work if
			//	reference collapse is correctly implemented.
			match_operand_branch(p->get(), true);
		else
			match_operand_branch(o, o_copy);
	}
	else if(!t.Value)
		ResolveTerm([&](const TermNode& nd, bool has_ref){
			if(nd)
				throw ParameterMismatch(ystdex::sfmt("Invalid nonempty operand"
					" value '%s' found for empty list parameter.",
					TermToStringWithReferenceMark(nd, has_ref).c_str()));
		}, o);
	else if(const auto p = AccessPtr<TokenValue>(t))
		bind_value(*p, o, o_copy);
	else
		throw ParameterMismatch(ystdex::sfmt(
			"Invalid parameter value '%s' found.", TermToString(t).c_str()));
}


ReductionStatus
DefineLazy(TermNode& term, ContextNode& ctx)
{
	DoDefine(term, [&](TermNode& formals){
		BindParameter(ctx, formals, term);
		return ReductionStatus::Clean;
	});
	return DoDefineReturn(term);
}

ReductionStatus
DefineWithNoRecursion(TermNode& term, ContextNode& ctx)
{
	// XXX: Terms shall be moved and saved into the actions for thunked code.
#if YF_Impl_NPLA1_Enable_Thunked
	DoDefine(term, [&](TermNode& formals){
		// TODO: Blocked. Use C++14 lambda initializers to simplify
		//	implementation.
		return ReduceSubsequent(term, ctx, std::bind([&](const TermNode& saved){
			// NOTE: This does not support PTC.
			BindParameter(ctx, saved, term);
			return DoDefineReturn(term);
		}, std::move(formals)));
	});
	return ReductionStatus::Partial;
#else
	DoDefine(term, [&](const TermNode& formals){
		ReduceCheckedSync(term, ctx);
		// NOTE: This does not support PTC.
		BindParameter(ctx, formals, term);
	});
	return DoDefineReturn(term);
#endif
}

ReductionStatus
DefineWithRecursion(TermNode& term, ContextNode& ctx)
{
	// XXX: Terms shall be moved and saved into the actions for thunked code.
#if YF_Impl_NPLA1_Enable_Thunked
	DoDefine(term, [&](TermNode& formals){
		auto p_saved(ShareMoveTerm(formals));
		// TODO: Avoid %shared_ptr.
		auto p_thunk(YSLib::allocate_shared<RecursiveThunk>(
			term.get_allocator(), ctx.GetRecordRef(), *p_saved));

		// TODO: Blocked. Use C++14 lambda initializers to simplify
		//	implementation.
		return ReduceSubsequent(term, ctx, std::bind([&](const
			shared_ptr<TermNode>&, const shared_ptr<RecursiveThunk>& p_gd){
			// NOTE: This does not support PTC.
			BindParameter(ctx, p_gd->Term, term);
			// NOTE: This cannot support PTC because only the implicit commit
			//	operation is in the tail context.
			p_gd->Commit();
			return DoDefineReturn(term);
		}, std::move(p_saved), YSLib::allocate_shared<RecursiveThunk>(
			term.get_allocator(), ctx.GetRecordRef(), *p_saved)));
	});
	return ReductionStatus::Partial;
#else
	DoDefine(term, [&](const TermNode& formals){
		// NOTE: This does not support PTC.
		RecursiveThunk gd(ctx.GetRecordRef(), formals);

		ReduceCheckedSync(term, ctx);
		BindParameter(ctx, formals, term);
		// NOTE: This cannot support PTC because only the implicit commit
		//	operation is in the tail context.
		gd.Commit();
	});
	return DoDefineReturn(term);
#endif
}

void
Undefine(TermNode& term, ContextNode& ctx, bool forced)
{
	Retain(term);
	if(term.size() == 2)
	{
		const auto&
			n(AccessTerm<const TokenValue>(Deref(std::next(term.begin()))));

		if(IsNPLASymbol(n))
			term.Value = ctx.GetRecordRef().Remove(n, forced);
		else
			throw InvalidSyntax(ystdex::sfmt("Invalid token '%s' found as name"
				" to be undefined.", n.c_str()));
	}
	else
		throw
			InvalidSyntax("Expected exact one term as name to be undefined.");
}


ReductionStatus
If(TermNode& term, ContextNode& ctx)
{
	Retain(term);

	const auto size(term.size());

	if(size == 3 || size == 4)
	{
		auto i(std::next(term.begin()));

		return ReduceSubsequent(*i, ctx, [&, i]{
			auto j(i);

			if(!ExtractBool(*j, true))
				++j;
			return ++j != term.end() ? ReduceAgainLifted(term, ctx, *j)
				: ReductionStatus::Clean;
		});
	}
	else
		throw InvalidSyntax("Syntax error in conditional form.");
}

ReductionStatus
Lambda(TermNode& term, ContextNode& ctx)
{
	return LambdaImpl(term, ctx, {});
}

ReductionStatus
LambdaRef(TermNode& term, ContextNode& ctx)
{
	return LambdaImpl(term, ctx, true);
}

ReductionStatus
Vau(TermNode& term, ContextNode& ctx)
{
	return VauImpl(term, ctx, {});
}

ReductionStatus
VauRef(TermNode& term, ContextNode& ctx)
{
	return VauImpl(term, ctx, true);
}

ReductionStatus
VauWithEnvironment(TermNode& term, ContextNode& ctx)
{
	return VauWithEnvironmentImpl(term, ctx, {});
}

ReductionStatus
VauWithEnvironmentRef(TermNode& term, ContextNode& ctx)
{
	return VauWithEnvironmentImpl(term, ctx, true);
}


ReductionStatus
Sequence(TermNode& term, ContextNode& ctx)
{
	Retain(term);
	RemoveHead(term);
	return ReduceOrdered(term, ctx);
}


ReductionStatus
And(TermNode& term, ContextNode& ctx)
{
	return AndOr(term, ctx, true);
}

ReductionStatus
Or(TermNode& term, ContextNode& ctx)
{
	return AndOr(term, ctx, {});
}


void
CallSystem(TermNode& term)
{
	CallUnaryAs<const string>(
		ystdex::compose(usystem, std::mem_fn(&string::c_str)), term);
}

ReductionStatus
Cons(TermNode& term)
{
	return ConsImpl(term, true);
}

ReductionStatus
ConsRef(TermNode& term)
{
	return ConsImpl(term, {});
}

void
SetFirst(TermNode& term)
{
	SetFirstImpl(term, true);
}

void
SetFirstRef(TermNode& term)
{
	SetFirstImpl(term, {});
}

void
SetRest(TermNode& term)
{
	SetRestImpl(term, true);
}

void
SetRestRef(TermNode& term)
{
	SetRestImpl(term, {});
}

void
Equal(TermNode& term)
{
	EqualTermReference(term, [](const TermNode& x, const TermNode& y){
		const bool lx(IsLeaf(x)), ly(IsLeaf(y));

		return lx && ly ? YSLib::HoldSame(x.Value, y.Value) : &x == &y;
	});
}

void
EqualLeaf(TermNode& term)
{
	EqualTermValue(term, ystdex::equal_to<>());
}

void
EqualReference(TermNode& term)
{
	EqualTermValue(term, YSLib::HoldSame);
}

void
EqualValue(TermNode& term)
{
	EqualTermReference(term, [](const TermNode& x, const TermNode& y){
		const bool lx(IsLeaf(x)), ly(IsLeaf(y));

		return lx && ly ? x.Value == y.Value : &x == &y;
	});
}

ReductionStatus
Eval(TermNode& term, ContextNode& ctx)
{
	return EvalImpl(term, ctx, {});
}

ReductionStatus
EvalRef(TermNode& term, ContextNode& ctx)
{
	return EvalImpl(term, ctx, true);
}

ReductionStatus
EvalString(TermNode& term, ContextNode& ctx)
{
	return EvalStringImpl(term, ctx, {});
}

ReductionStatus
EvalStringRef(TermNode& term, ContextNode& ctx)
{
	return EvalStringImpl(term, ctx, true);
}

ReductionStatus
EvalUnit(TermNode& term)
{
	CallBinaryAs<const string, REPLContext>(
		[&](const string& unit, REPLContext& rctx){
		term.SetContent(rctx.Perform(unit));
	}, term);
	return ReductionStatus::Retained;
}

void
GetCurrentEnvironment(TermNode& term, ContextNode& ctx)
{
	RetainN(term, 0);
	term.Value = ValueObject(ctx.WeakenRecord());
}

void
LockCurrentEnvironment(TermNode& term, ContextNode& ctx)
{
	RetainN(term, 0);
	term.Value = ValueObject(ctx.ShareRecord());
}

ReductionStatus
MakeEncapsulationType(TermNode& term)
{
	shared_ptr<void> p_type(new yimpl(byte));
	const auto& tag(in_place_type<ContextHandler>);

	term.GetContainerRef() = {{NoContainer, MakeIndex(0), tag,
		StrictContextHandler(Encapsulate(p_type))}, {NoContainer,
		MakeIndex(1), tag, StrictContextHandler(Encapsulated(p_type))},
		{NoContainer, MakeIndex(2), tag,
		StrictContextHandler(Decapsulate(p_type))}};
	return ReductionStatus::Retained;
}

void
MakeEnvironment(TermNode& term)
{
	Retain(term);

	const auto a(term.get_allocator());

	if(term.size() > 1)
	{
		ValueObject parent;
		const auto tr([&](TNCIter iter){
			return ystdex::make_transform(iter, [&](TNCIter i){
				return ReferenceTerm(*i).Value;
			});
		});

		parent.emplace<EnvironmentList>(tr(std::next(term.begin())),
			tr(term.end()), a);
		term.Value = NPL::AllocateEnvironment(a, std::move(parent));
	}
	else
		term.Value = NPL::AllocateEnvironment(a);
}

ReductionStatus
ValueOf(TermNode& term, const ContextNode& ctx)
{
	RetainN(term);
	LiftToOther(term, Deref(std::next(term.begin())));
	if(const auto p_id = AccessPtr<string>(term))
		TryRet(EvaluateIdentifier(term, ctx, *p_id))
		CatchIgnore(BadIdentifier&)
	term.Value = ValueToken::Null;
	return ReductionStatus::Clean;
}


ContextHandler
Wrap(const ContextHandler& h)
{
	return StrictContextHandler(h);
}

ContextHandler
WrapOnce(const ContextHandler& h)
{
	if(const auto p = h.target<FormContextHandler>())
		return StrictContextHandler(*p);
	throw TypeError(ystdex::sfmt("Wrapping failed with type '%s'.",
		h.target_type().name()));
}

ContextHandler
Unwrap(const ContextHandler& h)
{
	if(const auto p = h.target<StrictContextHandler>())
		return ContextHandler(p->Handler);
	throw TypeError(ystdex::sfmt("Unwrapping failed with type '%s'.",
		h.target_type().name()));
}

} // namespace Forms;

} // namesapce A1;

} // namespace NPL;

