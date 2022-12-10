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
#include "precompiled_scui.hxx"




#include "scuiimoptdlg.hxx"
#include "scresid.hxx"
#include "imoptdlg.hrc"
#include <rtl/tencinfo.h>
//========================================================================
// ScDelimiterTable
//========================================================================

class ScDelimiterTable
{
public:
		ScDelimiterTable( const String& rDelTab )
			:	theDelTab ( rDelTab ),
				cSep	  ( '\t' ),
				nCount	  ( rDelTab.GetTokenCount('\t') ),
				nIter	  ( 0 )
			{}

	sal_uInt16	GetCode( const String& rDelimiter ) const;
	String	GetDelimiter( sal_Unicode nCode ) const;

	String	FirstDel()	{ nIter = 0; return theDelTab.GetToken( nIter, cSep ); }
	String	NextDel()	{ nIter +=2; return theDelTab.GetToken( nIter, cSep ); }

private:
	const String		theDelTab;
	const sal_Unicode	cSep;
	const xub_StrLen	nCount;
	xub_StrLen			nIter;
};

//------------------------------------------------------------------------

sal_uInt16 ScDelimiterTable::GetCode( const String& rDel ) const
{
	sal_Unicode nCode = 0;
	xub_StrLen i = 0;

	if ( nCount >= 2 )
	{
		while ( i<nCount )
		{
			if ( rDel == theDelTab.GetToken( i, cSep ) )
			{
				nCode = (sal_Unicode) theDelTab.GetToken( i+1, cSep ).ToInt32();
				i     = nCount;
			}
			else
				i += 2;
		}
	}

	return nCode;
}

//------------------------------------------------------------------------

String ScDelimiterTable::GetDelimiter( sal_Unicode nCode ) const
{
	String aStrDel;
	xub_StrLen i = 0;

	if ( nCount >= 2 )
	{
		while ( i<nCount )
		{
			if ( nCode == (sal_Unicode) theDelTab.GetToken( i+1, cSep ).ToInt32() )
			{
				aStrDel = theDelTab.GetToken( i, cSep );
				i       = nCount;
			}
			else
				i += 2;
		}
	}

	return aStrDel;
}

//========================================================================
// ScImportOptionsDlg
//========================================================================

ScImportOptionsDlg::ScImportOptionsDlg(
        Window*                 pParent,
        sal_Bool                    bAscii,
        const ScImportOptions*  pOptions,
        const String*           pStrTitle,
        sal_Bool                    bMultiByte,
        sal_Bool                    bOnlyDbtoolsEncodings,
        sal_Bool                    bImport )

	:	ModalDialog	( pParent, ScResId( RID_SCDLG_IMPORTOPT ) ),
        aFlFieldOpt ( this, ScResId( FL_FIELDOPT ) ),
		aFtFont		( this, ScResId( FT_FONT ) ),
        aLbFont     ( this, ScResId( bAscii ? DDLB_FONT : LB_FONT ) ),
		aFtFieldSep	( this, ScResId( FT_FIELDSEP ) ),
		aEdFieldSep	( this, ScResId( ED_FIELDSEP ) ),
		aFtTextSep	( this, ScResId( FT_TEXTSEP ) ),
		aEdTextSep	( this, ScResId( ED_TEXTSEP ) ),
        aCbQuoteAll ( this, ScResId( CB_QUOTEALL ) ),
        aCbShown    ( this, ScResId( CB_SAVESHOWN ) ),
        aCbFixed    ( this, ScResId( CB_FIXEDWIDTH ) ),
		aBtnOk		( this, ScResId( BTN_OK ) ),
		aBtnCancel	( this, ScResId( BTN_CANCEL ) ),
        aBtnHelp	( this, ScResId( BTN_HELP ) )
{
    String sFieldSep( ScResId( SCSTR_FIELDSEP ) );
    sFieldSep.SearchAndReplaceAscii( "%TAB",   String(ScResId(SCSTR_FIELDSEP_TAB)) );
    sFieldSep.SearchAndReplaceAscii( "%SPACE", String(ScResId(SCSTR_FIELDSEP_SPACE)) );

	// im Ctor-Initializer nicht moeglich (MSC kann das nicht):
	pFieldSepTab = new ScDelimiterTable( sFieldSep );
	pTextSepTab  = new ScDelimiterTable( String(ScResId(SCSTR_TEXTSEP)) );

	String aStr = pFieldSepTab->FirstDel();
	sal_Unicode nCode;

	while ( aStr.Len() > 0 )
	{
		aEdFieldSep.InsertEntry( aStr );
		aStr = pFieldSepTab->NextDel();
	}

	aStr = pTextSepTab->FirstDel();

	while ( aStr.Len() > 0 )
	{
		aEdTextSep.InsertEntry( aStr );
		aStr = pTextSepTab->NextDel();
	}

	aEdFieldSep.SetText( aEdFieldSep.GetEntry(0) );
	aEdTextSep.SetText( aEdTextSep.GetEntry(0) );

    if ( bOnlyDbtoolsEncodings )
    {
        // Even dBase export allows multibyte now
        if ( bMultiByte )
            aLbFont.FillFromDbTextEncodingMap( bImport );
        else
            aLbFont.FillFromDbTextEncodingMap( bImport, RTL_TEXTENCODING_INFO_MULTIBYTE );
    }
    else if ( !bAscii )
    {   //!TODO: Unicode would need work in each filter
		if ( bMultiByte )
            aLbFont.FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE );
		else
			aLbFont.FillFromTextEncodingTable( bImport, RTL_TEXTENCODING_INFO_UNICODE |
                RTL_TEXTENCODING_INFO_MULTIBYTE );
	}
	else
	{
		if ( pOptions )
		{
			nCode = pOptions->nFieldSepCode;
			aStr  = pFieldSepTab->GetDelimiter( nCode );

			if ( !aStr.Len() )
				aEdFieldSep.SetText( String((sal_Unicode)nCode) );
			else
				aEdFieldSep.SetText( aStr );

			nCode = pOptions->nTextSepCode;
			aStr  = pTextSepTab->GetDelimiter( nCode );

			if ( !aStr.Len() )
				aEdTextSep.SetText( String((sal_Unicode)nCode) );
			else
				aEdTextSep.SetText( aStr );
		}
		// all encodings allowed, even Unicode
		aLbFont.FillFromTextEncodingTable( bImport );
	}

    if( bAscii )
    {
        Size aWinSize( GetSizePixel() );
        aWinSize.Height() = aCbFixed.GetPosPixel().Y() + aCbFixed.GetSizePixel().Height();
        Size aDiffSize( LogicToPixel( Size( 0, 6 ), MapMode( MAP_APPFONT ) ) );
        aWinSize.Height() += aDiffSize.Height();
        SetSizePixel( aWinSize );
        aCbFixed.Show();
        aCbFixed.SetClickHdl( LINK( this, ScImportOptionsDlg, FixedWidthHdl ) );
        aCbFixed.Check( sal_False );
        aCbShown.Show();
        aCbShown.Check( sal_True );
        aCbQuoteAll.Show();
        aCbQuoteAll.Check( sal_False );
    }
    else
    {
        aFlFieldOpt.SetText( aFtFont.GetText() );
		aFtFieldSep.Hide();
        aFtTextSep.Hide();
        aFtFont.Hide();
		aEdFieldSep.Hide();
        aEdTextSep.Hide();
        aCbFixed.Hide();
        aCbShown.Hide();
        aCbQuoteAll.Hide();
		aLbFont.GrabFocus();
        aLbFont.SetDoubleClickHdl( LINK( this, ScImportOptionsDlg, DoubleClickHdl ) );
    }

	aLbFont.SelectTextEncoding( pOptions ? pOptions->eCharSet :
		gsl_getSystemTextEncoding() );

	// optionaler Titel:
	if ( pStrTitle )
		SetText( *pStrTitle );

	FreeResource();
}

//------------------------------------------------------------------------

__EXPORT ScImportOptionsDlg::~ScImportOptionsDlg()
{
	delete pFieldSepTab;
	delete pTextSepTab;
}

//------------------------------------------------------------------------

void ScImportOptionsDlg::GetImportOptions( ScImportOptions& rOptions ) const
{
	rOptions.SetTextEncoding( aLbFont.GetSelectTextEncoding() );

    if ( aCbFixed.IsVisible() )
	{
		rOptions.nFieldSepCode = GetCodeFromCombo( aEdFieldSep );
		rOptions.nTextSepCode  = GetCodeFromCombo( aEdTextSep );
        rOptions.bFixedWidth = aCbFixed.IsChecked();
        rOptions.bSaveAsShown = aCbShown.IsChecked();
        rOptions.bQuoteAllText = aCbQuoteAll.IsChecked();
	}
}

//------------------------------------------------------------------------

sal_uInt16 ScImportOptionsDlg::GetCodeFromCombo( const ComboBox& rEd ) const
{
	ScDelimiterTable* pTab;
	String  aStr( rEd.GetText() );
	sal_uInt16  nCode;

	if ( &rEd == &aEdTextSep )
		pTab = pTextSepTab;
	else
		pTab = pFieldSepTab;

	if ( !aStr.Len() )
	{
		nCode = 0;			// kein Trennzeichen
	}
	else
	{
		nCode = pTab->GetCode( aStr );

		if ( nCode == 0 )
			nCode = (sal_uInt16)aStr.GetChar(0);
	}

	return nCode;
}

//------------------------------------------------------------------------

IMPL_LINK( ScImportOptionsDlg, FixedWidthHdl, CheckBox*, pCheckBox )
{
    if( pCheckBox == &aCbFixed )
    {
        sal_Bool bEnable = !aCbFixed.IsChecked();
        aFtFieldSep.Enable( bEnable );
        aEdFieldSep.Enable( bEnable );
        aFtTextSep.Enable( bEnable );
        aEdTextSep.Enable( bEnable );
        aCbShown.Enable( bEnable );
        aCbQuoteAll.Enable( bEnable );
    }
    return 0;
}

 IMPL_LINK( ScImportOptionsDlg, DoubleClickHdl, ListBox*, pLb )
{
    if ( pLb == &aLbFont )
    {
        aBtnOk.Click();
    }
    return 0;
}
