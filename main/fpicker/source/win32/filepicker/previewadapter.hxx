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



#ifndef _PREVIEWADAPTER_HXX_
#define _PREVIEWADAPTER_HXX_

#include <sal/types.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/uno/Any.hxx>

#if defined _MSC_VER
#pragma warning(push, 1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif
#include <memory>

// forward declaration
class CPreviewAdapterImpl;

//---------------------------------------------
// A kind of a facade for the preview class.
// We want to hide the fact that the preview
// window may only become visible if there is
// a valid parent window (means, the FilePicker)
// is in execution mode. So unless someone sets
// the preview active with a valid parent 
// window the preview may not be visible
//---------------------------------------------

class CPreviewAdapter
{
public:
	
	// ctor
	CPreviewAdapter(HINSTANCE instance);
	
	~CPreviewAdapter();

	::com::sun::star::uno::Sequence<sal_Int16> SAL_CALL getSupportedImageFormats();

    sal_Int32 SAL_CALL getTargetColorDepth();

    sal_Int32 SAL_CALL getAvailableWidth();

    sal_Int32 SAL_CALL getAvailableHeight();

    void SAL_CALL setImage(sal_Int16 aImageFormat, const ::com::sun::star::uno::Any& aImage) 
		throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException);

    sal_Bool SAL_CALL setShowState(sal_Bool bShowState);

    sal_Bool SAL_CALL getShowState();		
	
	void SAL_CALL setParent(HWND parent);

	//--------------------------------------
	// notification from parent
	//--------------------------------------

	void SAL_CALL notifyParentShow(bool bShow);
	
	void SAL_CALL notifyParentSizeChanged();
	
	void SAL_CALL notifyParentWindowPosChanged();

private:
	// hide implementation details using the
	// bridge pattern
	std::auto_ptr<CPreviewAdapterImpl> m_pImpl;

// prevent copy and assignment
private:
    CPreviewAdapter(const CPreviewAdapter&);
    CPreviewAdapter& operator=(const CPreviewAdapter&);
};


#endif
