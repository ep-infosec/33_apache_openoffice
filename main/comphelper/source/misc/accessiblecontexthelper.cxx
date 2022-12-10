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
#include "precompiled_comphelper.hxx"
#include <comphelper/accessiblecontexthelper.hxx>
#include <comphelper/accessibleeventbuffer.hxx>
#include <osl/diagnose.h>
#include <cppuhelper/weakref.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <comphelper/accessibleeventnotifier.hxx>

//.........................................................................
namespace comphelper
{
//.........................................................................

	using namespace ::com::sun::star::uno;
	using namespace ::com::sun::star::lang;
	using namespace ::com::sun::star::accessibility;

	//=====================================================================
	//= OContextHelper_Impl
	//=====================================================================
	/** implementation class for OAccessibleContextHelper. No own thread safety!
	*/
	class OContextHelper_Impl
	{
	private:
		OAccessibleContextHelper*			m_pAntiImpl;		// the owning instance
		IMutex*								m_pExternalLock;	// the optional additional external lock

	    ::cppu::OInterfaceContainerHelper*	m_pEventListeners;
		WeakReference< XAccessible >		m_aCreator;			// the XAccessible which created our XAccessibleContext

		AccessibleEventNotifier::TClientId	m_nClientId;

	public:
		inline	Reference< XAccessible >	getCreator( ) const					{ return m_aCreator; }
		inline	void						setCreator( const Reference< XAccessible >& _rAcc );

		inline	IMutex*						getExternalLock( )					{ return m_pExternalLock; }
		inline	void						setExternalLock( IMutex* _pLock )	{ m_pExternalLock = _pLock; }

		inline	AccessibleEventNotifier::TClientId
											getClientId() const					{ return m_nClientId; }
		inline	void						setClientId( const AccessibleEventNotifier::TClientId _nId )
																				{ m_nClientId = _nId; }

	public:
		OContextHelper_Impl( OAccessibleContextHelper* _pAntiImpl )
			:m_pAntiImpl( _pAntiImpl )
			,m_pExternalLock( NULL )
			,m_pEventListeners( NULL )
			,m_nClientId( 0 )
		{
		}
	};

	//---------------------------------------------------------------------
	inline	void OContextHelper_Impl::setCreator( const Reference< XAccessible >& _rAcc )
	{
		m_aCreator = _rAcc;
	}

	//=====================================================================
	//= OAccessibleContextHelper
	//=====================================================================
	//---------------------------------------------------------------------
	OAccessibleContextHelper::OAccessibleContextHelper( )
		:OAccessibleContextHelper_Base( GetMutex() )
		,m_pImpl( NULL )
	{
		m_pImpl = new OContextHelper_Impl( this );
	}

	//---------------------------------------------------------------------
	OAccessibleContextHelper::OAccessibleContextHelper( IMutex* _pExternalLock )
		:OAccessibleContextHelper_Base( GetMutex() )
		,m_pImpl( NULL )
	{
		m_pImpl = new OContextHelper_Impl( this );
		m_pImpl->setExternalLock( _pExternalLock );
	}

	//---------------------------------------------------------------------
	void OAccessibleContextHelper::forgetExternalLock()
	{
		m_pImpl->setExternalLock( NULL );
	}

	//---------------------------------------------------------------------
	OAccessibleContextHelper::~OAccessibleContextHelper( )
	{
		forgetExternalLock();
			// this ensures that the lock, which may be already destroyed as part of the derivee,
			// is not used anymore

		ensureDisposed();

		delete m_pImpl;
		m_pImpl = NULL;
	}

	//---------------------------------------------------------------------
	IMutex* OAccessibleContextHelper::getExternalLock( )
	{
		return m_pImpl->getExternalLock();
	}

	//---------------------------------------------------------------------
	void SAL_CALL OAccessibleContextHelper::disposing()
	{
		::osl::ClearableMutexGuard aGuard( GetMutex() );

		if ( m_pImpl->getClientId( ) )
		{
			AccessibleEventNotifier::revokeClientNotifyDisposing( m_pImpl->getClientId( ), *this );
			m_pImpl->setClientId( 0 );
		}
	}

	//---------------------------------------------------------------------
	void SAL_CALL OAccessibleContextHelper::addEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
	{
		OMutexGuard aGuard( getExternalLock() );
			// don't use the OContextEntryGuard - it will throw an exception if we're not alive
			// anymore, while the most recent specification for XComponent states that we should
			// silently ignore the call in such a situation
		if ( !isAlive() )
		{
			if ( _rxListener.is() )
				_rxListener->disposing( EventObject( *this ) );
			return;
		}

		if ( _rxListener.is() )
		{
			if ( !m_pImpl->getClientId( ) )
				m_pImpl->setClientId( AccessibleEventNotifier::registerClient( ) );

			AccessibleEventNotifier::addEventListener( m_pImpl->getClientId( ), _rxListener );
		}
	}

	//---------------------------------------------------------------------
	void SAL_CALL OAccessibleContextHelper::removeEventListener( const Reference< XAccessibleEventListener >& _rxListener ) throw (RuntimeException)
	{
		OMutexGuard aGuard( getExternalLock() );
			// don't use the OContextEntryGuard - it will throw an exception if we're not alive
			// anymore, while the most recent specification for XComponent states that we should
			// silently ignore the call in such a situation
		if ( !isAlive() )
			return;

		if ( _rxListener.is() )
		{
			sal_Int32 nListenerCount = AccessibleEventNotifier::removeEventListener( m_pImpl->getClientId( ), _rxListener );
			if ( !nListenerCount )
			{
				// no listeners anymore
				// -> revoke ourself. This may lead to the notifier thread dying (if we were the last client),
				// and at least to us not firing any events anymore, in case somebody calls
				// NotifyAccessibleEvent, again
				AccessibleEventNotifier::revokeClient( m_pImpl->getClientId( ) );
				m_pImpl->setClientId( 0 );
			}
		}
	}

	//---------------------------------------------------------------------
	void SAL_CALL OAccessibleContextHelper::NotifyAccessibleEvent( const sal_Int16 _nEventId,
		const Any& _rOldValue, const Any& _rNewValue )
	{
		if ( !m_pImpl->getClientId( ) )
			// if we don't have a client id for the notifier, then we don't have listeners, then
			// we don't need to notify anything
			return;

		// build an event object
		AccessibleEventObject aEvent;
		aEvent.Source = *this;
		aEvent.EventId = _nEventId;
		aEvent.OldValue = _rOldValue;
		aEvent.NewValue = _rNewValue;

		// let the notifier handle this event
		AccessibleEventNotifier::addEvent( m_pImpl->getClientId( ), aEvent );
	}

	//---------------------------------------------------------------------
	void SAL_CALL OAccessibleContextHelper::BufferAccessibleEvent( const sal_Int16 _nEventId,
		const Any& _rOldValue, const Any& _rNewValue,
        AccessibleEventBuffer & _rBuffer )
	{
		// TODO: this whole method (as well as the class AccessibleEventBuffer) should be removed
		// The reasons why they have been introduces id that we needed to collect a set of events
		// before notifying them altogether (after releasing our mutex). With the other
		// NotifyAccessibleEvent being asynchronous now, this should not be necessary anymore
		// - clients could use the other version now.

		// copy our current listeners
		Sequence< Reference< XInterface > > aListeners;
		if ( m_pImpl->getClientId( ) )
			aListeners = AccessibleEventNotifier::getEventListeners( m_pImpl->getClientId( ) );

		if ( aListeners.getLength() )
		{
			AccessibleEventObject aEvent;
			aEvent.Source = *this;
			OSL_ENSURE( aEvent.Source.is(), "OAccessibleContextHelper::BufferAccessibleEvent: invalid creator!" );
			aEvent.EventId = _nEventId;
			aEvent.OldValue = _rOldValue;
			aEvent.NewValue = _rNewValue;

            _rBuffer.addEvent( aEvent, aListeners );
		}
	}

	//---------------------------------------------------------------------
	sal_Bool OAccessibleContextHelper::isAlive() const
	{
		return !GetBroadcastHelper().bDisposed && !GetBroadcastHelper().bInDispose;
	}

	//---------------------------------------------------------------------
	void OAccessibleContextHelper::ensureAlive() const SAL_THROW( ( DisposedException ) )
	{
		if( !isAlive() )
			throw DisposedException();
	}

	//---------------------------------------------------------------------
	void OAccessibleContextHelper::ensureDisposed( )
	{
		if ( !GetBroadcastHelper().bDisposed )
		{
			OSL_ENSURE( 0 == m_refCount, "OAccessibleContextHelper::ensureDisposed: this method _has_ to be called from without your dtor only!" );
			acquire();
			dispose();
		}
	}

	//---------------------------------------------------------------------
	void OAccessibleContextHelper::lateInit( const Reference< XAccessible >& _rxAccessible )
	{
		m_pImpl->setCreator( _rxAccessible );
	}

	//---------------------------------------------------------------------
	Reference< XAccessible > OAccessibleContextHelper::getAccessibleCreator( ) const
	{
		return m_pImpl->getCreator();
	}

	//---------------------------------------------------------------------
	sal_Int32 SAL_CALL OAccessibleContextHelper::getAccessibleIndexInParent(  ) throw (RuntimeException)
	{
		OExternalLockGuard aGuard( this );

		// -1 for child not found/no parent (according to specification)
		sal_Int32 nRet = -1;

		try
		{
			
			Reference< XAccessibleContext > xParentContext( implGetParentContext() );

			//  iterate over parent's children and search for this object
			if ( xParentContext.is() )
			{
				// our own XAccessible for comparing with the children of our parent
				Reference< XAccessible > xCreator( m_pImpl->getCreator() );

				OSL_ENSURE( xCreator.is(), "OAccessibleContextHelper::getAccessibleIndexInParent: invalid creator!" );
					// two ideas why this could be NULL:
					// * nobody called our late ctor (init), so we never had a creator at all -> bad
					// * the creator is already dead. In this case, we should have been disposed, and
					//	 never survived the above OContextEntryGuard.
					// in all other situations the creator should be non-NULL

				if ( xCreator.is() )
				{
					sal_Int32 nChildCount = xParentContext->getAccessibleChildCount();
					for ( sal_Int32 nChild = 0; ( nChild < nChildCount ) && ( -1 == nRet ); ++nChild )
					{
						Reference< XAccessible > xChild( xParentContext->getAccessibleChild( nChild ) );
						if ( xChild.get() == xCreator.get() )
							nRet = nChild;
					}
				}
			}
		}
		catch( const Exception& )
		{
			OSL_ENSURE( sal_False, "OAccessibleContextHelper::getAccessibleIndexInParent: caught an exception!" );
		}

		return nRet;
	}

	//---------------------------------------------------------------------
	Locale SAL_CALL OAccessibleContextHelper::getLocale(  ) throw (IllegalAccessibleComponentStateException, RuntimeException)
	{
		// simply ask the parent
		Reference< XAccessible > xParent = getAccessibleParent();
		Reference< XAccessibleContext > xParentContext;
		if ( xParent.is() )
			xParentContext = xParent->getAccessibleContext();

		if ( !xParentContext.is() )
			throw IllegalAccessibleComponentStateException( ::rtl::OUString(), *this );

		return xParentContext->getLocale();
	}

	//---------------------------------------------------------------------
	Reference< XAccessibleContext > OAccessibleContextHelper::implGetParentContext() SAL_THROW( ( RuntimeException ) )
	{
		Reference< XAccessible > xParent = getAccessibleParent();
		Reference< XAccessibleContext > xParentContext;
		if ( xParent.is() )
			xParentContext = xParent->getAccessibleContext();
		return xParentContext;
	}

//.........................................................................
}	// namespace comphelper
//.........................................................................


