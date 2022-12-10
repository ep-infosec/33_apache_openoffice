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
#include "precompiled_sc.hxx"



#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "unitconv.hxx"
#include "global.hxx"
#include "viewopti.hxx"			//! move ScLinkConfigItem to separate header!

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

// --------------------------------------------------------------------

const sal_Unicode cDelim = 0x01;		// Delimiter zwischen From und To


// --- ScUnitConverterData --------------------------------------------

ScUnitConverterData::ScUnitConverterData( const String& rFromUnit,
			const String& rToUnit, double fVal )
		:
		StrData( rFromUnit ),
		fValue( fVal )
{
	String aTmp;
	ScUnitConverterData::BuildIndexString( aTmp, rFromUnit, rToUnit );
	SetString( aTmp );
}


ScUnitConverterData::ScUnitConverterData( const ScUnitConverterData& r )
		:
		StrData( r ),
		fValue( r.fValue )
{
}


ScDataObject* ScUnitConverterData::Clone() const
{
	return new ScUnitConverterData( *this );
}


// static
void ScUnitConverterData::BuildIndexString( String& rStr,
			const String& rFromUnit, const String& rToUnit )
{
#if 1
// case sensitive
	rStr = rFromUnit;
	rStr += cDelim;
	rStr += rToUnit;
#else
// not case sensitive
	rStr = rFromUnit;
	String aTo( rToUnit );
	ScGlobal::pCharClass->toUpper( rStr );
	ScGlobal::pCharClass->toUpper( aTo );
	rStr += cDelim;
	rStr += aTo;
#endif
}


// --- ScUnitConverter ------------------------------------------------

#define CFGPATH_UNIT		"Office.Calc/UnitConversion"
#define CFGSTR_UNIT_FROM	"FromUnit"
#define CFGSTR_UNIT_TO		"ToUnit"
#define CFGSTR_UNIT_FACTOR	"Factor"

ScUnitConverter::ScUnitConverter( sal_uInt16 nInit, sal_uInt16 nDeltaP ) :
        ScStrCollection( nInit, nDeltaP, sal_False )
{
	//	read from configuration - "convert.ini" is no longer used
	//!	config item as member to allow change of values

	ScLinkConfigItem aConfigItem( OUString::createFromAscii( CFGPATH_UNIT ) );

	// empty node name -> use the config item's path itself
    OUString aEmptyString;
	Sequence<OUString> aNodeNames = aConfigItem.GetNodeNames( aEmptyString );

	long nNodeCount = aNodeNames.getLength();
	if ( nNodeCount )
	{
		const OUString* pNodeArray = aNodeNames.getConstArray();
		Sequence<OUString> aValNames( nNodeCount * 3 );
		OUString* pValNameArray = aValNames.getArray();
		const OUString sSlash('/');

		long nIndex = 0;
		for (long i=0; i<nNodeCount; i++)
		{
			OUString sPrefix = pNodeArray[i];
			sPrefix += sSlash;

			pValNameArray[nIndex] = sPrefix;
			pValNameArray[nIndex++] += OUString::createFromAscii( CFGSTR_UNIT_FROM );
			pValNameArray[nIndex] = sPrefix;
			pValNameArray[nIndex++] += OUString::createFromAscii( CFGSTR_UNIT_TO );
			pValNameArray[nIndex] = sPrefix;
			pValNameArray[nIndex++] += OUString::createFromAscii( CFGSTR_UNIT_FACTOR );
		}

		Sequence<Any> aProperties = aConfigItem.GetProperties(aValNames);

		if (aProperties.getLength() == aValNames.getLength())
		{
			const Any* pProperties = aProperties.getConstArray();

			OUString sFromUnit;
			OUString sToUnit;
			double fFactor = 0;

			nIndex = 0;
			for (long i=0; i<nNodeCount; i++)
			{
				pProperties[nIndex++] >>= sFromUnit;
				pProperties[nIndex++] >>= sToUnit;
				pProperties[nIndex++] >>= fFactor;

				ScUnitConverterData* pNew = new ScUnitConverterData( sFromUnit, sToUnit, fFactor );
				if ( !Insert( pNew ) )
					delete pNew;
			}
		}
	}
}

sal_Bool ScUnitConverter::GetValue( double& fValue, const String& rFromUnit,
				const String& rToUnit ) const
{
	ScUnitConverterData aSearch( rFromUnit, rToUnit );
	sal_uInt16 nIndex;
	if ( Search( &aSearch, nIndex ) )
	{
		fValue = ((const ScUnitConverterData*)(At( nIndex )))->GetValue();
		return sal_True;
	}
	fValue = 1.0;
	return sal_False;
}


