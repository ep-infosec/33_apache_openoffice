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
#include "precompiled_svx.hxx"

#include <accessiblecell.hxx>

#include "svx/DescriptionGenerator.hxx"

#include <com/sun/star/accessibility/AccessibleRole.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>

#include <vcl/svapp.hxx>

#include <unotools/accessiblestatesethelper.hxx>

#include <editeng/outlobj.hxx>
#include <svx/unoshtxt.hxx>
#include <svx/svdotext.hxx>

using ::rtl::OUString;
using namespace ::sdr::table;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace	::com::sun::star::accessibility;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;

namespace accessibility {

// --------------------------------------------------------------------
// AccessibleCell
// --------------------------------------------------------------------

AccessibleCell::AccessibleCell( const ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible>& rxParent, const sdr::table::CellRef& rCell, sal_Int32 nIndex, const AccessibleShapeTreeInfo& rShapeTreeInfo )
: AccessibleCellBase( rxParent, AccessibleRole::TABLE_CELL )
, maShapeTreeInfo( rShapeTreeInfo )
, mnIndexInParent( nIndex )
, mpText( NULL )
, mxCell( rCell )
{
	//Init the pAccTable var
	pAccTable = dynamic_cast <AccessibleTableShape *> (rxParent.get());
}

// --------------------------------------------------------------------

AccessibleCell::~AccessibleCell (void)
{
	DBG_ASSERT( mpText == 0, "svx::AccessibleCell::~AccessibleCell(), not disposed!?" );
}

// --------------------------------------------------------------------

void AccessibleCell::Init (void)
{
	SdrView* pView = maShapeTreeInfo.GetSdrView();
	const Window* pWindow = maShapeTreeInfo.GetWindow ();
	if( (pView != NULL) && (pWindow != NULL) && mxCell.is())
	{
        OutlinerParaObject* pOutlinerParaObject = mxCell->GetEditOutlinerParaObject(); // Get the OutlinerParaObject if text edit is active

        bool bOwnParaObject = pOutlinerParaObject != 0;

		if( !pOutlinerParaObject )
			pOutlinerParaObject = mxCell->GetOutlinerParaObject();

		// create AccessibleTextHelper to handle this shape's text
        if( pOutlinerParaObject )
        {
            // non-empty text -> use full-fledged edit source right away
            ::std::auto_ptr<SvxEditSource> pEditSource( new SvxTextEditSource( mxCell->GetObject(), mxCell.get(), *pView, *pWindow) );
            mpText = new AccessibleTextHelper( pEditSource );
			mpText->SetEventSource(this);
        }

        if( bOwnParaObject)
            delete pOutlinerParaObject;
    }
}

// --------------------------------------------------------------------

sal_Bool AccessibleCell::SetState (sal_Int16 aState)
{
    sal_Bool bStateHasChanged = sal_False;

    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        // Offer FOCUSED state to edit engine and detect whether the state
        // changes.
        sal_Bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus (sal_True);
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::SetState (aState);

    return bStateHasChanged;
}

// --------------------------------------------------------------------

sal_Bool AccessibleCell::ResetState (sal_Int16 aState)
{
    sal_Bool bStateHasChanged = sal_False;

    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        // Try to remove FOCUSED state from the edit engine and detect
        // whether the state changes.
        sal_Bool bIsFocused = mpText->HaveFocus ();
        mpText->SetFocus (sal_False);
        bStateHasChanged = (bIsFocused != mpText->HaveFocus ());
    }
    else
        bStateHasChanged = AccessibleContextBase::ResetState (aState);

    return bStateHasChanged;
}

// --------------------------------------------------------------------

sal_Bool AccessibleCell::GetState (sal_Int16 aState)
{
    if (aState == AccessibleStateType::FOCUSED && mpText != NULL)
    {
        // Just delegate the call to the edit engine.  The state is not
        // merged into the state set.
        return mpText->HaveFocus();
    }
    else
        return AccessibleContextBase::GetState (aState);
}

//-----------------------------------------------------------------------------

bool AccessibleCell::operator== (const AccessibleCell& rAccessibleCell)
{
	return this == &rAccessibleCell;
}

//-----------------------------------------------------------------------------
// XInterface
//-----------------------------------------------------------------------------

Any SAL_CALL AccessibleCell::queryInterface( const Type& aType ) throw (RuntimeException)
{
	return AccessibleCellBase::queryInterface( aType );
}

//-----------------------------------------------------------------------------

void SAL_CALL AccessibleCell::acquire(  ) throw ()
{
	AccessibleCellBase::acquire();
}

//-----------------------------------------------------------------------------

void SAL_CALL AccessibleCell::release(  ) throw ()
{
	AccessibleCellBase::release();
}

// --------------------------------------------------------------------
// XAccessibleContext
// --------------------------------------------------------------------

/** The children of this cell come from the paragraphs of text.
*/
sal_Int32 SAL_CALL AccessibleCell::getAccessibleChildCount() throw (::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ThrowIfDisposed ();
	return mpText != NULL ? mpText->GetChildCount () : 0;
}

// --------------------------------------------------------------------

/** Forward the request to the shape.  Return the requested shape or throw
    an exception for a wrong index.
*/
Reference<XAccessible> SAL_CALL AccessibleCell::getAccessibleChild (sal_Int32 nIndex) throw (IndexOutOfBoundsException, RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ThrowIfDisposed ();

	// todo: does GetChild throw IndexOutOfBoundsException?
	return mpText->GetChild (nIndex);
}

// --------------------------------------------------------------------

/**	Return a copy of the state set.
    Possible states are:
		ENABLED
		SHOWING
		VISIBLE
*/
Reference<XAccessibleStateSet> SAL_CALL AccessibleCell::getAccessibleStateSet (void) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ::osl::MutexGuard aGuard (maMutex);
    Reference<XAccessibleStateSet> xStateSet;

	if (rBHelper.bDisposed || mpText == NULL)
	{
        // Return a minimal state set that only contains the DEFUNC state.
        xStateSet = AccessibleContextBase::getAccessibleStateSet ();
	}
    else
    {
        ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());

        if(pStateSet)
        {
            // Merge current FOCUSED state from edit engine.
            if (mpText != NULL) 
			{
				if (mpText->HaveFocus())
                    pStateSet->AddState (AccessibleStateType::FOCUSED);
                else
                    pStateSet->RemoveState (AccessibleStateType::FOCUSED);
			}
			// Set the invisible state for merged cell 
			if (mxCell.is() && mxCell->isMerged())
				pStateSet->RemoveState(AccessibleStateType::VISIBLE);
			else
				pStateSet->AddState(AccessibleStateType::VISIBLE);


			//Solution:Just when the parent table is not read-only,set states EDITABLE,RESIZABLE,MOVEABLE
		    ::com::sun::star::uno::Reference<XAccessible> xTempAcc = getAccessibleParent();
		    if( xTempAcc.is() )
		    {
		    	::com::sun::star::uno::Reference<XAccessibleContext> 
		    							xTempAccContext = xTempAcc->getAccessibleContext();
		    	if( xTempAccContext.is() )
		    	{
		    		::com::sun::star::uno::Reference<XAccessibleStateSet> rState = 
		    			xTempAccContext->getAccessibleStateSet();
		    		if( rState.is() )    		{
						com::sun::star::uno::Sequence<short> pStates = rState->getStates();
						int count = pStates.getLength();
						for( int iIndex = 0;iIndex < count;iIndex++ )
						{
							if( pStates[iIndex] == AccessibleStateType::EDITABLE )
							{
								pStateSet->AddState (AccessibleStateType::EDITABLE);
							    pStateSet->AddState (AccessibleStateType::RESIZABLE);
							    pStateSet->AddState (AccessibleStateType::MOVEABLE);
								break;	
							}
						}
					}
				}
		    }
            // Create a copy of the state set that may be modified by the
            // caller without affecting the current state set.
            xStateSet = Reference<XAccessibleStateSet>(new ::utl::AccessibleStateSetHelper (*pStateSet));
        }
    }

    return xStateSet;
}

// --------------------------------------------------------------------
// XAccessibleComponent
// --------------------------------------------------------------------

sal_Bool SAL_CALL AccessibleCell::containsPoint( const ::com::sun::star::awt::Point& aPoint) throw (::com::sun::star::uno::RuntimeException)
{
	return AccessibleComponentBase::containsPoint( aPoint );
}

/** The implementation below is at the moment straightforward.  It iterates
    over all children (and thereby instances all children which have not
    been already instatiated) until a child covering the specifed point is
    found.
    This leaves room for improvement.  For instance, first iterate only over
    the already instantiated children and only if no match is found
    instantiate the remaining ones.
*/
Reference<XAccessible > SAL_CALL  AccessibleCell::getAccessibleAtPoint ( const ::com::sun::star::awt::Point& aPoint) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ::osl::MutexGuard aGuard (maMutex);

    sal_Int32 nChildCount = getAccessibleChildCount ();
    for (sal_Int32 i=0; i<nChildCount; ++i)
    {
        Reference<XAccessible> xChild (getAccessibleChild (i));
        if (xChild.is())
        {
            Reference<XAccessibleComponent> xChildComponent (xChild->getAccessibleContext(), uno::UNO_QUERY);
            if (xChildComponent.is())
            {
                awt::Rectangle aBBox (xChildComponent->getBounds());
                if ( (aPoint.X >= aBBox.X)
                    && (aPoint.Y >= aBBox.Y)
                    && (aPoint.X < aBBox.X+aBBox.Width)
                    && (aPoint.Y < aBBox.Y+aBBox.Height) )
                    return xChild;
            }
        }
    }

    // Have not found a child under the given point.  Returning empty
    // reference to indicate this.
    return uno::Reference<XAccessible>();
}

// --------------------------------------------------------------------

::com::sun::star::awt::Rectangle SAL_CALL AccessibleCell::getBounds(void) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ::osl::MutexGuard aGuard (maMutex);

    ThrowIfDisposed ();
	::com::sun::star::awt::Rectangle aBoundingBox;
	if( mxCell.is() )
	{
		// Get the cell's bounding box in internal coordinates (in 100th of mm)
		const ::Rectangle aCellRect( mxCell->getCellRect() );

		// Transform coordinates from internal to pixel.
		if (maShapeTreeInfo.GetViewForwarder() == NULL)
			throw uno::RuntimeException (::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleCell has no valid view forwarder")),static_cast<uno::XWeak*>(this));

		::Size aPixelSize( maShapeTreeInfo.GetViewForwarder()->LogicToPixel(::Size(aCellRect.GetWidth(), aCellRect.GetHeight())) );
		::Point aPixelPosition( maShapeTreeInfo.GetViewForwarder()->LogicToPixel( aCellRect.TopLeft() ));

		// Clip the shape's bounding box with the bounding box of its parent.
		Reference<XAccessibleComponent> xParentComponent ( getAccessibleParent(), uno::UNO_QUERY);
		if (xParentComponent.is())
		{
			// Make the coordinates relative to the parent.
			awt::Point aParentLocation (xParentComponent->getLocationOnScreen());
			int x = aPixelPosition.getX() - aParentLocation.X;
			int y = aPixelPosition.getY() - aParentLocation.Y;

			// Clip with parent (with coordinates relative to itself).
			::Rectangle aBBox ( x, y, x + aPixelSize.getWidth(), y + aPixelSize.getHeight());
			awt::Size aParentSize (xParentComponent->getSize());
			::Rectangle aParentBBox (0,0, aParentSize.Width, aParentSize.Height);
			aBBox = aBBox.GetIntersection (aParentBBox);
			aBoundingBox = awt::Rectangle (	aBBox.getX(), aBBox.getY(), aBBox.getWidth(), aBBox.getHeight());
		}
		else
		{
			OSL_TRACE ("parent does not support component");
			aBoundingBox = awt::Rectangle (aPixelPosition.getX(), aPixelPosition.getY(),aPixelSize.getWidth(), aPixelSize.getHeight());
		}
	}

    return aBoundingBox;
}

// --------------------------------------------------------------------

::com::sun::star::awt::Point SAL_CALL AccessibleCell::getLocation(void) throw (RuntimeException)
{
    ThrowIfDisposed ();
	::com::sun::star::awt::Rectangle aBoundingBox(getBounds());
    return ::com::sun::star::awt::Point(aBoundingBox.X, aBoundingBox.Y);
}

// --------------------------------------------------------------------

::com::sun::star::awt::Point SAL_CALL AccessibleCell::getLocationOnScreen(void) throw(RuntimeException)
{
    ThrowIfDisposed ();

    // Get relative position...
    ::com::sun::star::awt::Point aLocation(getLocation ());

    // ... and add absolute position of the parent.
    Reference<XAccessibleComponent> xParentComponent( getAccessibleParent(), uno::UNO_QUERY);
    if(xParentComponent.is())
    {
        ::com::sun::star::awt::Point aParentLocation(xParentComponent->getLocationOnScreen());
        aLocation.X += aParentLocation.X;
        aLocation.Y += aParentLocation.Y;
    }
    else
	{
        OSL_TRACE ("getLocation: parent does not support XAccessibleComponent");
	}
    
	return aLocation;
}

// --------------------------------------------------------------------

awt::Size SAL_CALL AccessibleCell::getSize (void) throw (RuntimeException)
{
    ThrowIfDisposed ();
    awt::Rectangle aBoundingBox (getBounds());
    return awt::Size (aBoundingBox.Width, aBoundingBox.Height);
}

// --------------------------------------------------------------------

void SAL_CALL AccessibleCell::addFocusListener ( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener) throw (::com::sun::star::uno::RuntimeException)
{
	AccessibleComponentBase::addFocusListener( xListener );
}

// --------------------------------------------------------------------

void SAL_CALL AccessibleCell::removeFocusListener (const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XFocusListener >& xListener ) throw (::com::sun::star::uno::RuntimeException)
{
	AccessibleComponentBase::removeFocusListener( xListener );
}

// --------------------------------------------------------------------

void SAL_CALL AccessibleCell::grabFocus (void) throw (::com::sun::star::uno::RuntimeException)
{
	AccessibleComponentBase::grabFocus();
}

// --------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleCell::getForeground(void) throw (RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0x0ffffffL);

	// todo
    return nColor;
}

// --------------------------------------------------------------------

sal_Int32 SAL_CALL AccessibleCell::getBackground (void) throw (RuntimeException)
{
    ThrowIfDisposed ();
    sal_Int32 nColor (0L);

	// todo
    return nColor;
}

// --------------------------------------------------------------------
// XAccessibleExtendedComponent
// --------------------------------------------------------------------

::com::sun::star::uno::Reference< ::com::sun::star::awt::XFont > SAL_CALL AccessibleCell::getFont (void) throw (::com::sun::star::uno::RuntimeException)
{
//todo
	return AccessibleComponentBase::getFont();
}

// --------------------------------------------------------------------

::rtl::OUString SAL_CALL AccessibleCell::getTitledBorderText (void) throw (::com::sun::star::uno::RuntimeException)
{
	return AccessibleComponentBase::getTitledBorderText();
}

// --------------------------------------------------------------------

::rtl::OUString SAL_CALL AccessibleCell::getToolTipText (void) throw (::com::sun::star::uno::RuntimeException)
{
	return AccessibleComponentBase::getToolTipText();
}

// --------------------------------------------------------------------
// XAccessibleEventBroadcaster
// --------------------------------------------------------------------

void SAL_CALL AccessibleCell::addEventListener( const Reference<XAccessibleEventListener >& rxListener)  throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ::osl::MutexGuard aGuard (maMutex);
	if (rBHelper.bDisposed || rBHelper.bInDispose)
	{
		Reference<XInterface> xSource( static_cast<XComponent *>(this) );
		lang::EventObject aEventObj(xSource);
		rxListener->disposing(aEventObj);
	}
    else
    {
        AccessibleContextBase::addEventListener (rxListener);
        if (mpText != NULL)
            mpText->AddEventListener (rxListener);
    }
}

// --------------------------------------------------------------------

void SAL_CALL AccessibleCell::removeEventListener( const Reference<XAccessibleEventListener >& rxListener) throw (RuntimeException)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    AccessibleContextBase::removeEventListener(rxListener);
    if (mpText != NULL)
        mpText->RemoveEventListener (rxListener);
}

// --------------------------------------------------------------------
// XServiceInfo
// --------------------------------------------------------------------

OUString SAL_CALL AccessibleCell::getImplementationName(void) throw (RuntimeException)
{
	return OUString(RTL_CONSTASCII_USTRINGPARAM("AccessibleCell"));
}

// --------------------------------------------------------------------

Sequence<OUString> SAL_CALL AccessibleCell::getSupportedServiceNames(void) throw (RuntimeException)
{
    ThrowIfDisposed ();

	// Get list of supported service names from base class...
    uno::Sequence<OUString> aServiceNames = AccessibleContextBase::getSupportedServiceNames();
    sal_Int32 nCount (aServiceNames.getLength());

    // ...and add additional names.
    aServiceNames.realloc (nCount + 1);
    static const OUString sAdditionalServiceName (RTL_CONSTASCII_USTRINGPARAM("com.sun.star.drawing.AccessibleCell"));
    aServiceNames[nCount] = sAdditionalServiceName;

    return aServiceNames;
}

// --------------------------------------------------------------------
// IAccessibleViewForwarderListener
// --------------------------------------------------------------------

void AccessibleCell::ViewForwarderChanged (ChangeType /*aChangeType*/, const IAccessibleViewForwarder* /*pViewForwarder*/)
{
    // Inform all listeners that the graphical representation (i.e. size
    // and/or position) of the shape has changed.
    CommitChange(AccessibleEventId::VISIBLE_DATA_CHANGED, Any(), Any());

    // update our children that our screen position might have changed
    if( mpText )
        mpText->UpdateChildren();
}

// --------------------------------------------------------------------
// protected
// --------------------------------------------------------------------

void AccessibleCell::disposing (void)
{
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());
    ::osl::MutexGuard aGuard (maMutex);

    // Make sure to send an event that this object looses the focus in the
    // case that it has the focus.
    ::utl::AccessibleStateSetHelper* pStateSet = static_cast< ::utl::AccessibleStateSetHelper*>(mxStateSet.get());
    if (pStateSet != NULL)
        pStateSet->RemoveState(AccessibleStateType::FOCUSED);

    if (mpText != NULL)
    {
        mpText->Dispose();
        delete mpText;
        mpText = NULL;
    }

    // Cleanup.  Remove references to objects to allow them to be
    // destroyed.
    mxCell.clear();
    maShapeTreeInfo = AccessibleShapeTreeInfo();

    // Call base classes.
    AccessibleContextBase::dispose ();
}

sal_Int32 SAL_CALL AccessibleCell::getAccessibleIndexInParent (void) throw (RuntimeException)
{
    ThrowIfDisposed ();
    return mnIndexInParent;
}

sdr::table::CellRef AccessibleCell::getCellRef()
{
	return mxCell;
}
::rtl::OUString AccessibleCell::getCellName( sal_Int32 nCol, sal_Int32 nRow )
{
    rtl::OUStringBuffer aBuf;

    if (nCol < 26*26)
    {
        if (nCol < 26)
            aBuf.append( static_cast<sal_Unicode>( 'A' +
                        static_cast<sal_uInt16>(nCol)));
        else
        {
            aBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) / 26) - 1));
            aBuf.append( static_cast<sal_Unicode>( 'A' +
                        (static_cast<sal_uInt16>(nCol) % 26)));
        }
    }
    else
    {
        String aStr;
        while (nCol >= 26)
        {
            sal_Int32 nC = nCol % 26;
            aStr += static_cast<sal_Unicode>( 'A' +
                    static_cast<sal_uInt16>(nC));
            nCol = nCol - nC;
            nCol = nCol / 26 - 1;
        }
        aStr += static_cast<sal_Unicode>( 'A' +
                static_cast<sal_uInt16>(nCol));
        aStr.Reverse();
        aBuf.append( aStr);
    }
    aBuf.append( OUString::valueOf(nRow+1) );
    return aBuf.makeStringAndClear();
}
::rtl::OUString SAL_CALL AccessibleCell::getAccessibleName (void) throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());

    if( pAccTable ) 
	try
    {
        sal_Int32 nRow = 0, nCol = 0;
        pAccTable->getColumnAndRow(mnIndexInParent, nCol, nRow);
        return getCellName( nCol, nRow );
    }
    catch( Exception& )
    {
    }

    return AccessibleCellBase::getAccessibleName();
}
void AccessibleCell::UpdateChildren()
{
	if (mpText)						
		mpText->UpdateChildren();
}

/* MT: Above getAccessibleName was introduced with IA2 CWS, while below was introduce in 3.3 meanwhile. Check which one is correct
If this is correct, we also don't need 	sdr::table::CellRef getCellRef(), UpdateChildren(), getCellName( sal_Int32 nCol, sal_Int32 nRow ) above

::rtl::OUString SAL_CALL AccessibleCell::getAccessibleName (void) throw (::com::sun::star::uno::RuntimeException)
{
    ThrowIfDisposed ();
    ::vos::OGuard aSolarGuard (::Application::GetSolarMutex());

    if( mxCell.is() )
        return mxCell->getName();

    return AccessibleCellBase::getAccessibleName();
}
*/

} // end of namespace accessibility
