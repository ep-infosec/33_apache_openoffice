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
#include "precompiled_xmloff.hxx"

#include <tools/debug.hxx>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/container/XIndexReplace.hpp>
#include <com/sun/star/style/NumberingType.hpp>
#include <com/sun/star/container/XNamed.hpp>
#include "XMLTextNumRuleInfo.hxx"
// --> OD 2008-04-25 #refactorlists#
#include "xmloff/XMLTextListAutoStylePool.hxx"
// <--

using ::rtl::OUString;

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::style;

// --> OD 2008-05-08 #refactorlists#
// Complete refactoring of the class and enhancement of the class for lists.
XMLTextNumRuleInfo::XMLTextNumRuleInfo()
    : msNumberingRules(RTL_CONSTASCII_USTRINGPARAM("NumberingRules"))
    , msNumberingLevel(RTL_CONSTASCII_USTRINGPARAM("NumberingLevel"))
    , msNumberingStartValue(RTL_CONSTASCII_USTRINGPARAM("NumberingStartValue"))
    , msParaIsNumberingRestart(RTL_CONSTASCII_USTRINGPARAM("ParaIsNumberingRestart"))
    , msNumberingIsNumber(RTL_CONSTASCII_USTRINGPARAM("NumberingIsNumber"))
    , msNumberingIsOutline(RTL_CONSTASCII_USTRINGPARAM("NumberingIsOutline"))
    , msPropNameListId(RTL_CONSTASCII_USTRINGPARAM("ListId"))
    , msPropNameStartWith(RTL_CONSTASCII_USTRINGPARAM("StartWith"))
    // --> OD 2008-11-26 #158694#
    , msContinuingPreviousSubTree(RTL_CONSTASCII_USTRINGPARAM("ContinueingPreviousSubTree")) // misspelling kept for compatibility
    , msListLabelStringProp(RTL_CONSTASCII_USTRINGPARAM("ListLabelString"))
    // <--
    , mxNumRules()
    , msNumRulesName()
    , msListId()
    , mnListStartValue( -1 )
    , mnListLevel( 0 )
    , mbIsNumbered( sal_False )
    , mbIsRestart( sal_False )
    , mnListLevelStartValue( -1 )
    , mbOutlineStyleAsNormalListStyle( sal_False )
{
	Reset();
}

// --> OD 2006-09-27 #i69627#
void XMLTextNumRuleInfo::Set(
        const ::com::sun::star::uno::Reference <
                        ::com::sun::star::text::XTextContent > & xTextContent,
        const sal_Bool bOutlineStyleAsNormalListStyle,
        const XMLTextListAutoStylePool& rListAutoPool,
        // --> OD 2008-11-26 #158694#
        const sal_Bool bExportTextNumberElement )
        // <--
{
	Reset();
    // --> OD 2006-09-27 #i69627#
    mbOutlineStyleAsNormalListStyle = bOutlineStyleAsNormalListStyle;
    // <--

	Reference< XPropertySet > xPropSet( xTextContent, UNO_QUERY );
	Reference< XPropertySetInfo > xPropSetInfo = xPropSet->getPropertySetInfo();

    // check if this paragraph supports a numbering
    if( !xPropSetInfo->hasPropertyByName( msNumberingLevel ) )
		return;

    if( xPropSet->getPropertyValue( msNumberingLevel ) >>= mnListLevel )
	{
        if( xPropSetInfo->hasPropertyByName( msNumberingRules ) )
        {
            xPropSet->getPropertyValue( msNumberingRules ) >>= mxNumRules;
        }
	}
	else
	{
		// in applications using the outliner we always have a numbering rule,
		// so a void property no numbering
        mnListLevel = 0;
	}

    // --> OD 2008-12-17 #i97312#
    if ( mxNumRules.is() && mxNumRules->getCount() < 1 )
    {
        DBG_ASSERT( false,
                    "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance does not contain any numbering rule" );
        Reset();
        return;
    }
    // <--

    // --> OD 2010-01-13 #b6912256#
    if ( mnListLevel < 0 )
    {
        DBG_ASSERT( false,
                    "<XMLTextNumRuleInfo::Set(..)> - unexpected numbering level" );
        Reset();
        return;
    }

    // --> OD 2006-09-27 #i69627#
    bool bSuppressListStyle( false );
    if ( mxNumRules.is() )
    {
        if ( !mbOutlineStyleAsNormalListStyle )
        {
            sal_Bool bIsOutline = sal_False;
            Reference<XPropertySet> xNumRulesProps(mxNumRules, UNO_QUERY);
            if ( xNumRulesProps.is() &&
                 xNumRulesProps->getPropertySetInfo()->
                                    hasPropertyByName( msNumberingIsOutline ) )
            {
                xNumRulesProps->getPropertyValue( msNumberingIsOutline ) >>= bIsOutline;
                bSuppressListStyle = bIsOutline ? true : false;
            }
        }
    }

    if( mxNumRules.is() && !bSuppressListStyle )
    // <--
	{
        // First try to find the numbering rules in the list auto style pool.
        // If not found, the numbering rules instance has to be named.
        msNumRulesName = rListAutoPool.Find( mxNumRules );
        if ( msNumRulesName.getLength() == 0 )
        {
            Reference < XNamed > xNamed( mxNumRules, UNO_QUERY );
            DBG_ASSERT( xNamed.is(),
                        "<XMLTextNumRuleInfo::Set(..)> - numbering rules instance have to be named. Serious defect -> please inform OD." );
            if( xNamed.is() )
            {
                msNumRulesName = xNamed->getName();
            }
        }
        DBG_ASSERT( msNumRulesName.getLength() > 0,
                    "<XMLTextNumRuleInfo::Set(..)> - no name found for numbering rules instance. Serious defect -> please inform OD." );

        if( xPropSetInfo->hasPropertyByName( msPropNameListId ) )
        {
            xPropSet->getPropertyValue( msPropNameListId ) >>= msListId;
        }

        // --> OD 2008-11-26 #158694#
        mbContinuingPreviousSubTree = sal_False;
        if( xPropSetInfo->hasPropertyByName( msContinuingPreviousSubTree ) )
        {
            xPropSet->getPropertyValue( msContinuingPreviousSubTree ) >>= mbContinuingPreviousSubTree;
        }
        // <--

        mbIsNumbered = sal_True;
        if( xPropSetInfo->hasPropertyByName( msNumberingIsNumber ) )
		{
            if( !(xPropSet->getPropertyValue( msNumberingIsNumber ) >>= mbIsNumbered ) )
            {
    			OSL_ENSURE( false, "numbered paragraph without number info" );
                mbIsNumbered = sal_False;
            }
		}

        if( mbIsNumbered )
		{
            if( xPropSetInfo->hasPropertyByName( msParaIsNumberingRestart ) )
			{
                xPropSet->getPropertyValue( msParaIsNumberingRestart ) >>= mbIsRestart;
			}
            if( xPropSetInfo->hasPropertyByName( msNumberingStartValue ) )
			{
                xPropSet->getPropertyValue( msNumberingStartValue ) >>= mnListStartValue;
			}
		}

        OSL_ENSURE( mnListLevel < mxNumRules->getCount(), "wrong num rule level" );
        if( mnListLevel >= mxNumRules->getCount() )
		{
			Reset();
			return;
		}

        Sequence<PropertyValue> aProps;
        mxNumRules->getByIndex( mnListLevel ) >>= aProps;

        const PropertyValue* pPropArray = aProps.getConstArray();
        sal_Int32 nCount = aProps.getLength();
        for( sal_Int32 i=0; i<nCount; i++ )
        {
          const PropertyValue& rProp = pPropArray[i];

            if ( rProp.Name == msPropNameStartWith )
            {
                rProp.Value >>= mnListLevelStartValue;
                break;
            }
        }

        // --> OD 2008-11-26 #158694#
        msListLabelString = ::rtl::OUString();
        if ( bExportTextNumberElement &&
             xPropSetInfo->hasPropertyByName( msListLabelStringProp ) )
        {
            xPropSet->getPropertyValue( msListLabelStringProp ) >>= msListLabelString;
        }
        // <--

        // paragraph's list level range is [0..9] representing list levels [1..10]
        ++mnListLevel;
	}
    else
    {
        mnListLevel = 0;
    }
}

sal_Bool XMLTextNumRuleInfo::BelongsToSameList( const XMLTextNumRuleInfo& rCmp ) const
{
    sal_Bool bRet( sal_True );
    // Currently only the text documents support <ListId>.
    if ( rCmp.msListId.getLength() > 0 ||
         msListId.getLength() > 0 )
    {
        bRet = rCmp.msListId == msListId;
    }
    else
    {
        bRet = HasSameNumRules( rCmp );
    }

    return bRet;
}
// <--
