/** 
 * @file llviewquery.h
 * @brief Query algorithm for flattening and filtering the view hierarchy.
 *
 * $LicenseInfo:firstyear=2001&license=viewergpl$
 * 
 * Copyright (c) 2001-2009, Linden Research, Inc.
 * 
 * Second Life Viewer Source Code
 * The source code in this file ("Source Code") is provided by Linden Lab
 * to you under the terms of the GNU General Public License, version 2.0
 * ("GPL"), unless you have obtained a separate licensing agreement
 * ("Other License"), formally executed by you and Linden Lab.  Terms of
 * the GPL can be found in doc/GPL-license.txt in this distribution, or
 * online at http://secondlifegrid.net/programs/open_source/licensing/gplv2
 * 
 * There are special exceptions to the terms and conditions of the GPL as
 * it is applied to this Source Code. View the full text of the exception
 * in the file doc/FLOSS-exception.txt in this software distribution, or
 * online at
 * http://secondlifegrid.net/programs/open_source/licensing/flossexception
 * 
 * By copying, modifying or distributing this software, you acknowledge
 * that you have read and understood your obligations described above,
 * and agree to abide by those obligations.
 * 
 * ALL LINDEN LAB SOURCE CODE IS PROVIDED "AS IS." LINDEN LAB MAKES NO
 * WARRANTIES, EXPRESS, IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY,
 * COMPLETENESS OR PERFORMANCE.
 * $/LicenseInfo$
 */

#ifndef LL_LLVIEWQUERY_H
#define LL_LLVIEWQUERY_H

#include <list>	

#include "llsingleton.h"
#include "llui.h"

class LLView;

typedef std::list<LLView *>			viewList_t;
typedef std::pair<BOOL, BOOL>		filterResult_t;

// Abstract base class for all query filters.
class LLQueryFilter
{
public:
	virtual ~LLQueryFilter() {};
	virtual filterResult_t operator() (const LLView* const view, const viewList_t & children) const = 0;
};

class LLQuerySorter
{
public:
	virtual ~LLQuerySorter() {};
	virtual void operator() (LLView * parent, viewList_t &children) const;
};

class LLLeavesFilter : public LLQueryFilter, public LLSingleton<LLLeavesFilter>
{
	LLSINGLETON_EMPTY_CTOR(LLLeavesFilter);
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override;
};

class LLRootsFilter : public LLQueryFilter, public LLSingleton<LLRootsFilter>
{
	LLSINGLETON_EMPTY_CTOR(LLRootsFilter);
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override;
};

class LLVisibleFilter : public LLQueryFilter, public LLSingleton<LLVisibleFilter>
{
	LLSINGLETON_EMPTY_CTOR(LLVisibleFilter);
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override;
};

class LLEnabledFilter : public LLQueryFilter, public LLSingleton<LLEnabledFilter>
{
	LLSINGLETON_EMPTY_CTOR(LLEnabledFilter);
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override;
};

class LLTabStopFilter : public LLQueryFilter, public LLSingleton<LLTabStopFilter>
{
	LLSINGLETON_EMPTY_CTOR(LLTabStopFilter);
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override;
};

class LLCtrlFilter : public LLQueryFilter, public LLSingleton<LLCtrlFilter>
{
	LLSINGLETON_EMPTY_CTOR(LLCtrlFilter);
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override;
};

template <class T>
class LLWidgetTypeFilter : public LLQueryFilter
{
	/*virtual*/ filterResult_t operator() (const LLView* const view, const viewList_t & children) const override
	{
		return filterResult_t(dynamic_cast<const T*>(view) != nullptr, TRUE);
	}

};

// Algorithm for flattening
class LLViewQuery
{
public:
	typedef std::list<const LLQueryFilter*>		filterList_t;
	typedef filterList_t::iterator				filterList_iter_t;
	typedef filterList_t::const_iterator		filterList_const_iter_t;

	LLViewQuery() : mPreFilters(), mPostFilters(), mSorterp() {}
	virtual ~LLViewQuery() {}

	void addPreFilter(const LLQueryFilter* prefilter) { mPreFilters.push_back(prefilter); }
	void addPostFilter(const LLQueryFilter* postfilter) { mPostFilters.push_back(postfilter); }
	const filterList_t & getPreFilters() const { return mPreFilters; }
	const filterList_t & getPostFilters() const { return mPostFilters; }

	void setSorter(const LLQuerySorter* sorter) { mSorterp = sorter; }
	const LLQuerySorter* getSorter() const { return mSorterp; }

	viewList_t run(LLView * view) const;
	// syntactic sugar
	viewList_t operator () (LLView * view) const { return run(view); }

	// override this method to provide iteration over other types of children
	virtual void filterChildren(LLView * view, viewList_t& filtered_children) const;

private:

	filterResult_t runFilters(LLView* view, viewList_t const& children, filterList_t const& filters) const;

	filterList_t mPreFilters;
	filterList_t mPostFilters;
	const LLQuerySorter* mSorterp;
};

class LLCtrlQuery : public LLViewQuery
{
public:
	LLCtrlQuery();
};

#endif // LL_LLVIEWQUERY_H
