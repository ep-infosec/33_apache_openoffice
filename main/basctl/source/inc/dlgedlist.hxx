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



#ifndef _BASCTL_DLGEDLIST_HXX
#define _BASCTL_DLGEDLIST_HXX

#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#include <com/sun/star/container/XContainerListener.hpp>

class DlgEdObj;

//============================================================================
// DlgEdPropListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::beans::XPropertyChangeListener > PropertyChangeListenerHelper;

class DlgEdPropListenerImpl: public PropertyChangeListenerHelper
{
private:
	DlgEdObj*		pDlgEdObj;

public:
	DlgEdPropListenerImpl(DlgEdObj* pObj);
	virtual ~DlgEdPropListenerImpl();

	// XEventListener
	virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

	// XPropertyChangeListener
	virtual void SAL_CALL propertyChange( const  ::com::sun::star::beans::PropertyChangeEvent& evt ) throw(::com::sun::star::uno::RuntimeException);

};

//============================================================================
// DlgEdEvtContListenerImpl
//============================================================================

typedef ::cppu::WeakImplHelper1< ::com::sun::star::container::XContainerListener > ContainerListenerHelper;

class DlgEdEvtContListenerImpl: public ContainerListenerHelper
{
private:
	DlgEdObj*		pDlgEdObj;

public:
	DlgEdEvtContListenerImpl(DlgEdObj* pObj);
	virtual ~DlgEdEvtContListenerImpl();

	// XEventListener
	virtual void SAL_CALL disposing( const  ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

	// XContainerListener
	virtual void SAL_CALL elementInserted( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL elementReplaced( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
	virtual void SAL_CALL elementRemoved( const ::com::sun::star::container::ContainerEvent& Event ) throw(::com::sun::star::uno::RuntimeException);
};

#endif // _BASCTL_DLGEDLIST_HXX
