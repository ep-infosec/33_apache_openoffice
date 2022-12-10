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


#include <tools/debug.hxx>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <wrtsh.hxx>
#include "barcfg.hxx"

#include <unomid.h>

using namespace utl;
using rtl::OUString;
using namespace com::sun::star::uno;

#define SEL_TYPE_TABLE_TEXT		0
#define SEL_TYPE_LIST_TEXT      1
#define SEL_TYPE_TABLE_LIST     2
#define SEL_TYPE_BEZIER         3
#define SEL_TYPE_GRAPHIC		4

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwToolbarConfigItem::SwToolbarConfigItem( sal_Bool bWeb ) :
    ConfigItem(bWeb ? C2U("Office.WriterWeb/ObjectBar") :  C2U("Office.Writer/ObjectBar"),
        CONFIG_MODE_DELAYED_UPDATE|CONFIG_MODE_RELEASE_TREE)
{
	for(sal_uInt16 i = 0; i <= SEL_TYPE_GRAPHIC; i++ )
        aTbxIdArray[i] = -1;

	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues = GetProperties(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{
		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			if(pValues[nProp].hasValue())
			{
				sal_Int32 nVal = 0;
				pValues[nProp] >>= nVal;
                aTbxIdArray[nProp] = nVal;
			}
		}
	}
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
SwToolbarConfigItem::~SwToolbarConfigItem()
{
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
sal_Int32 lcl_getArrayIndex(int nSelType)
{
	sal_Int32 nRet = -1;
	if(nSelType & nsSelectionType::SEL_NUM)
	{
		if(nSelType & nsSelectionType::SEL_TBL)
			nRet = SEL_TYPE_TABLE_LIST;
		else
			nRet = SEL_TYPE_LIST_TEXT;
	}
	else if(nSelType & nsSelectionType::SEL_TBL)
		nRet = SEL_TYPE_TABLE_TEXT;
	else if(nSelType & nsSelectionType::SEL_BEZ)
		nRet = SEL_TYPE_BEZIER;
	else if(nSelType & nsSelectionType::SEL_GRF)
		nRet = SEL_TYPE_GRAPHIC;
	return nRet;
}
/* -----------------------------10.10.00 14:38--------------------------------

 ---------------------------------------------------------------------------*/
void SwToolbarConfigItem::SetTopToolbar( sal_Int32 nSelType, sal_Int32 nBarId )
{
	sal_Int32 nProp = lcl_getArrayIndex(nSelType);
	if(nProp >= 0)
	{
		aTbxIdArray[nProp] = nBarId;
		SetModified();
	}
}
/* -----------------------------10.10.00 13:33--------------------------------

 ---------------------------------------------------------------------------*/
Sequence<OUString> SwToolbarConfigItem::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
		"Selection/Table",                   //  SEL_TYPE_TABLE_TEXT
		"Selection/NumberedList",            //  SEL_TYPE_LIST_TEXT
		"Selection/NumberedList_InTable",     //  SEL_TYPE_TABLE_LIST
		"Selection/BezierObject",   		//  SEL_TYPE_BEZIER
		"Selection/Graphic"					//SEL_TYPE_GRAPHIC
	};
	const int nCount = 5;
	Sequence<OUString> aNames(nCount);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < nCount; i++)
		pNames[i] = OUString::createFromAscii(aPropNames[i]);
	return aNames;
}
/* -----------------------------10.10.00 13:36--------------------------------

 ---------------------------------------------------------------------------*/
void SwToolbarConfigItem::Commit()
{
	Sequence<OUString> aNames = GetPropertyNames();

	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
        pValues[nProp] <<= aTbxIdArray[nProp];
	PutProperties(aNames, aValues);
}

void SwToolbarConfigItem::Notify( const ::com::sun::star::uno::Sequence< rtl::OUString >& ) {}

