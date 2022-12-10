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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//	my own includes

#include <classes/propertysethelper.hxx>
#include <threadhelp/transactionguard.hxx>
#include <threadhelp/readguard.hxx>
#include <threadhelp/writeguard.hxx>

//_________________________________________________________________________________________________________________
//	interface includes

//_________________________________________________________________________________________________________________
//	other includes

//_________________________________________________________________________________________________________________
//	namespace

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported definitions

//-----------------------------------------------------------------------------
PropertySetHelper::PropertySetHelper(const css::uno::Reference< css::lang::XMultiServiceFactory >& xSMGR                       ,
                                           LockHelper*                                             pExternalLock               ,
                                           TransactionManager*                                     pExternalTransactionManager ,
                                           sal_Bool                                                bReleaseLockOnCall          )
    : m_xSMGR                (xSMGR                                )
    , m_lSimpleChangeListener(pExternalLock->getShareableOslMutex())
    , m_lVetoChangeListener  (pExternalLock->getShareableOslMutex())
    , m_bReleaseLockOnCall   (bReleaseLockOnCall                   )
    , m_rLock                (*pExternalLock                       )
    , m_rTransactionManager  (*pExternalTransactionManager         )
{
}

//-----------------------------------------------------------------------------
PropertySetHelper::~PropertySetHelper()
{
}

//-----------------------------------------------------------------------------
void PropertySetHelper::impl_setPropertyChangeBroadcaster(const css::uno::Reference< css::uno::XInterface >& xBroadcaster)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_rLock);
    m_xBroadcaster = xBroadcaster;
    aWriteLock.unlock();
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_addPropertyInfo(const css::beans::Property& aProperty)
    throw(css::beans::PropertyExistException,
          css::uno::Exception               )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(aProperty.Name);
    if (pIt != m_lProps.end())
        throw css::beans::PropertyExistException();

    m_lProps[aProperty.Name] = aProperty;
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_removePropertyInfo(const ::rtl::OUString& sProperty)
    throw(css::beans::UnknownPropertyException,
          css::uno::Exception                 )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_rLock);

    PropertySetHelper::TPropInfoHash::iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    m_lProps.erase(pIt);
    // <- SAFE
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_enablePropertySet()
{
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::impl_disablePropertySet()
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_rLock);

    css::uno::Reference< css::uno::XInterface > xThis(static_cast< css::beans::XPropertySet* >(this), css::uno::UNO_QUERY);
    css::lang::EventObject aEvent(xThis);

    m_lSimpleChangeListener.disposeAndClear(aEvent);
    m_lVetoChangeListener.disposeAndClear(aEvent);
    m_lProps.free();

    aWriteLock.unlock();
    // <- SAFE
}

//-----------------------------------------------------------------------------
sal_Bool PropertySetHelper::impl_existsVeto(const css::beans::PropertyChangeEvent& aEvent)
{
    /*  Dont use the lock here!
        The used helper is threadsafe and it lives for the whole lifetime of
        our own object.
    */
    ::cppu::OInterfaceContainerHelper* pVetoListener = m_lVetoChangeListener.getContainer(aEvent.PropertyName);
	if (! pVetoListener)
        return sal_False;

	::cppu::OInterfaceIteratorHelper pListener(*pVetoListener);
	while (pListener.hasMoreElements())
	{
        try
        {
            css::uno::Reference< css::beans::XVetoableChangeListener > xListener(
                ((css::beans::XVetoableChangeListener*)pListener.next()),
                css::uno::UNO_QUERY_THROW);
            xListener->vetoableChange(aEvent);
        }
        catch(const css::uno::RuntimeException&)
            { pListener.remove(); }
        catch(const css::beans::PropertyVetoException&)
            { return sal_True; }
	}

    return sal_False;
}

//-----------------------------------------------------------------------------
void PropertySetHelper::impl_notifyChangeListener(const css::beans::PropertyChangeEvent& aEvent)
{
    /*  Dont use the lock here!
        The used helper is threadsafe and it lives for the whole lifetime of
        our own object.
    */
    ::cppu::OInterfaceContainerHelper* pSimpleListener = m_lSimpleChangeListener.getContainer(aEvent.PropertyName);
	if (! pSimpleListener)
        return;

	::cppu::OInterfaceIteratorHelper pListener(*pSimpleListener);
	while (pListener.hasMoreElements())
	{
        try
        {
            css::uno::Reference< css::beans::XPropertyChangeListener > xListener(
                ((css::beans::XVetoableChangeListener*)pListener.next()),
                css::uno::UNO_QUERY_THROW);
            xListener->propertyChange(aEvent);
        }
        catch(const css::uno::RuntimeException&)
            { pListener.remove(); }
	}
}

//-----------------------------------------------------------------------------
css::uno::Reference< css::beans::XPropertySetInfo > SAL_CALL PropertySetHelper::getPropertySetInfo()
    throw(css::uno::RuntimeException)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    css::uno::Reference< css::beans::XPropertySetInfo > xInfo(static_cast< css::beans::XPropertySetInfo* >(this), css::uno::UNO_QUERY_THROW);
    return xInfo;
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::setPropertyValue(const ::rtl::OUString& sProperty,
                                                  const css::uno::Any&   aValue   )
    throw(css::beans::UnknownPropertyException,
          css::beans::PropertyVetoException   ,
          css::lang::IllegalArgumentException ,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    // TODO look for e.g. readonly props and reject setProp() call!

    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    WriteGuard aWriteLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    css::beans::Property aPropInfo = pIt->second;

    sal_Bool bLocked = sal_True;
    if (m_bReleaseLockOnCall)
    {
        aWriteLock.unlock();
        bLocked = sal_False;
        // <- SAFE
    }

    css::uno::Any aCurrentValue = impl_getPropertyValue(aPropInfo.Name, aPropInfo.Handle);

    if (! bLocked)
    {
        // SAFE ->
        aWriteLock.lock();
        bLocked = sal_True;
    }

    sal_Bool bWillBeChanged = (aCurrentValue != aValue);
    if (! bWillBeChanged)
        return;

    css::beans::PropertyChangeEvent aEvent;
    aEvent.PropertyName   = aPropInfo.Name;
    aEvent.Further        = sal_False;
    aEvent.PropertyHandle = aPropInfo.Handle;
    aEvent.OldValue       = aCurrentValue;
    aEvent.NewValue       = aValue;
    aEvent.Source         = css::uno::Reference< css::uno::XInterface >(m_xBroadcaster.get(), css::uno::UNO_QUERY);

    if (m_bReleaseLockOnCall)
    {
        aWriteLock.unlock();
        bLocked = sal_False;
        // <- SAFE
    }

    if (impl_existsVeto(aEvent))
        throw css::beans::PropertyVetoException();

    impl_setPropertyValue(aPropInfo.Name, aPropInfo.Handle, aValue);

    impl_notifyChangeListener(aEvent);
}

//-----------------------------------------------------------------------------
css::uno::Any SAL_CALL PropertySetHelper::getPropertyValue(const ::rtl::OUString& sProperty)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    css::beans::Property aPropInfo = pIt->second;

    sal_Bool bLocked = sal_True;
    if (m_bReleaseLockOnCall)
    {
        aReadLock.unlock();
        bLocked = sal_False;
        // <- SAFE
    }

    return impl_getPropertyValue(aPropInfo.Name, aPropInfo.Handle);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::addPropertyChangeListener(const ::rtl::OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lSimpleChangeListener.addInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::removePropertyChangeListener(const ::rtl::OUString&                                            sProperty,
                                                              const css::uno::Reference< css::beans::XPropertyChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lSimpleChangeListener.removeInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::addVetoableChangeListener(const ::rtl::OUString&                                            sProperty,
                                                           const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lVetoChangeListener.addInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
void SAL_CALL PropertySetHelper::removeVetoableChangeListener(const ::rtl::OUString&                                            sProperty,
                                                              const css::uno::Reference< css::beans::XVetoableChangeListener >& xListener)
    throw(css::beans::UnknownPropertyException,
          css::lang::WrappedTargetException   ,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_SOFTEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sProperty);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    aReadLock.unlock();
    // <- SAFE

    m_lVetoChangeListener.removeInterface(sProperty, xListener);
}

//-----------------------------------------------------------------------------
css::uno::Sequence< css::beans::Property > SAL_CALL PropertySetHelper::getProperties()
    throw(css::uno::RuntimeException)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    sal_Int32                                        c     = (sal_Int32)m_lProps.size();
    css::uno::Sequence< css::beans::Property >       lProps(c);
    PropertySetHelper::TPropInfoHash::const_iterator pIt   ;

    for (  pIt  = m_lProps.begin();
           pIt != m_lProps.end()  ;
         ++pIt                    )
    {
        lProps[--c] = pIt->second;
    }

    return lProps;
    // <- SAFE
}

//-----------------------------------------------------------------------------
css::beans::Property SAL_CALL PropertySetHelper::getPropertyByName(const ::rtl::OUString& sName)
    throw(css::beans::UnknownPropertyException,
          css::uno::RuntimeException          )
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::const_iterator pIt = m_lProps.find(sName);
    if (pIt == m_lProps.end())
        throw css::beans::UnknownPropertyException();

    return pIt->second;
    // <- SAFE
}

//-----------------------------------------------------------------------------
sal_Bool SAL_CALL PropertySetHelper::hasPropertyByName(const ::rtl::OUString& sName)
    throw(css::uno::RuntimeException)
{
    TransactionGuard aTransaction(m_rTransactionManager, E_HARDEXCEPTIONS);

    // SAFE ->
    ReadGuard aReadLock(m_rLock);

    PropertySetHelper::TPropInfoHash::iterator pIt    = m_lProps.find(sName);
    sal_Bool                                   bExist = (pIt != m_lProps.end());

    return bExist;
    // <- SAFE
}

} // namespace framework
