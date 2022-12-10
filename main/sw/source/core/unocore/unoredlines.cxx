/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/beans/XPropertySet.hpp>

#include <tools/debug.hxx>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>

#include <unoredlines.hxx>
#include <unoredline.hxx>
#include <unomid.h>
#include <pagedesc.hxx>
#include "poolfmt.hxx"
#include <doc.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <switerator.hxx>

using namespace ::com::sun::star;
using ::rtl::OUString;

SwXRedlines::SwXRedlines(SwDoc* _pDoc) :
	SwUnoCollection(_pDoc)
{
}

SwXRedlines::~SwXRedlines()
{
}

sal_Int32 SwXRedlines::getCount(  ) throw(uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());
	if(!IsValid())
		throw uno::RuntimeException();
	const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
	return rRedTbl.Count();
}

uno::Any SwXRedlines::getByIndex(sal_Int32 nIndex)
	throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
	vos::OGuard aGuard(Application::GetSolarMutex());
	if(!IsValid())
		throw uno::RuntimeException();
	const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
	uno::Any aRet;
	if(rRedTbl.Count() > nIndex && nIndex >= 0)
	{
		uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject((sal_uInt16)nIndex), *GetDoc() );
		aRet <<= xRet;
	}
	else
		throw lang::IndexOutOfBoundsException();
	return aRet;
}

uno::Reference< container::XEnumeration >  SwXRedlines::createEnumeration(void)
	throw( uno::RuntimeException )
{
	vos::OGuard aGuard(Application::GetSolarMutex());
	if(!IsValid())
		throw uno::RuntimeException();
	return uno::Reference< container::XEnumeration >(new SwXRedlineEnumeration(*GetDoc()));
}

uno::Type SwXRedlines::getElementType(  ) throw(uno::RuntimeException)
{
	return ::getCppuType((uno::Reference<beans::XPropertySet>*)0);
}

sal_Bool SwXRedlines::hasElements(  ) throw(uno::RuntimeException)
{
	vos::OGuard aGuard(Application::GetSolarMutex());
	if(!IsValid())
		throw uno::RuntimeException();
	const SwRedlineTbl& rRedTbl = GetDoc()->GetRedlineTbl();
	return rRedTbl.Count() > 0;
}

OUString SwXRedlines::getImplementationName(void) throw( uno::RuntimeException )
{
	return C2U("SwXRedlines");
}

sal_Bool SwXRedlines::supportsService(const rtl::OUString& /*ServiceName*/)
	throw( uno::RuntimeException )
{
	DBG_ERROR("not implemented");
	return sal_False;
}

uno::Sequence< OUString > SwXRedlines::getSupportedServiceNames(void)
	throw( uno::RuntimeException )
{
	DBG_ERROR("not implemented");
	return uno::Sequence< OUString >();
}

beans::XPropertySet* 	SwXRedlines::GetObject( SwRedline& rRedline, SwDoc& rDoc )
{
	SwPageDesc* pStdDesc = rDoc.GetPageDescFromPool(RES_POOLPAGE_STANDARD);
	SwIterator<SwXRedline,SwPageDesc> aIter(*pStdDesc);
	SwXRedline* pxRedline = aIter.First();
	while(pxRedline)
	{
		if(pxRedline->GetRedline() == &rRedline)
			break;
		pxRedline = aIter.Next();
	}
	if( !pxRedline )
		pxRedline = new SwXRedline(rRedline, rDoc);
	return pxRedline;
}

SwXRedlineEnumeration::SwXRedlineEnumeration(SwDoc& rDoc) :
	pDoc(&rDoc),
	nCurrentIndex(0)
{
	pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
}

SwXRedlineEnumeration::~SwXRedlineEnumeration()
{
}

sal_Bool SwXRedlineEnumeration::hasMoreElements(void) throw( uno::RuntimeException )
{
	if(!pDoc)
		throw uno::RuntimeException();
	return pDoc->GetRedlineTbl().Count() > nCurrentIndex;
}

uno::Any SwXRedlineEnumeration::nextElement(void)
	throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
	if(!pDoc)
		throw uno::RuntimeException();
	const SwRedlineTbl& rRedTbl = pDoc->GetRedlineTbl();
	if(!(rRedTbl.Count() > nCurrentIndex))
		throw container::NoSuchElementException();
	uno::Reference <beans::XPropertySet> xRet = SwXRedlines::GetObject( *rRedTbl.GetObject(nCurrentIndex++), *pDoc );
	uno::Any aRet;
	aRet <<= xRet;
	return aRet;
}

rtl::OUString SwXRedlineEnumeration::getImplementationName(void) throw( uno::RuntimeException )
{
	return C2U("SwXRedlineEnumeration");
}

sal_Bool SwXRedlineEnumeration::supportsService(const rtl::OUString& /*ServiceName*/) throw( uno::RuntimeException )
{
	return sal_False;
}

uno::Sequence< OUString > SwXRedlineEnumeration::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
	return uno::Sequence< OUString >();
}

void SwXRedlineEnumeration::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
	ClientModify(this, pOld, pNew);
	if(!GetRegisteredIn())
		pDoc = 0;
}
