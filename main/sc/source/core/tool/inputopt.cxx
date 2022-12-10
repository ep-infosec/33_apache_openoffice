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



//------------------------------------------------------------------

#include <tools/debug.hxx>

#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include "cfgids.hxx"
#include "inputopt.hxx"
#include "rechead.hxx"
#include "scresid.hxx"
#include "global.hxx"
#include "sc.hrc"
#include "miscuno.hxx"

using namespace utl;
using namespace rtl;
using namespace com::sun::star::uno;

//------------------------------------------------------------------

//	Version, ab der das Item kompatibel ist
#define SC_VERSION ((sal_uInt16)351)


//========================================================================
//      ScInputOptions - Eingabe-Optionen
//========================================================================

ScInputOptions::ScInputOptions()
{
	SetDefaults();
}

//------------------------------------------------------------------------

ScInputOptions::ScInputOptions( const ScInputOptions& rCpy )
{
	*this = rCpy;
}

//------------------------------------------------------------------------

ScInputOptions::~ScInputOptions()
{
}

//------------------------------------------------------------------------

void ScInputOptions::SetDefaults()
{
	nMoveDir		= DIR_BOTTOM;
	bMoveSelection	= sal_True;
	bEnterEdit		= sal_False;
	bExtendFormat	= sal_False;
	bRangeFinder	= sal_True;
	bExpandRefs		= sal_False;
	bMarkHeader		= sal_True;
	bUseTabCol		= sal_False;
	bTextWysiwyg	= sal_False;
    bReplCellsWarn  = sal_True;
}

//------------------------------------------------------------------------

const ScInputOptions& ScInputOptions::operator=( const ScInputOptions& rCpy )
{
	nMoveDir		= rCpy.nMoveDir;
	bMoveSelection	= rCpy.bMoveSelection;
	bEnterEdit		= rCpy.bEnterEdit;
	bExtendFormat	= rCpy.bExtendFormat;
	bRangeFinder	= rCpy.bRangeFinder;
	bExpandRefs		= rCpy.bExpandRefs;
	bMarkHeader		= rCpy.bMarkHeader;
	bUseTabCol		= rCpy.bUseTabCol;
	bTextWysiwyg	= rCpy.bTextWysiwyg;
    bReplCellsWarn  = rCpy.bReplCellsWarn;

	return *this;
}


//==================================================================
//	Config Item containing input options
//==================================================================

#define CFGPATH_INPUT			"Office.Calc/Input"

#define SCINPUTOPT_MOVEDIR			0
#define SCINPUTOPT_MOVESEL			1
#define SCINPUTOPT_EDTEREDIT		2
#define SCINPUTOPT_EXTENDFMT		3
#define SCINPUTOPT_RANGEFIND		4
#define SCINPUTOPT_EXPANDREFS		5
#define SCINPUTOPT_MARKHEADER		6
#define SCINPUTOPT_USETABCOL		7
#define SCINPUTOPT_TEXTWYSIWYG		8
#define SCINPUTOPT_REPLCELLSWARN    9
#define SCINPUTOPT_COUNT            10

Sequence<OUString> ScInputCfg::GetPropertyNames()
{
	static const char* aPropNames[] =
	{
		"MoveSelectionDirection",	// SCINPUTOPT_MOVEDIR
		"MoveSelection",			// SCINPUTOPT_MOVESEL
		"SwitchToEditMode",			// SCINPUTOPT_EDTEREDIT
		"ExpandFormatting",			// SCINPUTOPT_EXTENDFMT
		"ShowReference",			// SCINPUTOPT_RANGEFIND
		"ExpandReference",			// SCINPUTOPT_EXPANDREFS
		"HighlightSelection",		// SCINPUTOPT_MARKHEADER
		"UseTabCol",				// SCINPUTOPT_USETABCOL
        "UsePrinterMetrics",        // SCINPUTOPT_TEXTWYSIWYG
        "ReplaceCellsWarning"       // SCINPUTOPT_REPLCELLSWARN
	};
	Sequence<OUString> aNames(SCINPUTOPT_COUNT);
	OUString* pNames = aNames.getArray();
	for(int i = 0; i < SCINPUTOPT_COUNT; i++)
		pNames[i] = OUString::createFromAscii(aPropNames[i]);

	return aNames;
}

ScInputCfg::ScInputCfg() :
	ConfigItem( OUString::createFromAscii( CFGPATH_INPUT ) )
{
	sal_Int32 nIntVal = 0;

	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues = GetProperties(aNames);
	EnableNotification(aNames);
	const Any* pValues = aValues.getConstArray();
	DBG_ASSERT(aValues.getLength() == aNames.getLength(), "GetProperties failed");
	if(aValues.getLength() == aNames.getLength())
	{
		for(int nProp = 0; nProp < aNames.getLength(); nProp++)
		{
			DBG_ASSERT(pValues[nProp].hasValue(), "property value missing");
			if(pValues[nProp].hasValue())
			{
				switch(nProp)
				{
					case SCINPUTOPT_MOVEDIR:
						if ( pValues[nProp] >>= nIntVal )
							SetMoveDir( (sal_uInt16)nIntVal );
						break;
					case SCINPUTOPT_MOVESEL:
						SetMoveSelection( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_EDTEREDIT:
						SetEnterEdit( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_EXTENDFMT:
						SetExtendFormat( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_RANGEFIND:
						SetRangeFinder( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_EXPANDREFS:
						SetExpandRefs( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_MARKHEADER:
						SetMarkHeader( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_USETABCOL:
						SetUseTabCol( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
					case SCINPUTOPT_TEXTWYSIWYG:
						SetTextWysiwyg( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
						break;
                    case SCINPUTOPT_REPLCELLSWARN:
                        SetReplaceCellsWarn( ScUnoHelpFunctions::GetBoolFromAny( pValues[nProp] ) );
                        break;
				}
			}
		}
	}
}


void ScInputCfg::Commit()
{
	Sequence<OUString> aNames = GetPropertyNames();
	Sequence<Any> aValues(aNames.getLength());
	Any* pValues = aValues.getArray();

	for(int nProp = 0; nProp < aNames.getLength(); nProp++)
	{
		switch(nProp)
		{
			case SCINPUTOPT_MOVEDIR:
				pValues[nProp] <<= (sal_Int32) GetMoveDir();
				break;
			case SCINPUTOPT_MOVESEL:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetMoveSelection() );
				break;
			case SCINPUTOPT_EDTEREDIT:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetEnterEdit() );
				break;
			case SCINPUTOPT_EXTENDFMT:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetExtendFormat() );
				break;
			case SCINPUTOPT_RANGEFIND:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetRangeFinder() );
				break;
			case SCINPUTOPT_EXPANDREFS:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetExpandRefs() );
				break;
			case SCINPUTOPT_MARKHEADER:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetMarkHeader() );
				break;
			case SCINPUTOPT_USETABCOL:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetUseTabCol() );
				break;
			case SCINPUTOPT_TEXTWYSIWYG:
				ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetTextWysiwyg() );
				break;
            case SCINPUTOPT_REPLCELLSWARN:
                ScUnoHelpFunctions::SetBoolInAny( pValues[nProp], GetReplaceCellsWarn() );
                break;
		}
	}
	PutProperties(aNames, aValues);
}

void ScInputCfg::Notify( const Sequence<rtl::OUString>& /* aPropertyNames */ )
{
	DBG_ERROR("properties have been changed");
}

void ScInputCfg::SetOptions( const ScInputOptions& rNew )
{
	*(ScInputOptions*)this = rNew;
	SetModified();
}

void ScInputCfg::OptionsChanged()
{
	SetModified();
}


