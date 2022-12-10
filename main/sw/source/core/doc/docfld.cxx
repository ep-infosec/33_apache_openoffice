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


#include <hintids.hxx>

#include <string.h>
#include <float.h>
#include <tools/datetime.hxx>
#ifndef _SVSTDARR_HXX
#define _SVSTDARR_ULONGS
#include <svl/svarray.hxx>
#endif
#include <vcl/svapp.hxx>
#include <vcl/svapp.hxx>
#include <unotools/charclass.hxx>
#include <unotools/transliterationwrapper.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <cntfrm.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swtable.hxx>
#include <calc.hxx>
#include <txtfld.hxx>
#include <fmtfld.hxx>
#include <tox.hxx>
#include <txttxmrk.hxx>
#include <docfld.hxx>   // fuer Expression-Felder
#include <docufld.hxx>
#include <ddefld.hxx>
#include <usrfld.hxx>
#include <expfld.hxx>
#include <dbfld.hxx>
#include <flddat.hxx>
#include <chpfld.hxx>
#include <reffld.hxx>
#include <flddropdown.hxx>
#include <dbmgr.hxx>
#include <section.hxx>
#include <cellatr.hxx>
#include <docary.hxx>
#include <authfld.hxx>
#include <txtinet.hxx>
#include <fmtcntnt.hxx>
#include <poolfmt.hrc>      // fuer InitFldTypes

#include <SwUndoField.hxx>
#include "switerator.hxx"

using namespace ::com::sun::star::uno;

extern sal_Bool IsFrameBehind( const SwTxtNode& rMyNd, sal_uInt16 nMySttPos,
						const SwTxtNode& rBehindNd, sal_uInt16 nSttPos );

SV_IMPL_OP_PTRARR_SORT( _SetGetExpFlds, _SetGetExpFldPtr )


/*--------------------------------------------------------------------
	Beschreibung: Feldtypen einfuegen
 --------------------------------------------------------------------*/
/*
 *	Implementierung der Feldfunktionen am Doc
 *	Return immer einen gueltigen Pointer auf den Typ. Wenn er also neu
 *	zugefuegt oder schon vorhanden ist.
 */

SwFieldType* SwDoc::InsertFldType(const SwFieldType &rFldTyp)
{
	sal_uInt16 nSize = pFldTypes->Count(),
			nFldWhich = rFldTyp.Which();

	sal_uInt16 i = INIT_FLDTYPES;

	switch( nFldWhich )
	{
	case RES_SETEXPFLD:
			//JP 29.01.96: SequenceFelder beginnen aber bei INIT_FLDTYPES - 3!!
			//			   Sonst gibt es doppelte Nummernkreise!!
			//MIB 14.03.95: Ab sofort verlaesst sich auch der SW3-Reader
			//beim Aufbau der String-Pools und beim Einlesen von SetExp-Feldern
			//hierauf
			if( nsSwGetSetExpType::GSE_SEQ & ((SwSetExpFieldType&)rFldTyp).GetType() )
				i -= INIT_SEQ_FLDTYPES;
		// kein break;
	case RES_DBFLD:
	case RES_USERFLD:
	case RES_DDEFLD:
		{
			const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
			String sFldNm( rFldTyp.GetName() );
			for( ; i < nSize; ++i )
				if( nFldWhich == (*pFldTypes)[i]->Which() &&
					rSCmp.isEqual( sFldNm, (*pFldTypes)[i]->GetName() ))
						return (*pFldTypes)[i];
		}
		break;

	case RES_AUTHORITY:
		for( ; i < nSize; ++i )
			if( nFldWhich == (*pFldTypes)[i]->Which() )
				return (*pFldTypes)[i];
		break;

	default:
		for( i = 0; i < nSize; ++i )
			if( nFldWhich == (*pFldTypes)[i]->Which() )
				return (*pFldTypes)[i];
	}

	SwFieldType* pNew = rFldTyp.Copy();
	switch( nFldWhich )
	{
	case RES_DDEFLD:
		((SwDDEFieldType*)pNew)->SetDoc( this );
		break;

	case RES_DBFLD:
	case RES_TABLEFLD:
	case RES_DATETIMEFLD:
	case RES_GETEXPFLD:
		((SwValueFieldType*)pNew)->SetDoc( this );
		break;

	case RES_USERFLD:
	case RES_SETEXPFLD:
		((SwValueFieldType*)pNew)->SetDoc( this );
		// JP 29.07.96: opt. FeldListe fuer den Calculator vorbereiten:
		pUpdtFlds->InsertFldType( *pNew );
		break;
	case RES_AUTHORITY :
		((SwAuthorityFieldType*)pNew)->SetDoc( this );
		break;
	}

	pFldTypes->Insert( pNew, nSize );
	SetModified();

	return (*pFldTypes)[ nSize ];
}

void SwDoc::InsDeletedFldType( SwFieldType& rFldTyp )
{
	// der FeldTyp wurde als geloescht gekennzeichnet und aus dem
	// Array entfernt. Nun muss man nach diesem wieder suchen.
	// - Ist der nicht vorhanden, dann kann er eingefuegt werden.
	// - Wird genau der gleiche Typ gefunden, dann muss der geloeschte
	//	 einen anderen Namen erhalten.

	sal_uInt16 nSize = pFldTypes->Count(), nFldWhich = rFldTyp.Which();
	sal_uInt16 i = INIT_FLDTYPES;

	ASSERT( RES_SETEXPFLD == nFldWhich ||
			RES_USERFLD == nFldWhich ||
			RES_DDEFLD == nFldWhich, "Falscher FeldTyp" );

	const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
	const String& rFldNm = rFldTyp.GetName();
	SwFieldType* pFnd;

	for( ; i < nSize; ++i )
		if( nFldWhich == (pFnd = (*pFldTypes)[i])->Which() &&
			rSCmp.isEqual( rFldNm, pFnd->GetName() ) )
		{
			// neuen Namen suchen
			sal_uInt16 nNum = 1;
			do {
				String sSrch( rFldNm );
				sSrch.Append( String::CreateFromInt32( nNum ));
				for( i = INIT_FLDTYPES; i < nSize; ++i )
					if( nFldWhich == (pFnd = (*pFldTypes)[i])->Which() &&
						rSCmp.isEqual( sSrch, pFnd->GetName() ) )
						break;

				if( i >= nSize )		// nicht gefunden
				{
					((String&)rFldNm) = sSrch;
					break;		// raus aus der While-Schleife
				}
				++nNum;
			} while( sal_True );
			break;
		}

	// nicht gefunden, also eintragen und Flag loeschen
	pFldTypes->Insert( &rFldTyp, nSize );
	switch( nFldWhich )
	{
	case RES_SETEXPFLD:
		((SwSetExpFieldType&)rFldTyp).SetDeleted( sal_False );
		break;
	case RES_USERFLD:
		((SwUserFieldType&)rFldTyp).SetDeleted( sal_False );
		break;
	case RES_DDEFLD:
		((SwDDEFieldType&)rFldTyp).SetDeleted( sal_False );
		break;
	}
}

/*--------------------------------------------------------------------
	Beschreibung: Feldtypen loeschen
 --------------------------------------------------------------------*/

void SwDoc::RemoveFldType(sal_uInt16 nFld)
{
	ASSERT( INIT_FLDTYPES <= nFld,	"keine InitFields loeschen" );
	/*
	 * Abheangige Felder vorhanden -> ErrRaise
	 */
	sal_uInt16 nSize = pFldTypes->Count();
	if(nFld < nSize)
	{
		SwFieldType* pTmp = (*pFldTypes)[nFld];

		// JP 29.07.96: opt. FeldListe fuer den Calculator vorbereiten:
		sal_uInt16 nWhich = pTmp->Which();
		switch( nWhich )
		{
		case RES_SETEXPFLD:
		case RES_USERFLD:
			pUpdtFlds->RemoveFldType( *pTmp );
			// kein break;
		case RES_DDEFLD:
			if( pTmp->GetDepends() && !IsUsed( *pTmp ) )
			{
				if( RES_SETEXPFLD == nWhich )
					((SwSetExpFieldType*)pTmp)->SetDeleted( sal_True );
				else if( RES_USERFLD == nWhich )
					((SwUserFieldType*)pTmp)->SetDeleted( sal_True );
				else
					((SwDDEFieldType*)pTmp)->SetDeleted( sal_True );
				nWhich = 0;
			}
			break;
		}

		if( nWhich )
		{
			ASSERT( !pTmp->GetDepends(), "Abhaengige vorh.!" );
			// Feldtype loschen
			delete pTmp;
		}
		pFldTypes->Remove( nFld );
		SetModified();
	}
}

const SwFldTypes* SwDoc::GetFldTypes() const
{
    return pFldTypes;
}

/*--------------------------------------------------------------------
	Beschreibung: Den ersten Typen mit ResId und Namen finden
 --------------------------------------------------------------------*/

SwFieldType* SwDoc::GetFldType(
    sal_uInt16 nResId,
    const String& rName,
    bool bDbFieldMatching // used in some UNO calls for RES_DBFLD to use different string matching code #i51815#
    ) const
{
	sal_uInt16 nSize = pFldTypes->Count(), i = 0;
	const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();

	switch( nResId )
	{
	case RES_SETEXPFLD:
			//JP 29.01.96: SequenceFelder beginnen aber bei INIT_FLDTYPES - 3!!
			//			   Sonst gibt es doppelte Nummernkreise!!
			//MIB 14.03.95: Ab sofort verlaesst sich auch der SW3-Reader
			//beim Aufbau der String-Pools und beim Einlesen von SetExp-Feldern
			//hierauf
		i = INIT_FLDTYPES - INIT_SEQ_FLDTYPES;
		break;

	case RES_DBFLD:
	case RES_USERFLD:
	case RES_DDEFLD:
	case RES_AUTHORITY:
		i = INIT_FLDTYPES;
		break;
	}

	SwFieldType* pRet = 0;
	for( ; i < nSize; ++i )
	{
		SwFieldType* pFldType = (*pFldTypes)[i];

		String aFldName( pFldType->GetName() );
		if (bDbFieldMatching && nResId == RES_DBFLD)	// #i51815#
			aFldName.SearchAndReplaceAll(DB_DELIM, '.');

		if( nResId == pFldType->Which() &&
			rSCmp.isEqual( rName, aFldName ))
		{
			pRet = pFldType;
			break;
		}
	}
	return pRet;
}


/*************************************************************************
|*				  SwDoc::UpdateFlds()
|*	  Beschreibung		Felder updaten
*************************************************************************/
/*
 *	  Alle sollen neu evaluiert werden.
 */

void SwDoc::UpdateFlds( SfxPoolItem *pNewHt, bool bCloseDB )
{
    // Modify() fuer jeden Feldtypen rufen,
	// abhaengige SwTxtFld werden benachrichtigt ...

	for( sal_uInt16 i=0; i < pFldTypes->Count(); ++i)
	{
		switch( (*pFldTypes)[i]->Which() )
		{
			// Tabellen-Felder als vorletztes Updaten
			// Referenzen als letztes Updaten
		case RES_GETREFFLD:
		case RES_TABLEFLD:
		case RES_DBFLD:
		case RES_JUMPEDITFLD:
		case RES_REFPAGESETFLD: 	// werden nie expandiert!
			break;

		case RES_DDEFLD:
		{
			if( !pNewHt )
			{
				SwMsgPoolItem aUpdateDDE( RES_UPDATEDDETBL );
				(*pFldTypes)[i]->ModifyNotification( 0, &aUpdateDDE );
			}
			else
				(*pFldTypes)[i]->ModifyNotification( 0, pNewHt );
			break;
		}
		case RES_GETEXPFLD:
		case RES_SETEXPFLD:
		case RES_HIDDENTXTFLD:
		case RES_HIDDENPARAFLD:
			// Expression-Felder werden gesondert behandelt
			if( !pNewHt )
				break;
		default:
			(*pFldTypes)[i]->ModifyNotification ( 0, pNewHt );
		}
	}

	if( !IsExpFldsLocked() )
		UpdateExpFlds( 0, sal_False );		// Expression-Felder Updaten

	// Tabellen
	UpdateTblFlds(pNewHt);

	// Referenzen
	UpdateRefFlds(pNewHt);

	if( bCloseDB )
		GetNewDBMgr()->CloseAll();

	// Nur bei KomplettUpdate evaluieren
	SetModified();
}

/******************************************************************************
 *						void SwDoc::UpdateUsrFlds()
 ******************************************************************************/

void SwDoc::UpdateUsrFlds()
{
	SwCalc* pCalc = 0;
	const SwFieldType* pFldType;
	for( sal_uInt16 i = INIT_FLDTYPES; i < pFldTypes->Count(); ++i )
		if( RES_USERFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
		{
			if( !pCalc )
				pCalc = new SwCalc( *this );
			((SwUserFieldType*)pFldType)->GetValue( *pCalc );
		}

	if( pCalc )
	{
		delete pCalc;
		SetModified();
	}
}

/*--------------------------------------------------------------------
	Beschreibung: Referenzfelder und TableFelder erneuern
 --------------------------------------------------------------------*/

void SwDoc::UpdateRefFlds( SfxPoolItem* pHt )
{
	SwFieldType* pFldType;
	for( sal_uInt16 i = 0; i < pFldTypes->Count(); ++i )
		if( RES_GETREFFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
			pFldType->ModifyNotification( 0, pHt );
}

void SwDoc::UpdateTblFlds( SfxPoolItem* pHt )
{
	ASSERT( !pHt || RES_TABLEFML_UPDATE  == pHt->Which(),
			"Was ist das fuer ein MessageItem?" );

	SwFieldType* pFldType(0);

	for (sal_uInt16 i = 0; i < pFldTypes->Count(); ++i)
	{
		if( RES_TABLEFLD == ( pFldType = (*pFldTypes)[i] )->Which() )
		{
			SwTableFmlUpdate* pUpdtFld = 0;
			if( pHt && RES_TABLEFML_UPDATE == pHt->Which() )
				pUpdtFld = (SwTableFmlUpdate*)pHt;

			SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
			for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
            {
				if( pFmtFld->GetTxtFld() )
				{
					SwTblField* pFld = (SwTblField*)pFmtFld->GetField();

					if( pUpdtFld )
					{
						// bestimme Tabelle, in der das Feld steht
						const SwTableNode* pTblNd;
						const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
						if( !rTxtNd.GetNodes().IsDocNodes() ||
							0 == ( pTblNd = rTxtNd.FindTableNode() ) )
							continue;

						switch( pUpdtFld->eFlags )
						{
						case TBL_CALC:
							// setze das Value-Flag zurueck
							// JP 17.06.96: interne Darstellung auf alle Formeln
							//				(Referenzen auf andere Tabellen!!!)
							if( nsSwExtendedSubType::SUB_CMD & pFld->GetSubType() )
								pFld->PtrToBoxNm( pUpdtFld->pTbl );
							else
								pFld->ChgValid( sal_False );
							break;
						case TBL_BOXNAME:
							// ist es die gesuchte Tabelle ??
							if( &pTblNd->GetTable() == pUpdtFld->pTbl )
								// zur externen Darstellung
								pFld->PtrToBoxNm( pUpdtFld->pTbl );
							break;
						case TBL_BOXPTR:
							// zur internen Darstellung
							// JP 17.06.96: interne Darstellung auf alle Formeln
							//				(Referenzen auf andere Tabellen!!!)
							pFld->BoxNmToPtr( pUpdtFld->pTbl );
							break;
						case TBL_RELBOXNAME:
							// ist es die gesuchte Tabelle ??
							if( &pTblNd->GetTable() == pUpdtFld->pTbl )
								// zur relativen Darstellung
								pFld->ToRelBoxNm( pUpdtFld->pTbl );
							break;
						default:
							break;
						}
					}
					else
						// setze bei allen das Value-Flag zurueck
						pFld->ChgValid( sal_False );
				}
            }

			break;
		}
		pFldType = 0;
	}

	// und dann noch alle Tabellen Box Formeln abklappern
	const SfxPoolItem* pItem;
	sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_BOXATR_FORMULA );
    for (sal_uInt32 i = 0; i < nMaxItems; ++i)
    {
		if( 0 != (pItem = GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
			((SwTblBoxFormula*)pItem)->GetDefinedIn() )
		{
			((SwTblBoxFormula*)pItem)->ChangeState( pHt );
		}
    }


	// alle Felder/Boxen sind jetzt invalide, also kann das Rechnen anfangen
	if( pHt && ( RES_TABLEFML_UPDATE != pHt->Which() ||
				TBL_CALC != ((SwTableFmlUpdate*)pHt)->eFlags ))
		return ;

	SwCalc* pCalc = 0;

	if( pFldType )
	{
		SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
		for( SwFmtFld* pFmtFld = aIter.Last(); pFmtFld; pFmtFld = aIter.Previous() )
        {
                // start calculation at the end
                // new fields are inserted at the beginning of the modify chain
                // that gives faster calculation on import
                // mba: do we really need this "optimization"? Is it still valid?
				SwTblField* pFld;
				if( !pFmtFld->GetTxtFld() || (nsSwExtendedSubType::SUB_CMD &
					(pFld = (SwTblField*)pFmtFld->GetField())->GetSubType() ))
					continue;

				// muss neu berechnet werden (und ist keine textuelle Anzeige)
				if( !pFld->IsValid() )
				{
					// bestimme Tabelle, in der das Feld steht
					const SwTxtNode& rTxtNd = pFmtFld->GetTxtFld()->GetTxtNode();
					if( !rTxtNd.GetNodes().IsDocNodes() )
						continue;
					const SwTableNode* pTblNd = rTxtNd.FindTableNode();
					if( !pTblNd )
						continue;

					// falls dieses Feld nicht in der zu updatenden
					// Tabelle steht, ueberspringen !!
					if( pHt && &pTblNd->GetTable() !=
											((SwTableFmlUpdate*)pHt)->pTbl )
						continue;

					if( !pCalc )
						pCalc = new SwCalc( *this );

					// bestimme die Werte aller SetExpresion Felder, die
					// bis zur Tabelle gueltig sind
					SwFrm* pFrm = 0;
					if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
					{
						// steht im Sonderbereich, wird teuer !!
						Point aPt;		// den im Layout 1. Frame returnen - Tab.Kopfzeile !!
						pFrm = rTxtNd.getLayoutFrm( GetCurrentLayout(), &aPt );
						if( pFrm )
						{
							SwPosition aPos( *pTblNd );
							if( GetBodyTxtNode( *this, aPos, *pFrm ) )
								FldsToCalc( *pCalc, _SetGetExpFld(
									aPos.nNode, pFmtFld->GetTxtFld(),
									&aPos.nContent ));
							else
								pFrm = 0;
						}
					}
					if( !pFrm )
					{
						// einen Index fuers bestimmen vom TextNode anlegen
						SwNodeIndex aIdx( rTxtNd );
						FldsToCalc( *pCalc,
							_SetGetExpFld( aIdx, pFmtFld->GetTxtFld() ));
					}

					SwTblCalcPara aPara( *pCalc, pTblNd->GetTable() );
					pFld->CalcField( aPara );
					if( aPara.IsStackOverFlow() )
					{
						if( aPara.CalcWithStackOverflow() )
							pFld->CalcField( aPara );
#ifdef DBG_UTIL
						else
						{
							// mind. ein ASSERT
							ASSERT( sal_False, "the chain formula could not be calculated" );
						}
#endif
					}
					pCalc->SetCalcError( CALC_NOERR );
				}
				pFmtFld->ModifyNotification( 0, pHt );
        } 
	}

	// dann berechene noch die Formeln an den Boxen
    for (sal_uInt32 i = 0; i < nMaxItems; ++i )
    {
		if( 0 != (pItem = GetAttrPool().GetItem2( RES_BOXATR_FORMULA, i ) ) &&
			((SwTblBoxFormula*)pItem)->GetDefinedIn() &&
			!((SwTblBoxFormula*)pItem)->IsValid() )
		{
			SwTblBoxFormula* pFml = (SwTblBoxFormula*)pItem;
			SwTableBox* pBox = pFml->GetTableBox();
			if( pBox && pBox->GetSttNd() &&
				pBox->GetSttNd()->GetNodes().IsDocNodes() )
			{
				const SwTableNode* pTblNd = pBox->GetSttNd()->FindTableNode();
				if( !pHt || &pTblNd->GetTable() ==
											((SwTableFmlUpdate*)pHt)->pTbl )
				{
					double nValue;
					if( !pCalc )
						pCalc = new SwCalc( *this );

					// bestimme die Werte aller SetExpresion Felder, die
					// bis zur Tabelle gueltig sind
					SwFrm* pFrm = 0;
					if( pTblNd->GetIndex() < GetNodes().GetEndOfExtras().GetIndex() )
					{
						// steht im Sonderbereich, wird teuer !!
						Point aPt;		// den im Layout 1. Frame returnen - Tab.Kopfzeile !!
						SwNodeIndex aCNdIdx( *pTblNd, +2 );
						SwCntntNode* pCNd = aCNdIdx.GetNode().GetCntntNode();
						if( !pCNd )
							pCNd = GetNodes().GoNext( &aCNdIdx );

						if( pCNd && 0 != (pFrm = pCNd->getLayoutFrm( GetCurrentLayout(), &aPt )) )
						{
							SwPosition aPos( *pCNd );
							if( GetBodyTxtNode( *this, aPos, *pFrm ) )
								FldsToCalc( *pCalc, _SetGetExpFld( aPos.nNode ));
							else
								pFrm = 0;
						}
					}
					if( !pFrm )
					{
						// einen Index fuers bestimmen vom TextNode anlegen
						SwNodeIndex aIdx( *pTblNd );
						FldsToCalc( *pCalc, _SetGetExpFld( aIdx ));
					}

					SwTblCalcPara aPara( *pCalc, pTblNd->GetTable() );
					pFml->Calc( aPara, nValue );

					if( aPara.IsStackOverFlow() )
					{
						if( aPara.CalcWithStackOverflow() )
							pFml->Calc( aPara, nValue );
#ifdef DBG_UTIL
						else
						{
							// mind. ein ASSERT
							ASSERT( sal_False, "the chain formula could not be calculated" );
						}
#endif
					}

					SwFrmFmt* pFmt = pBox->ClaimFrmFmt();
					SfxItemSet aTmp( GetAttrPool(),
									RES_BOXATR_BEGIN,RES_BOXATR_END-1 );

					if( pCalc->IsCalcError() )
						nValue = DBL_MAX;
					aTmp.Put( SwTblBoxValue( nValue ));
					if( SFX_ITEM_SET != pFmt->GetItemState( RES_BOXATR_FORMAT ))
						aTmp.Put( SwTblBoxNumFormat( 0 ));
                    pFmt->SetFmtAttr( aTmp );

					pCalc->SetCalcError( CALC_NOERR );
				}
			}
		}
    }

	if( pCalc )
		delete pCalc;
}

void SwDoc::UpdatePageFlds( SfxPoolItem* pMsgHnt )
{
	SwFieldType* pFldType;
	for( sal_uInt16 i = 0; i < INIT_FLDTYPES; ++i )
		switch( ( pFldType = (*pFldTypes)[ i ] )->Which() )
		{
		case RES_PAGENUMBERFLD:
		case RES_CHAPTERFLD:
		case RES_GETEXPFLD:
		case RES_REFPAGEGETFLD:
			pFldType->ModifyNotification( 0, pMsgHnt );
			break;
		case RES_DOCSTATFLD:
			pFldType->ModifyNotification( 0, 0 );
			break;
		}
	SetNewFldLst(true);
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

// ---- Loesche alle nicht referenzierten FeldTypen eines Dokumentes --
void SwDoc::GCFieldTypes()
{
	for( sal_uInt16 n = pFldTypes->Count(); n > INIT_FLDTYPES; )
		if( !(*pFldTypes)[ --n ]->GetDepends() )
			RemoveFldType( n );
}

void SwDoc::LockExpFlds()
{
    ++nLockExpFld;
}

void SwDoc::UnlockExpFlds()
{
    if( nLockExpFld )
        --nLockExpFld;
}

bool SwDoc::IsExpFldsLocked() const
{
    return 0 != nLockExpFld;
}

SwDocUpdtFld& SwDoc::GetUpdtFlds() const
{
    return *pUpdtFlds;
}

bool SwDoc::IsNewFldLst() const
{
    return mbNewFldLst;
}

void SwDoc::SetNewFldLst(bool bFlag)
{
    mbNewFldLst = bFlag;
}


//----------------------------------------------------------------------

// der StartIndex kann optional mit angegeben werden (z.B. wenn dieser
// zuvor schon mal erfragt wurde - ist sonst eine virtuelle Methode !!)

_SetGetExpFld::_SetGetExpFld(
    const SwNodeIndex& rNdIdx,
    const SwTxtFld* pFld,
    const SwIndex* pIdx )
{
    eSetGetExpFldType = TEXTFIELD;
    CNTNT.pTxtFld = pFld;
    nNode = rNdIdx.GetIndex();
    if( pIdx )
        nCntnt = pIdx->GetIndex();
    else if( pFld )
        nCntnt = *pFld->GetStart();
    else
        nCntnt = 0;
}

_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx,
							const SwTxtINetFmt& rINet, const SwIndex* pIdx )
{
	eSetGetExpFldType = TEXTINET;
	CNTNT.pTxtINet = &rINet;
	nNode = rNdIdx.GetIndex();
	if( pIdx )
		nCntnt = pIdx->GetIndex();
	else
		nCntnt = *rINet.GetStart();
}

	//Erweiterung fuer Sections:
	//	diese haben immer als Content-Position 0xffff !!
	//	Auf dieser steht nie ein Feld, maximal bis STRING_MAXLEN moeglich
_SetGetExpFld::_SetGetExpFld( const SwSectionNode& rSectNd,
								const SwPosition* pPos )
{
	eSetGetExpFldType = SECTIONNODE;
	CNTNT.pSection = &rSectNd.GetSection();

	if( pPos )
	{
		nNode = pPos->nNode.GetIndex();
		nCntnt = pPos->nContent.GetIndex();
	}
	else
	{
		nNode = rSectNd.GetIndex();
		nCntnt = 0;
	}
}

_SetGetExpFld::_SetGetExpFld( const SwTableBox& rTBox, const SwPosition* pPos )
{
	eSetGetExpFldType = TABLEBOX;
	CNTNT.pTBox = &rTBox;

	if( pPos )
	{
		nNode = pPos->nNode.GetIndex();
		nCntnt = pPos->nContent.GetIndex();
	}
	else
	{
		nNode = 0;
		nCntnt = 0;
		if( rTBox.GetSttNd() )
		{
			SwNodeIndex aIdx( *rTBox.GetSttNd() );
			const SwCntntNode* pNd = aIdx.GetNode().GetNodes().GoNext( &aIdx );
			if( pNd )
				nNode = pNd->GetIndex();
		}
	}
}

_SetGetExpFld::_SetGetExpFld( const SwNodeIndex& rNdIdx,
								const SwTxtTOXMark& rTOX,
								const SwIndex* pIdx )
{
	eSetGetExpFldType = TEXTTOXMARK;
	CNTNT.pTxtTOX = &rTOX;
	nNode = rNdIdx.GetIndex();
	if( pIdx )
		nCntnt = pIdx->GetIndex();
	else
		nCntnt = *rTOX.GetStart();
}

_SetGetExpFld::_SetGetExpFld( const SwPosition& rPos )
{
	eSetGetExpFldType = CRSRPOS;
	CNTNT.pPos = &rPos;
	nNode = rPos.nNode.GetIndex();
	nCntnt = rPos.nContent.GetIndex();
}

_SetGetExpFld::_SetGetExpFld( const SwFlyFrmFmt& rFlyFmt,
								const SwPosition* pPos  )
{
	eSetGetExpFldType = FLYFRAME;
	CNTNT.pFlyFmt = &rFlyFmt;
	if( pPos )
	{
		nNode = pPos->nNode.GetIndex();
		nCntnt = pPos->nContent.GetIndex();
	}
	else
	{
		const SwFmtCntnt& rCntnt = rFlyFmt.GetCntnt();
		nNode = rCntnt.GetCntntIdx()->GetIndex() + 1;
		nCntnt = 0;
	}
}

void _SetGetExpFld::GetPos( SwPosition& rPos ) const
{
	rPos.nNode = nNode;
	rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
}

void _SetGetExpFld::GetPosOfContent( SwPosition& rPos ) const
{
	const SwNode* pNd = GetNodeFromCntnt();
	if( pNd )
		pNd = pNd->GetCntntNode();

	if( pNd )
	{
		rPos.nNode = *pNd;
		rPos.nContent.Assign( (SwCntntNode*)pNd,GetCntPosFromCntnt() );
	}
	else
	{
		rPos.nNode = nNode;
		rPos.nContent.Assign( rPos.nNode.GetNode().GetCntntNode(), nCntnt );
	}
}

void _SetGetExpFld::SetBodyPos( const SwCntntFrm& rFrm )
{
	if( !rFrm.IsInDocBody() )
	{
		SwNodeIndex aIdx( *rFrm.GetNode() );
		SwDoc& rDoc = *aIdx.GetNodes().GetDoc();
		SwPosition aPos( aIdx );
#ifdef DBG_UTIL
		ASSERT( ::GetBodyTxtNode( rDoc, aPos, rFrm ), "wo steht das Feld" );
#else
		::GetBodyTxtNode( rDoc, aPos, rFrm );
#endif
		nNode = aPos.nNode.GetIndex();
		nCntnt = aPos.nContent.GetIndex();
	}
}

sal_Bool _SetGetExpFld::operator==( const _SetGetExpFld& rFld ) const
{
    return nNode == rFld.nNode
           && nCntnt == rFld.nCntnt
           && ( !CNTNT.pTxtFld
                || !rFld.CNTNT.pTxtFld
                || CNTNT.pTxtFld == rFld.CNTNT.pTxtFld );
}

sal_Bool _SetGetExpFld::operator<( const _SetGetExpFld& rFld ) const
{
	if( nNode < rFld.nNode || ( nNode == rFld.nNode && nCntnt < rFld.nCntnt ))
		return sal_True;
	else if( nNode != rFld.nNode || nCntnt != rFld.nCntnt )
		return sal_False;

	const SwNode *pFirst = GetNodeFromCntnt(),
				 *pNext = rFld.GetNodeFromCntnt();

	// Position gleich: nur weiter wenn beide FeldPointer besetzt sind !!
	if( !pFirst || !pNext )
		return sal_False;

	// gleiche Section ??
	if( pFirst->StartOfSectionNode() != pNext->StartOfSectionNode() )
	{
		// sollte einer in der Tabelle stehen ?
		const SwNode *pFirstStt, *pNextStt;
		const SwTableNode* pTblNd = pFirst->FindTableNode();
		if( pTblNd )
			pFirstStt = pTblNd->StartOfSectionNode();
		else
			pFirstStt = pFirst->StartOfSectionNode();

		if( 0 != ( pTblNd = pNext->FindTableNode() ) )
			pNextStt = pTblNd->StartOfSectionNode();
		else
			pNextStt = pNext->StartOfSectionNode();

		if( pFirstStt != pNextStt )
		{
			if( pFirst->IsTxtNode() && pNext->IsTxtNode() &&
				( pFirst->FindFlyStartNode() || pNext->FindFlyStartNode() ))
			{
				return ::IsFrameBehind( *(SwTxtNode*)pNext, nCntnt,
										*(SwTxtNode*)pFirst, nCntnt );
			}
			return pFirstStt->GetIndex() < pNextStt->GetIndex();
		}
	}

	// ist gleiche Section, dann Feld im gleichen Node ?
	if( pFirst != pNext )
		return pFirst->GetIndex() < pNext->GetIndex();

	// gleicher Node in der Section, dann Position im Node
	return GetCntPosFromCntnt() < rFld.GetCntPosFromCntnt();
}

const SwNode* _SetGetExpFld::GetNodeFromCntnt() const
{
	const SwNode* pRet = 0;
	if( CNTNT.pTxtFld )
		switch( eSetGetExpFldType )
		{
		case TEXTFIELD:
			pRet = &CNTNT.pTxtFld->GetTxtNode();
			break;

		case TEXTINET:
			pRet = &CNTNT.pTxtINet->GetTxtNode();
			break;

		case SECTIONNODE:
			pRet = CNTNT.pSection->GetFmt()->GetSectionNode();
			break;

		case CRSRPOS:
			pRet = &CNTNT.pPos->nNode.GetNode();
			break;

		case TEXTTOXMARK:
			pRet = &CNTNT.pTxtTOX->GetTxtNode();
			break;

		case TABLEBOX:
			if( CNTNT.pTBox->GetSttNd() )
			{
				SwNodeIndex aIdx( *CNTNT.pTBox->GetSttNd() );
				pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
			}
			break;

		case FLYFRAME:
			{
				SwNodeIndex aIdx( *CNTNT.pFlyFmt->GetCntnt().GetCntntIdx() );
				pRet = aIdx.GetNode().GetNodes().GoNext( &aIdx );
			}
			break;
		}
	return pRet;
}

xub_StrLen _SetGetExpFld::GetCntPosFromCntnt() const
{
	sal_uInt16 nRet = 0;
	if( CNTNT.pTxtFld )
		switch( eSetGetExpFldType )
		{
		case TEXTFIELD:
		case TEXTINET:
		case TEXTTOXMARK:
			nRet = *CNTNT.pTxtFld->GetStart();
			break;
		case CRSRPOS:
			nRet =  CNTNT.pPos->nContent.GetIndex();
			break;
		default:
			break;
		}
	return nRet;
}

_HashStr::_HashStr( const String& rName, const String& rText,
					_HashStr* pNxt )
	: SwHash( rName ), aSetStr( rText )
{
	pNext = pNxt;
}

// suche nach dem Namen, ist er vorhanden, returne seinen String, sonst
// einen LeerString
void LookString( SwHash** ppTbl, sal_uInt16 nSize, const String& rName,
					String& rRet, sal_uInt16* pPos )
{
	rRet = rName;
	rRet.EraseLeadingChars().EraseTrailingChars();
	SwHash* pFnd = Find( rRet, ppTbl, nSize, pPos );
	if( pFnd )
		rRet = ((_HashStr*)pFnd)->aSetStr;
	else
		rRet.Erase();
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

String lcl_GetDBVarName( SwDoc& rDoc, SwDBNameInfField& rDBFld )
{
	SwDBData aDBData( rDBFld.GetDBData( &rDoc ));
	String sDBNumNm;
	SwDBData aDocData = rDoc.GetDBData();

	if( aDBData != aDocData )
	{
		sDBNumNm = aDBData.sDataSource;
		sDBNumNm += DB_DELIM;
		sDBNumNm += String(aDBData.sCommand);
		sDBNumNm += DB_DELIM;
	}
	sDBNumNm += SwFieldType::GetTypeStr(TYP_DBSETNUMBERFLD);

	return sDBNumNm;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void lcl_CalcFld( SwDoc& rDoc, SwCalc& rCalc, const _SetGetExpFld& rSGEFld,
						SwNewDBMgr* pMgr )
{
	const SwTxtFld* pTxtFld = rSGEFld.GetTxtFld();
	if( !pTxtFld )
		return ;

	const SwField* pFld = pTxtFld->GetFmtFld().GetField();
	const sal_uInt16 nFldWhich = pFld->GetTyp()->Which();

	if( RES_SETEXPFLD == nFldWhich )
	{
		SwSbxValue aValue;
		if( nsSwGetSetExpType::GSE_EXPR & pFld->GetSubType() )
			aValue.PutDouble( ((SwSetExpField*)pFld)->GetValue() );
		else
			// Erweiterung fuers Rechnen mit Strings
			aValue.PutString( ((SwSetExpField*)pFld)->GetExpStr() );

		// setze im Calculator den neuen Wert
		rCalc.VarChange( pFld->GetTyp()->GetName(), aValue );
	}
	else if( pMgr )
	{
		switch( nFldWhich )
		{
		case RES_DBNUMSETFLD:
			{
				SwDBNumSetField* pDBFld = (SwDBNumSetField*)pFld;

				SwDBData aDBData(pDBFld->GetDBData(&rDoc));

				if( pDBFld->IsCondValid() &&
					pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
					rCalc.VarChange( lcl_GetDBVarName( rDoc, *pDBFld),
									pDBFld->GetFormat() );
			}
			break;
		case RES_DBNEXTSETFLD:
			{
				SwDBNextSetField* pDBFld = (SwDBNextSetField*)pFld;
				SwDBData aDBData(pDBFld->GetDBData(&rDoc));
				if( !pDBFld->IsCondValid() ||
					!pMgr->OpenDataSource( aDBData.sDataSource, aDBData.sCommand ))
					break;

				String sDBNumNm(lcl_GetDBVarName( rDoc, *pDBFld));
				SwCalcExp* pExp = rCalc.VarLook( sDBNumNm );
				if( pExp )
					rCalc.VarChange( sDBNumNm, pExp->nValue.GetLong() + 1 );
			}
			break;

		}
	}
}

void SwDoc::FldsToCalc( SwCalc& rCalc, const _SetGetExpFld& rToThisFld )
{
	// erzeuge die Sortierteliste aller SetFelder
	pUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_CALC );
	mbNewFldLst = sal_False;

	SwNewDBMgr* pMgr = GetNewDBMgr();
	pMgr->CloseAll(sal_False);

	if( pUpdtFlds->GetSortLst()->Count() )
	{
		sal_uInt16 nLast;
		_SetGetExpFld* pFld = (_SetGetExpFld*)&rToThisFld;
		if( pUpdtFlds->GetSortLst()->Seek_Entry( pFld, &nLast ) )
			++nLast;

        const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
		for( sal_uInt16 n = 0; n < nLast; ++n, ++ppSortLst )
			lcl_CalcFld( *this, rCalc, **ppSortLst, pMgr );
	}

	pMgr->CloseAll(sal_False);
}

void SwDoc::FldsToCalc( SwCalc& rCalc, sal_uLong nLastNd, sal_uInt16 nLastCnt )
{
	// erzeuge die Sortierteliste aller SetFelder
	pUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_CALC );
	mbNewFldLst = sal_False;

	SwNewDBMgr* pMgr = GetNewDBMgr();
	pMgr->CloseAll(sal_False);

	const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();

	for( sal_uInt16 n = pUpdtFlds->GetSortLst()->Count();
        n &&
        ( (*ppSortLst)->GetNode() < nLastNd ||
          ( (*ppSortLst)->GetNode() == nLastNd && (*ppSortLst)->GetCntnt() <= nLastCnt )
        );
		--n, ++ppSortLst )
		lcl_CalcFld( *this, rCalc, **ppSortLst, pMgr );

	pMgr->CloseAll(sal_False);
}

void SwDoc::FldsToExpand( SwHash**& ppHashTbl, sal_uInt16& rTblSize,
							const _SetGetExpFld& rToThisFld )
{
	// erzeuge die Sortierteliste aller SetFelder
	pUpdtFlds->MakeFldList( *this, mbNewFldLst, GETFLD_EXPAND );
	mbNewFldLst = sal_False;

	// HashTabelle fuer alle String Ersetzungen, wird "one the fly" gefuellt
	// (versuche eine "ungerade"-Zahl zu erzeugen)
	rTblSize = (( pUpdtFlds->GetSortLst()->Count() / 7 ) + 1 ) * 7;
	ppHashTbl = new SwHash*[ rTblSize ];
	memset( ppHashTbl, 0, sizeof( _HashStr* ) * rTblSize );

	sal_uInt16 nLast;
	{
		_SetGetExpFld* pTmp = (_SetGetExpFld*)&rToThisFld;
		if( pUpdtFlds->GetSortLst()->Seek_Entry( pTmp, &nLast ) )
			++nLast;
	}

	sal_uInt16 nPos;
	SwHash* pFnd;
	String aNew;
	const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
	for( ; nLast; --nLast, ++ppSortLst )
	{
		const SwTxtFld* pTxtFld = (*ppSortLst)->GetTxtFld();
		if( !pTxtFld )
			continue;

		const SwField* pFld = pTxtFld->GetFmtFld().GetField();
		switch( pFld->GetTyp()->Which() )
		{
		case RES_SETEXPFLD:
			if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )
			{
				// setze in der HashTabelle den neuen Wert
				// ist die "Formel" ein Feld ??
				SwSetExpField* pSFld = (SwSetExpField*)pFld;
				LookString( ppHashTbl, rTblSize, pSFld->GetFormula(), aNew );

                if( !aNew.Len() )               // nichts gefunden, dann ist
                    aNew = pSFld->GetFormula(); // die Formel der neue Wert

                // OD 11.02.2003 #i3141# - update expression of field as in
                // method <SwDoc::UpdateExpFlds(..)> for string/text fields
                pSFld->ChgExpStr( aNew );

				// suche den Namen vom Feld
				aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
				// Eintrag vorhanden ?
				pFnd = Find( aNew, ppHashTbl, rTblSize, &nPos );
				if( pFnd )
					// Eintrag in der HashTabelle aendern
					((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
				else
					// neuen Eintrag einfuegen
					*(ppHashTbl + nPos ) = new _HashStr( aNew,
							pSFld->GetExpStr(), (_HashStr*)*(ppHashTbl + nPos) );
			}
			break;
		case RES_DBFLD:
			{
				const String& rName = pFld->GetTyp()->GetName();

				// Eintrag in den HashTable eintragen
				// Eintrag vorhanden ?
				pFnd = Find( rName, ppHashTbl, rTblSize, &nPos );
                String const value(pFld->ExpandField(IsClipBoard()));
				if( pFnd )
                {
					// Eintrag in der HashTabelle aendern
                    static_cast<_HashStr*>(pFnd)->aSetStr = value;
                }
                else
                {
					// neuen Eintrag einfuegen
					*(ppHashTbl + nPos ) = new _HashStr( rName,
                        value, static_cast<_HashStr *>(*(ppHashTbl + nPos)));
                }
			}
			break;
		}
	}
}


void SwDoc::UpdateExpFlds( SwTxtFld* pUpdtFld, bool bUpdRefFlds )
{
    if( IsExpFldsLocked() || IsInReading() )
		return;

	sal_Bool bOldInUpdateFlds = pUpdtFlds->IsInUpdateFlds();
	pUpdtFlds->SetInUpdateFlds( sal_True );

	pUpdtFlds->MakeFldList( *this, sal_True, GETFLD_ALL );
	mbNewFldLst = sal_False;

	if( !pUpdtFlds->GetSortLst()->Count() )
	{
		if( bUpdRefFlds )
			UpdateRefFlds(NULL);

		pUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
		pUpdtFlds->SetFieldsDirty( sal_False );
		return ;
	}

	sal_uInt16 nWhich, n;

	// HashTabelle fuer alle String Ersetzungen, wird "one the fly" gefuellt
	// (versuche eine "ungerade"-Zahl zu erzeugen)
	sal_uInt16 nStrFmtCnt = (( pFldTypes->Count() / 7 ) + 1 ) * 7;
	SwHash** pHashStrTbl = new SwHash*[ nStrFmtCnt ];
	memset( pHashStrTbl, 0, sizeof( _HashStr* ) * nStrFmtCnt );

	{
		const SwFieldType* pFldType;
		// gesondert behandeln:
		for( n = pFldTypes->Count(); n; )
			switch( ( pFldType = (*pFldTypes)[ --n ] )->Which() )
			{
			case RES_USERFLD:
				{
					// Eintrag vorhanden ?
					sal_uInt16 nPos;
					const String& rNm = pFldType->GetName();
					String sExpand(((SwUserFieldType*)pFldType)->Expand(nsSwGetSetExpType::GSE_STRING, 0, 0));
					SwHash* pFnd = Find( rNm, pHashStrTbl, nStrFmtCnt, &nPos );
					if( pFnd )
						// Eintrag in der HashTabelle aendern ??
						((_HashStr*)pFnd)->aSetStr = sExpand;
					else
						// neuen Eintrag einfuegen
						*(pHashStrTbl + nPos ) = new _HashStr( rNm, sExpand,
												(_HashStr*)*(pHashStrTbl + nPos) );
				}
				break;
			case RES_SETEXPFLD:
				((SwSetExpFieldType*)pFldType)->SetOutlineChgNd( 0 );
				break;
			}
	}

	// Ok, das Array ist soweit mit allen Feldern gefuellt, dann rechne mal
	SwCalc aCalc( *this );

	String sDBNumNm( SwFieldType::GetTypeStr( TYP_DBSETNUMBERFLD ) );

	// aktuelle Datensatznummer schon vorher einstellen
	SwNewDBMgr* pMgr = GetNewDBMgr();
	pMgr->CloseAll(sal_False);
/*
	if(pMgr && pMgr->OpenDB(DBMGR_STD, GetDBDesc(), sal_False))
	{
		if(!pMgr->IsInMerge() )
			pMgr->ToFirstSelectedRecord(DBMGR_STD);

		aCalc.VarChange( sDBNumNm, pMgr->GetCurSelectedRecordId(DBMGR_STD));
	}
*/

	String aNew;
	const _SetGetExpFldPtr* ppSortLst = pUpdtFlds->GetSortLst()->GetData();
	for( n = pUpdtFlds->GetSortLst()->Count(); n; --n, ++ppSortLst )
	{
		SwSection* pSect = (SwSection*)(*ppSortLst)->GetSection();
		if( pSect )
		{
			//!SECTION

            SwSbxValue aValue = aCalc.Calculate(
                                        pSect->GetCondition() );
            if(!aValue.IsVoidValue())
                pSect->SetCondHidden( aValue.GetBool() );
            continue;
        }

        SwTxtFld* pTxtFld = (SwTxtFld*)(*ppSortLst)->GetTxtFld();
        if( !pTxtFld )
        {
            ASSERT( sal_False, "what is it now?" );
            continue;
        }

        SwFmtFld* pFmtFld = (SwFmtFld*)&pTxtFld->GetFmtFld();
        const SwField* pFld = pFmtFld->GetField();

        switch( nWhich = pFld->GetTyp()->Which() )
        {
        case RES_HIDDENTXTFLD:
        {
            SwHiddenTxtField* pHFld = (SwHiddenTxtField*)pFld;
            SwSbxValue aValue = aCalc.Calculate( pHFld->GetPar1() );
            sal_Bool bValue = !aValue.GetBool();
            if(!aValue.IsVoidValue())
            {
                pHFld->SetValue( bValue );
                // Feld Evaluieren
                pHFld->Evaluate(this);
            }
        }
        break;
        case RES_HIDDENPARAFLD:
        {
            SwHiddenParaField* pHPFld = (SwHiddenParaField*)pFld;
            SwSbxValue aValue = aCalc.Calculate( pHPFld->GetPar1() );
            sal_Bool bValue = aValue.GetBool();
            if(!aValue.IsVoidValue())
                pHPFld->SetHidden( bValue );
		}
		break;
		case RES_DBSETNUMBERFLD:
		{
			((SwDBSetNumberField*)pFld)->Evaluate(this);
			aCalc.VarChange( sDBNumNm, ((SwDBSetNumberField*)pFld)->GetSetNumber());
		}
		break;
		case RES_DBNEXTSETFLD:
		case RES_DBNUMSETFLD:
			UpdateDBNumFlds( *(SwDBNameInfField*)pFld, aCalc );
		break;
		case RES_DBFLD:
		{
			// Feld Evaluieren
			((SwDBField*)pFld)->Evaluate();

			SwDBData aTmpDBData(((SwDBField*)pFld)->GetDBData());

            if( pMgr->IsDataSourceOpen(aTmpDBData.sDataSource, aTmpDBData.sCommand, sal_False))
                aCalc.VarChange( sDBNumNm, pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType));

			const String& rName = pFld->GetTyp()->GetName();

			// Wert fuer den Calculator setzen
//JP 10.02.96: GetValue macht hier doch keinen Sinn
//			((SwDBField*)pFld)->GetValue();

//!OK			aCalc.VarChange(aName, ((SwDBField*)pFld)->GetValue(aCalc));

			// Eintrag in den HashTable eintragen
			// Eintrag vorhanden ?
			sal_uInt16 nPos;
			SwHash* pFnd = Find( rName, pHashStrTbl, nStrFmtCnt, &nPos );
            String const value(pFld->ExpandField(IsClipBoard()));
			if( pFnd )
            {
				// Eintrag in der HashTabelle aendern
                static_cast<_HashStr*>(pFnd)->aSetStr = value;
            }
			else
            {
				// neuen Eintrag einfuegen
				*(pHashStrTbl + nPos ) = new _HashStr( rName,
                    value, static_cast<_HashStr *>(*(pHashStrTbl + nPos)));
            }
		}
		break;
		case RES_GETEXPFLD:
		case RES_SETEXPFLD:
		{
			if( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType() )		// String Ersetzung
			{
				if( RES_GETEXPFLD == nWhich )
				{
					SwGetExpField* pGFld = (SwGetExpField*)pFld;

					if( (!pUpdtFld || pUpdtFld == pTxtFld )
                        && pGFld->IsInBodyTxt() )
					{
						LookString( pHashStrTbl, nStrFmtCnt,
									pGFld->GetFormula(), aNew );
						pGFld->ChgExpStr( aNew );
					}
				}
				else
				{
					SwSetExpField* pSFld = (SwSetExpField*)pFld;
					// ist die "Formel" ein Feld ??
					LookString( pHashStrTbl, nStrFmtCnt,
								pSFld->GetFormula(), aNew );

					if( !aNew.Len() )				// nichts gefunden, dann ist die
						aNew = pSFld->GetFormula();		// Formel der neue Wert

					// nur ein spezielles FeldUpdaten ?
					if( !pUpdtFld || pUpdtFld == pTxtFld )
						pSFld->ChgExpStr( aNew );

					// suche den Namen vom Feld
					aNew = ((SwSetExpFieldType*)pSFld->GetTyp())->GetSetRefName();
					// Eintrag vorhanden ?
					sal_uInt16 nPos;
					SwHash* pFnd = Find( aNew, pHashStrTbl, nStrFmtCnt, &nPos );
					if( pFnd )
						// Eintrag in der HashTabelle aendern
						((_HashStr*)pFnd)->aSetStr = pSFld->GetExpStr();
					else
						// neuen Eintrag einfuegen
						*(pHashStrTbl + nPos ) = pFnd = new _HashStr( aNew,
										pSFld->GetExpStr(),
										(_HashStr*)*(pHashStrTbl + nPos) );

					// Erweiterung fuers Rechnen mit Strings
					SwSbxValue aValue;
					aValue.PutString( ((_HashStr*)pFnd)->aSetStr );
					aCalc.VarChange( aNew, aValue );
				}
			}
			else			// Formel neu berechnen
			{
				if( RES_GETEXPFLD == nWhich )
				{
					SwGetExpField* pGFld = (SwGetExpField*)pFld;

					if( (!pUpdtFld || pUpdtFld == pTxtFld )
						&& pGFld->IsInBodyTxt() )
					{
                        SwSbxValue aValue = aCalc.Calculate(
                                        pGFld->GetFormula());
                        if(!aValue.IsVoidValue())
                            pGFld->SetValue(aValue.GetDouble() );
					}
				}
				else
				{
					SwSetExpField* pSFld = (SwSetExpField*)pFld;
					SwSetExpFieldType* pSFldTyp = (SwSetExpFieldType*)pFld->GetTyp();
					aNew = pSFldTyp->GetName();

					SwNode* pSeqNd = 0;

					if( pSFld->IsSequenceFld() )
					{
                        const sal_uInt8 nLvl = pSFldTyp->GetOutlineLvl();
						if( MAXLEVEL > nLvl )
						{
							// dann teste, ob die Nummer neu aufsetzen muss
							pSeqNd = GetNodes()[ (*ppSortLst)->GetNode() ];

							const SwTxtNode* pOutlNd = pSeqNd->
									FindOutlineNodeOfLevel( nLvl );
							if( pSFldTyp->GetOutlineChgNd() != pOutlNd )
							{
								pSFldTyp->SetOutlineChgNd( pOutlNd );
								aCalc.VarChange( aNew, 0 );
							}
						}
					}

					aNew += '=';
					aNew += pSFld->GetFormula();

                    SwSbxValue aValue = aCalc.Calculate( aNew );
                    double nErg = aValue.GetDouble();
                    // nur ein spezielles Feld updaten ?
                    if( !aValue.IsVoidValue() && (!pUpdtFld || pUpdtFld == pTxtFld) )
					{
						pSFld->SetValue( nErg );

						if( pSeqNd )
							pSFldTyp->SetChapter( *pSFld, *pSeqNd );
					}
				}
			}
		}
		} // switch

		pFmtFld->ModifyNotification( 0, 0 );		// Formatierung anstossen

		if( pUpdtFld == pTxtFld )		// sollte nur dieses geupdatet werden
		{
			if( RES_GETEXPFLD == nWhich ||		// nur GetFeld oder
				RES_HIDDENTXTFLD == nWhich ||	// HiddenTxt?
				RES_HIDDENPARAFLD == nWhich)	// HiddenParaFld?
				break;							// beenden
			pUpdtFld = 0;						// ab jetzt alle Updaten
		}
	}

	pMgr->CloseAll(sal_False);
	// HashTabelle wieder loeschen
	::DeleteHashTable( pHashStrTbl, nStrFmtCnt );

	// Referenzfelder updaten
	if( bUpdRefFlds )
		UpdateRefFlds(NULL);

	pUpdtFlds->SetInUpdateFlds( bOldInUpdateFlds );
	pUpdtFlds->SetFieldsDirty( sal_False );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::UpdateDBNumFlds( SwDBNameInfField& rDBFld, SwCalc& rCalc )
{
	SwNewDBMgr* pMgr = GetNewDBMgr();

	sal_uInt16 nFldType = rDBFld.Which();

	sal_Bool bPar1 = rCalc.Calculate( rDBFld.GetPar1() ).GetBool();

	if( RES_DBNEXTSETFLD == nFldType )
		((SwDBNextSetField&)rDBFld).SetCondValid( bPar1 );
	else
		((SwDBNumSetField&)rDBFld).SetCondValid( bPar1 );

	if( rDBFld.GetRealDBData().sDataSource.getLength() )
	{
		// Eine bestimmte Datenbank bearbeiten
		if( RES_DBNEXTSETFLD == nFldType )
			((SwDBNextSetField&)rDBFld).Evaluate(this);
		else
			((SwDBNumSetField&)rDBFld).Evaluate(this);

		SwDBData aTmpDBData( rDBFld.GetDBData(this) );

        if( pMgr->OpenDataSource( aTmpDBData.sDataSource, aTmpDBData.sCommand, -1, false ))
			rCalc.VarChange( lcl_GetDBVarName( *this, rDBFld),
                        pMgr->GetSelectedRecordId(aTmpDBData.sDataSource, aTmpDBData.sCommand, aTmpDBData.nCommandType) );
	}
	else
	{
		DBG_ERROR("TODO: what should happen with unnamed DBFields?");
	}
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::_InitFieldTypes()		// wird vom CTOR gerufen!!
{
	// Feldtypen
	sal_uInt16 nFldType = 0;
	pFldTypes->Insert( new SwDateTimeFieldType(this), nFldType++ );
	pFldTypes->Insert( new SwChapterFieldType, nFldType++ );
	pFldTypes->Insert( new SwPageNumberFieldType, nFldType++ );
	pFldTypes->Insert( new SwAuthorFieldType, nFldType++ );
	pFldTypes->Insert( new SwFileNameFieldType(this), nFldType++ );
	pFldTypes->Insert( new SwDBNameFieldType(this), nFldType++);
	pFldTypes->Insert( new SwGetExpFieldType(this), nFldType++ );
	pFldTypes->Insert( new SwGetRefFieldType( this ), nFldType++ );
	pFldTypes->Insert( new SwHiddenTxtFieldType, nFldType++ );
	pFldTypes->Insert( new SwPostItFieldType(this), nFldType++ );
	pFldTypes->Insert( new SwDocStatFieldType(this), nFldType++);
	pFldTypes->Insert( new SwDocInfoFieldType(this), nFldType++);
	pFldTypes->Insert( new SwInputFieldType( this ), nFldType++ );
	pFldTypes->Insert( new SwTblFieldType( this ), nFldType++);
	pFldTypes->Insert( new SwMacroFieldType(this), nFldType++ );
	pFldTypes->Insert( new SwHiddenParaFieldType, nFldType++ );
	pFldTypes->Insert( new SwDBNextSetFieldType, nFldType++ );
	pFldTypes->Insert( new SwDBNumSetFieldType, nFldType++ );
	pFldTypes->Insert( new SwDBSetNumberFieldType, nFldType++ );
	pFldTypes->Insert( new SwTemplNameFieldType(this), nFldType++);
	pFldTypes->Insert( new SwTemplNameFieldType(this),nFldType++);
	pFldTypes->Insert( new SwExtUserFieldType, nFldType++ );
	pFldTypes->Insert( new SwRefPageSetFieldType, nFldType++ );
	pFldTypes->Insert( new SwRefPageGetFieldType( this ), nFldType++ );
	pFldTypes->Insert( new SwJumpEditFieldType( this ), nFldType++ );
	pFldTypes->Insert( new SwScriptFieldType( this ), nFldType++ );
	pFldTypes->Insert( new SwCombinedCharFieldType, nFldType++ );
    pFldTypes->Insert( new SwDropDownFieldType, nFldType++ );

	// Types muessen am Ende stehen !!
	// Im InsertFldType wird davon ausgegangen !!!!
	// MIB 14.04.95: Im Sw3StringPool::Setup (sw3imp.cxx) und
	//				 lcl_sw3io_InSetExpField (sw3field.cxx) jetzt auch
	pFldTypes->Insert( new SwSetExpFieldType(this,
				SW_RESSTR(STR_POOLCOLL_LABEL_ABB), nsSwGetSetExpType::GSE_SEQ), nFldType++);
	pFldTypes->Insert( new SwSetExpFieldType(this,
				SW_RESSTR(STR_POOLCOLL_LABEL_TABLE), nsSwGetSetExpType::GSE_SEQ),nFldType++);
	pFldTypes->Insert( new SwSetExpFieldType(this,
				SW_RESSTR(STR_POOLCOLL_LABEL_FRAME), nsSwGetSetExpType::GSE_SEQ),nFldType++);
	pFldTypes->Insert( new SwSetExpFieldType(this,
				SW_RESSTR(STR_POOLCOLL_LABEL_DRAWING), nsSwGetSetExpType::GSE_SEQ),nFldType++);

	ASSERT( nFldType == INIT_FLDTYPES, "Bad initsize: SwFldTypes" );
}

void SwDoc::InsDelFldInFldLst( bool bIns, const SwTxtFld& rFld )
{
	if( !mbNewFldLst || !IsInDtor() )
		pUpdtFlds->InsDelFldInFldLst( bIns, rFld );
}

SwDBData SwDoc::GetDBData()
{
	return GetDBDesc();
}

const SwDBData& SwDoc::GetDBDesc()
{
    if(!aDBData.sDataSource.getLength())
    {
        const sal_uInt16 nSize = pFldTypes->Count();
        for(sal_uInt16 i = 0; i < nSize && !aDBData.sDataSource.getLength(); ++i)
        {
            SwFieldType& rFldType = *((*pFldTypes)[i]);
            sal_uInt16 nWhich = rFldType.Which();
            if(IsUsed(rFldType))
            {
                switch(nWhich)
                {
                    case RES_DBFLD:
                    case RES_DBNEXTSETFLD:
                    case RES_DBNUMSETFLD:
                    case RES_DBSETNUMBERFLD:
                    {
                        SwIterator<SwFmtFld,SwFieldType> aIter( rFldType );
                        for( SwFmtFld* pFld = aIter.First(); pFld; pFld = aIter.Next() )
                        {
                            if(pFld->IsFldInDoc())
                            {
                                if(RES_DBFLD == nWhich)
                                    aDBData = (static_cast < SwDBFieldType * > (pFld->GetField()->GetTyp()))->GetDBData();
                                else
                                    aDBData = (static_cast < SwDBNameInfField* > (pFld->GetField()))->GetRealDBData();
                                break;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    if(!aDBData.sDataSource.getLength())
        aDBData = GetNewDBMgr()->GetAddressDBName();
    return aDBData;
}

void SwDoc::SetInitDBFields( sal_Bool b )
{
	GetNewDBMgr()->SetInitDBFields( b );
}

/*--------------------------------------------------------------------
	Beschreibung: Alle von Feldern verwendete Datenbanken herausfinden
 --------------------------------------------------------------------*/
String lcl_DBDataToString(const SwDBData& rData)
{
	String sRet = rData.sDataSource;
	sRet += DB_DELIM;
	sRet += (String)rData.sCommand;
	sRet += DB_DELIM;
	sRet += String::CreateFromInt32(rData.nCommandType);
	return sRet;
}
void SwDoc::GetAllUsedDB( SvStringsDtor& rDBNameList,
							const SvStringsDtor* pAllDBNames )
{
	SvStringsDtor aUsedDBNames;
	SvStringsDtor aAllDBNames;

	if( !pAllDBNames )
	{
		GetAllDBNames( aAllDBNames );
		pAllDBNames = &aAllDBNames;
	}

	SwSectionFmts& rArr = GetSections();
	for (sal_uInt16 n = rArr.Count(); n; )
	{
		SwSection* pSect = rArr[ --n ]->GetSection();

		if( pSect )
		{
			String aCond( pSect->GetCondition() );
			AddUsedDBToList( rDBNameList, FindUsedDBs( *pAllDBNames,
												aCond, aUsedDBNames ) );
			aUsedDBNames.DeleteAndDestroy( 0, aUsedDBNames.Count() );
		}
	}

	const SfxPoolItem* pItem;
	sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
    for (sal_uInt32 n = 0; n < nMaxItems; ++n)
	{
		if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
			continue;

		const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
		const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
		if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
			continue;

		const SwField* pFld = pFmtFld->GetField();
		switch( pFld->GetTyp()->Which() )
		{
			case RES_DBFLD:
				AddUsedDBToList( rDBNameList,
								lcl_DBDataToString(((SwDBField*)pFld)->GetDBData() ));
				break;

			case RES_DBSETNUMBERFLD:
			case RES_DBNAMEFLD:
				AddUsedDBToList( rDBNameList,
								lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData() ));
				break;

			case RES_DBNUMSETFLD:
			case RES_DBNEXTSETFLD:
				AddUsedDBToList( rDBNameList,
								lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData() ));
				// kein break  // JP: ist das so richtig ??

			case RES_HIDDENTXTFLD:
			case RES_HIDDENPARAFLD:
				AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
											pFld->GetPar1(), aUsedDBNames ));
				aUsedDBNames.DeleteAndDestroy( 0, aUsedDBNames.Count() );
				break;

			case RES_SETEXPFLD:
			case RES_GETEXPFLD:
			case RES_TABLEFLD:
				AddUsedDBToList(rDBNameList, FindUsedDBs( *pAllDBNames,
										pFld->GetFormula(), aUsedDBNames ));
				aUsedDBNames.DeleteAndDestroy( 0, aUsedDBNames.Count() );
				break;
		}
	}
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::GetAllDBNames( SvStringsDtor& rAllDBNames )
{
	SwNewDBMgr* pMgr = GetNewDBMgr();

	const SwDSParamArr& rArr = pMgr->GetDSParamArray();
	for(sal_uInt16 i = 0; i < rArr.Count(); i++)
	{
		SwDSParam* pParam = rArr[i];
		String* pStr = new String( pParam->sDataSource );
		(*pStr)	+= DB_DELIM;
        (*pStr) += (String)pParam->sCommand;
		rAllDBNames.Insert( pStr, rAllDBNames.Count() );
	}
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

SvStringsDtor& SwDoc::FindUsedDBs( const SvStringsDtor& rAllDBNames,
									const String& rFormel,
									SvStringsDtor& rUsedDBNames )
{
	const CharClass& rCC = GetAppCharClass();
	String 	sFormel( rFormel);
#ifndef UNX
	rCC.toUpper( sFormel );
#endif

	xub_StrLen nPos;
	for (sal_uInt16 i = 0; i < rAllDBNames.Count(); ++i )
	{
		const String* pStr = rAllDBNames.GetObject(i);

		if( STRING_NOTFOUND != (nPos = sFormel.Search( *pStr )) &&
			sFormel.GetChar( nPos + pStr->Len() ) == '.' &&
			(!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 )))
		{
			// Tabellenname suchen
			xub_StrLen nEndPos;
			nPos += pStr->Len() + 1;
			if( STRING_NOTFOUND != (nEndPos = sFormel.Search('.', nPos)) )
			{
				String* pDBNm = new String( *pStr );
				pDBNm->Append( DB_DELIM );
				pDBNm->Append( sFormel.Copy( nPos, nEndPos - nPos ));
				rUsedDBNames.Insert( pDBNm, rUsedDBNames.Count() );
			}
		}
	}
	return rUsedDBNames;
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::AddUsedDBToList( SvStringsDtor& rDBNameList,
							 const SvStringsDtor& rUsedDBNames )
{
	for (sal_uInt16 i = 0; i < rUsedDBNames.Count(); i++)
		AddUsedDBToList( rDBNameList, *rUsedDBNames.GetObject(i) );
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::AddUsedDBToList( SvStringsDtor& rDBNameList, const String& rDBName)
{
	if( !rDBName.Len() )
		return;

#ifdef UNX
	for( sal_uInt16 i = 0; i < rDBNameList.Count(); ++i )
		if( rDBName == rDBNameList.GetObject(i)->GetToken(0) )
			return;
#else
	const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
	for( sal_uInt16 i = 0; i < rDBNameList.Count(); ++i )
		if( rSCmp.isEqual( rDBName, rDBNameList.GetObject(i)->GetToken(0) ) )
			return;
#endif

    SwDBData aData;
    aData.sDataSource = rDBName.GetToken(0, DB_DELIM);
    aData.sCommand = rDBName.GetToken(1, DB_DELIM);
    aData.nCommandType = -1;
    GetNewDBMgr()->CreateDSData(aData);
	String* pNew = new String( rDBName );
	rDBNameList.Insert( pNew, rDBNameList.Count() );
}

/*--------------------------------------------------------------------
	 Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::ChangeDBFields( const SvStringsDtor& rOldNames,
							const String& rNewName )
{
	SwDBData aNewDBData;
	aNewDBData.sDataSource = rNewName.GetToken(0, DB_DELIM);
	aNewDBData.sCommand = rNewName.GetToken(1, DB_DELIM);
	aNewDBData.nCommandType = (short)rNewName.GetToken(2, DB_DELIM).ToInt32();

	String sFormel;

	SwSectionFmts& rArr = GetSections();
    for (sal_uInt16 n = rArr.Count(); n; )
	{
		SwSection* pSect = rArr[ --n ]->GetSection();

		if( pSect )
		{
			sFormel = pSect->GetCondition();
			ReplaceUsedDBs( rOldNames, rNewName, sFormel);
			pSect->SetCondition(sFormel);
		}
	}

	const SfxPoolItem* pItem;
	sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );

    for (sal_uInt32 n = 0; n < nMaxItems; ++n )
	{
		if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_FIELD, n ) ))
			continue;

		SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
		SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
		if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
			continue;

		SwField* pFld = pFmtFld->GetField();
		sal_Bool bExpand = sal_False;

		switch( pFld->GetTyp()->Which() )
		{
			case RES_DBFLD:
				if( IsNameInArray( rOldNames, lcl_DBDataToString(((SwDBField*)pFld)->GetDBData())))
				{
					SwDBFieldType* pOldTyp = (SwDBFieldType*)pFld->GetTyp();

					SwDBFieldType* pTyp = (SwDBFieldType*)InsertFldType(
							SwDBFieldType(this, pOldTyp->GetColumnName(), aNewDBData));

                    pFmtFld->RegisterToFieldType( *pTyp );
					pFld->ChgTyp(pTyp);

					((SwDBField*)pFld)->ClearInitialized();
					((SwDBField*)pFld)->InitContent();

					bExpand = sal_True;
				}
				break;

			case RES_DBSETNUMBERFLD:
			case RES_DBNAMEFLD:
				if( IsNameInArray( rOldNames,
								lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData())))
				{
					((SwDBNameInfField*)pFld)->SetDBData(aNewDBData);
					bExpand = sal_True;
				}
				break;

			case RES_DBNUMSETFLD:
			case RES_DBNEXTSETFLD:
				if( IsNameInArray( rOldNames,
								lcl_DBDataToString(((SwDBNameInfField*)pFld)->GetRealDBData())))
				{
					((SwDBNameInfField*)pFld)->SetDBData(aNewDBData);
					bExpand = sal_True;
				}
				// kein break;
			case RES_HIDDENTXTFLD:
			case RES_HIDDENPARAFLD:
				sFormel = pFld->GetPar1();
				ReplaceUsedDBs( rOldNames, rNewName, sFormel);
				pFld->SetPar1( sFormel );
				bExpand = sal_True;
				break;

			case RES_SETEXPFLD:
			case RES_GETEXPFLD:
			case RES_TABLEFLD:
				sFormel = pFld->GetFormula();
				ReplaceUsedDBs( rOldNames, rNewName, sFormel);
				pFld->SetPar2( sFormel );
				bExpand = sal_True;
				break;
		}

		if (bExpand)
			pTxtFld->ExpandTxtFld( true );
	}
	SetModified();
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDoc::ReplaceUsedDBs( const SvStringsDtor& rUsedDBNames,
							const String& rNewName, String& rFormel )
{
	const CharClass& rCC = GetAppCharClass();
	String 	sFormel(rFormel);
	String	sNewName( rNewName );
	sNewName.SearchAndReplace( DB_DELIM, '.');
	//the command type is not part of the condition
	sNewName = sNewName.GetToken(0, DB_DELIM);
	String sUpperNewNm( sNewName );


	for( sal_uInt16 i = 0; i < rUsedDBNames.Count(); ++i )
	{
		String	sDBName( *rUsedDBNames.GetObject( i ) );

		sDBName.SearchAndReplace( DB_DELIM, '.');
		//cut off command type
		sDBName = sDBName.GetToken(0, DB_DELIM);
		if( !sDBName.Equals( sUpperNewNm ))
		{
			xub_StrLen nPos = 0;

			while ((nPos = sFormel.Search(sDBName, nPos)) != STRING_NOTFOUND)
			{
				if( sFormel.GetChar( nPos + sDBName.Len() ) == '.' &&
					(!nPos || !rCC.isLetterNumeric( sFormel, nPos - 1 )))
				{
					rFormel.Erase( nPos, sDBName.Len() );
					rFormel.Insert( sNewName, nPos );
					//prevent re-searching - this is useless and provokes
					//endless loops when names containing each other and numbers are exchanged
					//e.g.: old ?12345.12345  new: i12345.12345
					nPos = nPos + sNewName.Len();
					sFormel = rFormel;
				}
			}
		}
	}
}

/*--------------------------------------------------------------------
	 Beschreibung:
 --------------------------------------------------------------------*/

sal_Bool SwDoc::IsNameInArray( const SvStringsDtor& rArr, const String& rName )
{
#ifdef UNX
	for( sal_uInt16 i = 0; i < rArr.Count(); ++i )
		if( rName == *rArr[ i ] )
			return sal_True;
#else
	const ::utl::TransliterationWrapper& rSCmp = GetAppCmpStrIgnore();
	for( sal_uInt16 i = 0; i < rArr.Count(); ++i )
		if( rSCmp.isEqual( rName, *rArr[ i] ))
			return sal_True;
#endif
	return sal_False;
}

void SwDoc::SetFixFields( bool bOnlyTimeDate, const DateTime* pNewDateTime )
{
	sal_Bool bIsModified = IsModified();

	sal_uLong nDate, nTime;
	if( pNewDateTime )
	{
		nDate = pNewDateTime->GetDate();
		nTime = pNewDateTime->GetTime();
	}
	else
	{
		nDate = Date().GetDate();
		nTime = Time().GetTime();
	}

	sal_uInt16 aTypes[5] = {
		/*0*/	RES_DOCINFOFLD,
		/*1*/	RES_AUTHORFLD,
		/*2*/	RES_EXTUSERFLD,
		/*3*/	RES_FILENAMEFLD,
		/*4*/	RES_DATETIMEFLD };	// MUSS am Ende stehen!!

	sal_uInt16 nStt = bOnlyTimeDate ? 4 : 0;

	for( ; nStt < 5; ++nStt )
	{
		SwFieldType* pFldType = GetSysFldType( aTypes[ nStt ] );
		SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
		for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld; pFmtFld = aIter.Next() )
		{
			if( pFmtFld && pFmtFld->GetTxtFld() )
			{
				sal_Bool bChgd = sal_False;
				switch( aTypes[ nStt ] )
				{
				case RES_DOCINFOFLD:
					if( ((SwDocInfoField*)pFmtFld->GetField())->IsFixed() )
					{
						bChgd = sal_True;
						SwDocInfoField* pDocInfFld = (SwDocInfoField*)pFmtFld->GetField();
						pDocInfFld->SetExpansion( ((SwDocInfoFieldType*)
									pDocInfFld->GetTyp())->Expand(
										pDocInfFld->GetSubType(),
										pDocInfFld->GetFormat(),
										pDocInfFld->GetLanguage(),
										pDocInfFld->GetName() ) );
					}
					break;

				case RES_AUTHORFLD:
					if( ((SwAuthorField*)pFmtFld->GetField())->IsFixed() )
					{
						bChgd = sal_True;
						SwAuthorField* pAuthorFld = (SwAuthorField*)pFmtFld->GetField();
						pAuthorFld->SetExpansion( ((SwAuthorFieldType*)
									pAuthorFld->GetTyp())->Expand(
												pAuthorFld->GetFormat() ) );
					}
					break;

				case RES_EXTUSERFLD:
					if( ((SwExtUserField*)pFmtFld->GetField())->IsFixed() )
					{
						bChgd = sal_True;
						SwExtUserField* pExtUserFld = (SwExtUserField*)pFmtFld->GetField();
						pExtUserFld->SetExpansion( ((SwExtUserFieldType*)
									pExtUserFld->GetTyp())->Expand(
											pExtUserFld->GetSubType(),
											pExtUserFld->GetFormat()));
					}
					break;

				case RES_DATETIMEFLD:
					if( ((SwDateTimeField*)pFmtFld->GetField())->IsFixed() )
					{
						bChgd = sal_True;
						((SwDateTimeField*)pFmtFld->GetField())->SetDateTime(
                                                    DateTime(Date(nDate), Time(nTime)) );
					}
					break;

				case RES_FILENAMEFLD:
					if( ((SwFileNameField*)pFmtFld->GetField())->IsFixed() )
					{
						bChgd = sal_True;
						SwFileNameField* pFileNameFld =
							(SwFileNameField*)pFmtFld->GetField();
						pFileNameFld->SetExpansion( ((SwFileNameFieldType*)
									pFileNameFld->GetTyp())->Expand(
											pFileNameFld->GetFormat() ) );
					}
					break;
				}

				// Formatierung anstossen
				if( bChgd )
					pFmtFld->ModifyNotification( 0, 0 );
			}
		}
	}

	if( !bIsModified )
		ResetModified();
}

bool SwDoc::SetFieldsDirty( bool b, const SwNode* pChk, sal_uLong nLen )
{
	// teste ggfs. mal, ob die angegbenen Nodes ueberhaupt Felder beinhalten.
	// wenn nicht, braucht das Flag nicht veraendert werden.
	sal_Bool bFldsFnd = sal_False;
	if( b && pChk && !GetUpdtFlds().IsFieldsDirty() && !IsInDtor()
		// ?? was ist mit Undo, da will man es doch auch haben !!
		/*&& &pChk->GetNodes() == &GetNodes()*/ )
	{
		b = sal_False;
		if( !nLen )
			++nLen;
		sal_uLong nStt = pChk->GetIndex();
		const SwNodes& rNds = pChk->GetNodes();
		while( nLen-- )
		{
			const SwTxtNode* pTNd = rNds[ nStt++ ]->GetTxtNode();
			if( pTNd )
			{
				if( //pTNd->GetFmtColl() &&		//#outline level,zhaojianwei
				//	MAXLEVEL > pTNd->GetTxtColl()->GetOutlineLevel() )
					pTNd->GetAttrOutlineLevel() != 0 )//<-end,zhaojianwei
					// Kapitelfelder aktualisieren
					b = sal_True;
				else if( pTNd->GetpSwpHints() && pTNd->GetSwpHints().Count() )
					for( sal_uInt16 n = 0, nEnd = pTNd->GetSwpHints().Count();
							n < nEnd; ++n )
					{
						const SwTxtAttr* pAttr = pTNd->GetSwpHints()[ n ];
						if ( pAttr->Which() == RES_TXTATR_FIELD )
						{
							b = sal_True;
							break;
						}
					}

				if( b )
					break;
			}
		}
		bFldsFnd = b;
	}
	GetUpdtFlds().SetFieldsDirty( b );
	return bFldsFnd;
}
/* -----------------------------21.12.99 12:55--------------------------------

 ---------------------------------------------------------------------------*/
void SwDoc::ChangeAuthorityData( const SwAuthEntry* pNewData )
{
	const sal_uInt16 nSize = pFldTypes->Count();

	for( sal_uInt16 i = INIT_FLDTYPES; i < nSize; ++i )
	{
		SwFieldType* pFldType = (*pFldTypes)[i];
		if( RES_AUTHORITY  == pFldType->Which() )
		{
			SwAuthorityFieldType* pAuthType = (SwAuthorityFieldType*)pFldType;
			pAuthType->ChangeEntryContent(pNewData);
			break;
		}
	}

}
/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDocUpdtFld::InsDelFldInFldLst( sal_Bool bIns, const SwTxtFld& rFld )
{
    const sal_uInt16 nWhich = rFld.GetFmtFld().GetField()->GetTyp()->Which();
	switch( nWhich )
	{
	case RES_DBFLD:
	case RES_SETEXPFLD:
	case RES_HIDDENPARAFLD:
	case RES_HIDDENTXTFLD:
	case RES_DBNUMSETFLD:
	case RES_DBNEXTSETFLD:
	case RES_DBSETNUMBERFLD:
	case RES_GETEXPFLD:
		break;			// diese muessen ein-/ausgetragen werden!

	default:
		return;
	}

	SetFieldsDirty( sal_True );
	if( !pFldSortLst )
	{
		if( !bIns ) 			// keine Liste vorhanden und loeschen
			return; 			// dann nichts tun
		pFldSortLst = new _SetGetExpFlds( 64, 16 );
	}

	if( bIns )		// neu einfuegen:
		GetBodyNode( rFld, nWhich );
	else
	{
		// ueber den pTxtFld Pointer suchen. Ist zwar eine Sortierte
		// Liste, aber nach Node-Positionen sortiert. Bis dieser
		// bestimmt ist, ist das Suchen nach dem Pointer schon fertig
		for( sal_uInt16 n = 0; n < pFldSortLst->Count(); ++n )
			if( &rFld == (*pFldSortLst)[ n ]->GetPointer() )
				pFldSortLst->DeleteAndDestroy( n--, 1 );
				// ein Feld kann mehrfach vorhanden sein!
	}
}

void SwDocUpdtFld::MakeFldList( SwDoc& rDoc, int bAll, int eGetMode )
{
    if( !pFldSortLst || bAll || !( eGetMode & nFldLstGetMode ) ||
        rDoc.GetNodes().Count() != nNodes )
        _MakeFldList( rDoc, eGetMode );
}

void SwDocUpdtFld::_MakeFldList( SwDoc& rDoc, int eGetMode )
{
	// neue Version: gehe ueber alle Felder vom Attribut-Pool
	if( pFldSortLst )
		delete pFldSortLst;
	pFldSortLst = new _SetGetExpFlds( 64, 16 );

    /// OD 09.08.2002 [#101207#,#101216#,#101778#] - consider and unhide sections
    ///     with hide condition, only in mode GETFLD_ALL (<eGetMode == GETFLD_ALL>)
    ///     notes by OD:
    ///         eGetMode == GETFLD_CALC in call from methods SwDoc::FldsToCalc
    ///         eGetMode == GETFLD_EXPAND in call from method SwDoc::FldsToExpand
    ///         eGetMode == GETFLD_ALL in call from method SwDoc::UpdateExpFlds
    ///         I figured out that hidden section only have to be shown,
    ///         if fields have updated (call by SwDoc::UpdateExpFlds) and thus
    ///         the hide conditions of section have to be updated.
    ///         For correct updating the hide condition of a section, its position
    ///         have to be known in order to insert the hide condition as a new
    ///         expression field into the sorted field list (<pFldSortLst>).
    if ( eGetMode == GETFLD_ALL )
	// zuerst die Bereiche einsammeln. Alle die ueber Bedingung
	// gehiddet sind, wieder mit Frames versorgen, damit die darin
	// enthaltenen Felder richtig einsortiert werden!!!
    {
		// damit die Frames richtig angelegt werden, muessen sie in der
		// Reihenfolgen von oben nach unten expandiert werden
		SvULongs aTmpArr;
		SwSectionFmts& rArr = rDoc.GetSections();
		SwSectionNode* pSectNd;
		sal_uInt16 nArrStt = 0;
		sal_uLong nSttCntnt = rDoc.GetNodes().GetEndOfExtras().GetIndex();

        for (sal_uInt16 n = rArr.Count(); n; )
		{
			SwSection* pSect = rArr[ --n ]->GetSection();
            if( pSect && pSect->IsHidden() && pSect->GetCondition().Len() &&
				0 != ( pSectNd = pSect->GetFmt()->GetSectionNode() ))
			{
				sal_uLong nIdx = pSectNd->GetIndex();
				sal_uInt16 i;

				for( i = 0; i < aTmpArr.Count() && aTmpArr[ i ] < nIdx; ++i )
					;
				aTmpArr.Insert( nIdx, i );
				if( nIdx < nSttCntnt )
					++nArrStt;
			}
		}

        // erst alle anzeigen, damit die Frames vorhanden sind. Mit deren
        // Position wird das BodyAnchor ermittelt.
        // Dafuer erst den ContentBereich, dann die Sonderbereiche!!!
        for (sal_uInt16 n = nArrStt; n < aTmpArr.Count(); ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            ASSERT( pSectNd, "Wo ist mein SectionNode" );
            pSectNd->GetSection().SetCondHidden( sal_False );
        }
        for (sal_uInt16 n = 0; n < nArrStt; ++n)
        {
            pSectNd = rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode();
            ASSERT( pSectNd, "Wo ist mein SectionNode" );
            pSectNd->GetSection().SetCondHidden( sal_False );
        }

        // so, erst jetzt alle sortiert in die Liste eintragen
        for (sal_uInt16 n = 0; n < aTmpArr.Count(); ++n)
        {
            GetBodyNode( *rDoc.GetNodes()[ aTmpArr[ n ] ]->GetSectionNode() );
        }
    }

	String sTrue( String::CreateFromAscii(
									RTL_CONSTASCII_STRINGPARAM( "sal_True" ))),
		   sFalse( String::CreateFromAscii(
		   							RTL_CONSTASCII_STRINGPARAM( "sal_False" )));

	sal_Bool bIsDBMgr = 0 != rDoc.GetNewDBMgr();
	sal_uInt16 nWhich, n;
	const String* pFormel = 0;
	const SfxPoolItem* pItem;
	sal_uInt32 nMaxItems = rDoc.GetAttrPool().GetItemCount2( RES_TXTATR_FIELD );
	for( n = 0; n < nMaxItems; ++n )
	{
        if( 0 == (pItem = rDoc.GetAttrPool().GetItem2( RES_TXTATR_FIELD, n )) )
			continue;

		const SwFmtFld* pFmtFld = (SwFmtFld*)pItem;
		const SwTxtFld* pTxtFld = pFmtFld->GetTxtFld();
		if( !pTxtFld || !pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
			continue;

		const SwField* pFld = pFmtFld->GetField();
		switch( nWhich = pFld->GetTyp()->Which() )
		{
			case RES_DBSETNUMBERFLD:
			case RES_GETEXPFLD:
				if( GETFLD_ALL == eGetMode )
					pFormel = &sTrue;
				break;

			case RES_DBFLD:
				if( GETFLD_EXPAND & eGetMode )
					pFormel = &sTrue;
				break;

			case RES_SETEXPFLD:
                /// fields of subtype <string> have also been add
                /// for calculation (eGetMode == GETFLD_CALC).
                /// Thus, add fields of subtype <string> in all modes
                ///     (eGetMode == GETFLD_EXPAND||GETFLD_CALC||GETFLD_ALL)
                /// and fields of other subtypes only in the modes
                ///     (eGetMode == GETFLD_CALC||GETFLD_ALL)
                /* "old" if construct - not deleted for history and code review
                if( ( nsSwGetSetExpType::GSE_STRING & pFld->GetSubType()
                        ? GETFLD_EXPAND : GETFLD_CALC )
                        & eGetMode )
                */
                if ( !(eGetMode == GETFLD_EXPAND) ||
                     (nsSwGetSetExpType::GSE_STRING & pFld->GetSubType()) )
                {
					pFormel = &sTrue;
                }
				break;

			case RES_HIDDENPARAFLD:
				if( GETFLD_ALL == eGetMode )
				{
					pFormel = &pFld->GetPar1();
					if( !pFormel->Len() || pFormel->Equals( sFalse ))
						((SwHiddenParaField*)pFld)->SetHidden( sal_False );
					else if( pFormel->Equals( sTrue ))
						((SwHiddenParaField*)pFld)->SetHidden( sal_True );
					else
						break;

					pFormel = 0;
					// Formatierung anstossen
					((SwFmtFld*)pFmtFld)->ModifyNotification( 0, 0 );
				}
				break;

			case RES_HIDDENTXTFLD:
				if( GETFLD_ALL == eGetMode )
				{
					pFormel = &pFld->GetPar1();
					if( !pFormel->Len() || pFormel->Equals( sFalse ))
						((SwHiddenTxtField*)pFld)->SetValue( sal_True );
					else if( pFormel->Equals( sTrue ))
						((SwHiddenTxtField*)pFld)->SetValue( sal_False );
					else
						break;

					pFormel = 0;

					// Feld Evaluieren
					((SwHiddenTxtField*)pFld)->Evaluate(&rDoc);
					// Formatierung anstossen
					((SwFmtFld*)pFmtFld)->ModifyNotification( 0, 0 );
				}
				break;

			case RES_DBNUMSETFLD:
			{
				SwDBData aDBData(((SwDBNumSetField*)pFld)->GetDBData(&rDoc));

                if (
                     (bIsDBMgr && rDoc.GetNewDBMgr()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && ((SwDBNumSetField*)pFld)->IsCondValid()))
                   )
                {
                    pFormel = &pFld->GetPar1();
                }
			}
			break;
			case RES_DBNEXTSETFLD:
			{
				SwDBData aDBData(((SwDBNextSetField*)pFld)->GetDBData(&rDoc));

                if (
                     (bIsDBMgr && rDoc.GetNewDBMgr()->OpenDataSource(aDBData.sDataSource, aDBData.sCommand)) &&
                     (GETFLD_ALL == eGetMode || (GETFLD_CALC & eGetMode && ((SwDBNextSetField*)pFld)->IsCondValid()))
                   )
                {
                    pFormel = &pFld->GetPar1();
                }
			}
			break;
		}

		if( pFormel && pFormel->Len() )
		{
			GetBodyNode( *pTxtFld, nWhich );
			pFormel = 0;
		}
	}
	nFldLstGetMode = static_cast<sal_uInt8>( eGetMode );
	nNodes = rDoc.GetNodes().Count();

#ifdef JP_DEBUG
	{
	SvFileStream sOut( "f:\\x.x", STREAM_STD_WRITE );
	sOut.Seek( STREAM_SEEK_TO_END );
	sOut << "------------------" << endl;
	const _SetGetExpFldPtr* pSortLst = pFldSortLst->GetData();
	for( sal_uInt16 n = pFldSortLst->Count(); n; --n, ++pSortLst )
	{
		String sStr( (*pSortLst)->GetNode() );
		sStr += "\t, ";
		sStr += (*pSortLst)->GetCntnt();
		sStr += "\tNode: ";
		sStr += (*pSortLst)->GetFld()->GetTxtNode().StartOfSectionIndex();
		sStr += "\tPos: ";
		sStr += *(*pSortLst)->GetFld()->GetStart();
		sStr += "\tType: ";
		sStr += (*pSortLst)->GetFld()->GetFld().GetFld()->GetTyp()->Which();

		sOut << sStr.GetStr() << endl;
	}
	}
#endif
	// JP_DEBUG
}

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

void SwDocUpdtFld::GetBodyNode( const SwTxtFld& rTFld, sal_uInt16 nFldWhich )
{
	const SwTxtNode& rTxtNd = rTFld.GetTxtNode();
	const SwDoc& rDoc = *rTxtNd.GetDoc();

	// immer den ersten !! (in Tab-Headline, Kopf-/Fuss )
	Point aPt;
	const SwCntntFrm* pFrm = rTxtNd.getLayoutFrm( rDoc.GetCurrentLayout(), &aPt, 0, sal_False );

    _SetGetExpFld* pNew = NULL;
	sal_Bool bIsInBody = sal_False;

	if( !pFrm || pFrm->IsInDocBody() )
	{
		// einen Index fuers bestimmen vom TextNode anlegen
		SwNodeIndex aIdx( rTxtNd );
		bIsInBody = rDoc.GetNodes().GetEndOfExtras().GetIndex() < aIdx.GetIndex();

        // #104291# dvo: We don't want to update fields in redlines, or those
        // in frames whose anchor is in redline. However, we do want to update
        // fields in hidden sections. So: In order to be updated, a field 1)
        // must have a frame, or 2) it must be in the document body.
        if( (pFrm != NULL) || bIsInBody )
            pNew = new _SetGetExpFld( aIdx, &rTFld );
	}
	else
	{
		// einen Index fuers bestimmen vom TextNode anlegen
		SwPosition aPos( rDoc.GetNodes().GetEndOfPostIts() );
#ifdef DBG_UTIL
		ASSERT( GetBodyTxtNode( rDoc, aPos, *pFrm ), "wo steht das Feld" );
#else
		GetBodyTxtNode( rDoc, aPos, *pFrm );
#endif
		pNew = new _SetGetExpFld( aPos.nNode, &rTFld, &aPos.nContent );
	}

	// bei GetExp.-/DB.-Felder immer das BodyTxtFlag setzen
	if( RES_GETEXPFLD == nFldWhich )
	{
		SwGetExpField* pGetFld = (SwGetExpField*)rTFld.GetFmtFld().GetField();
		pGetFld->ChgBodyTxtFlag( bIsInBody );
	}
	else if( RES_DBFLD == nFldWhich )
	{
		SwDBField* pDBFld = (SwDBField*)rTFld.GetFmtFld().GetField();
		pDBFld->ChgBodyTxtFlag( bIsInBody );
	}

    if( pNew != NULL )
        if( !pFldSortLst->Insert( pNew ))
            delete pNew;
}

void SwDocUpdtFld::GetBodyNode( const SwSectionNode& rSectNd )
{
	const SwDoc& rDoc = *rSectNd.GetDoc();
	_SetGetExpFld* pNew = 0;

	if( rSectNd.GetIndex() < rDoc.GetNodes().GetEndOfExtras().GetIndex() )
	{
		do {			// middle check loop

			// dann muessen wir uns mal den Anker besorgen!
			// einen Index fuers bestimmen vom TextNode anlegen
			SwPosition aPos( rSectNd );
			SwCntntNode* pCNd = rDoc.GetNodes().GoNext( &aPos.nNode ); // zum naechsten ContentNode

			if( !pCNd || !pCNd->IsTxtNode() )
				break;

			// immer den ersten !! (in Tab-Headline, Kopf-/Fuss )
			Point aPt;
			const SwCntntFrm* pFrm = pCNd->getLayoutFrm( rDoc.GetCurrentLayout(), &aPt, 0, sal_False );
			if( !pFrm )
				break;

#ifdef DBG_UTIL
			ASSERT( GetBodyTxtNode( rDoc, aPos, *pFrm ), "wo steht das Feld" );
#else
			GetBodyTxtNode( rDoc, aPos, *pFrm );
#endif
			pNew = new _SetGetExpFld( rSectNd, &aPos );

		} while( sal_False );
	}

	if( !pNew )
		pNew = new _SetGetExpFld( rSectNd );

	if( !pFldSortLst->Insert( pNew ))
		delete pNew;
}

void SwDocUpdtFld::InsertFldType( const SwFieldType& rType )
{
	String sFldName;
	switch( rType.Which() )
	{
	case RES_USERFLD :
		sFldName = ((SwUserFieldType&)rType).GetName();
		break;
	case RES_SETEXPFLD:
		sFldName = ((SwSetExpFieldType&)rType).GetName();
		break;
	default:
		ASSERT( sal_False, "no valid field type" );
	}

	if( sFldName.Len() )
	{
		SetFieldsDirty( sal_True );
		// suchen und aus der HashTabelle entfernen
		GetAppCharClass().toLower( sFldName );
		sal_uInt16 n;

		SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );

		if( !pFnd )
		{
			SwCalcFldType* pNew = new SwCalcFldType( sFldName, &rType );
			pNew->pNext = aFldTypeTable[ n ];
			aFldTypeTable[ n ] = pNew;
		}
	}
}

void SwDocUpdtFld::RemoveFldType( const SwFieldType& rType )
{
	String sFldName;
	switch( rType.Which() )
	{
	case RES_USERFLD :
		sFldName = ((SwUserFieldType&)rType).GetName();
		break;
	case RES_SETEXPFLD:
		sFldName = ((SwSetExpFieldType&)rType).GetName();
		break;
	}

	if( sFldName.Len() )
	{
		SetFieldsDirty( sal_True );
		// suchen und aus der HashTabelle entfernen
		GetAppCharClass().toLower( sFldName );
		sal_uInt16 n;

		SwHash* pFnd = Find( sFldName, GetFldTypeTable(), TBLSZ, &n );
		if( pFnd )
		{
			if( aFldTypeTable[ n ] == pFnd )
				aFldTypeTable[ n ] = (SwCalcFldType*)pFnd->pNext;
			else
			{
				SwHash* pPrev = aFldTypeTable[ n ];
				while( pPrev->pNext != pFnd )
					pPrev = pPrev->pNext;
				pPrev->pNext = pFnd->pNext;
			}
			pFnd->pNext = 0;
			delete pFnd;
		}
	}
}

SwDocUpdtFld::SwDocUpdtFld()
	: pFldSortLst(0),  nFldUpdtPos(LONG_MAX), nFldLstGetMode(0)
{
	bInUpdateFlds = bFldsDirty = sal_False;
	memset( aFldTypeTable, 0, sizeof( aFldTypeTable ) );
}

SwDocUpdtFld::~SwDocUpdtFld()
{
	delete pFldSortLst;

	for( sal_uInt16 n = 0; n < TBLSZ; ++n )
		delete aFldTypeTable[n];
}

// #111840#
bool SwDoc::UpdateFld(SwTxtFld * pDstTxtFld, SwField & rSrcFld,
                      SwMsgPoolItem * pMsgHnt,
                      bool bUpdateFlds)
{
    ASSERT(pDstTxtFld, "no field to update!");

    sal_Bool bTblSelBreak = sal_False;

    SwFmtFld * pDstFmtFld = (SwFmtFld*)&pDstTxtFld->GetFmtFld();
    SwField * pDstFld = pDstFmtFld->GetField();
    sal_uInt16 nFldWhich = rSrcFld.GetTyp()->Which();
    SwNodeIndex aTblNdIdx(pDstTxtFld->GetTxtNode());

    if (pDstFld->GetTyp()->Which() ==
        rSrcFld.GetTyp()->Which())
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            SwPosition aPosition( pDstTxtFld->GetTxtNode() );
            aPosition.nContent = *pDstTxtFld->GetStart();

            SwUndo *const pUndo( new SwUndoFieldFromDoc( aPosition, *pDstFld, rSrcFld, pMsgHnt, bUpdateFlds) );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }

        SwField * pNewFld = rSrcFld.CopyField();
        pDstFmtFld->SetField(pNewFld);

        switch( nFldWhich )
        {
        case RES_SETEXPFLD:
        case RES_GETEXPFLD:
        case RES_HIDDENTXTFLD:
        case RES_HIDDENPARAFLD:
            UpdateExpFlds( pDstTxtFld, true );
            break;

        case RES_TABLEFLD:
            {
                const SwTableNode* pTblNd =
                    IsIdxInTbl(aTblNdIdx);
                if( pTblNd )
                {
                    SwTableFmlUpdate aTblUpdate( &pTblNd->
                                                 GetTable() );
                    if (bUpdateFlds)
                        UpdateTblFlds( &aTblUpdate );
                    else
                        pNewFld->GetTyp()->ModifyNotification(0, &aTblUpdate);

                    if (! bUpdateFlds)
                        bTblSelBreak = sal_True;
                }
            }
            break;

        case RES_MACROFLD:
            if( bUpdateFlds && pDstTxtFld->GetpTxtNode() )
                (pDstTxtFld->GetpTxtNode())->
                    ModifyNotification( 0, pDstFmtFld );
            break;

        case RES_DBNAMEFLD:
        case RES_DBNEXTSETFLD:
        case RES_DBNUMSETFLD:
        case RES_DBSETNUMBERFLD:
            ChgDBData(((SwDBNameInfField*) pNewFld)->GetRealDBData());
            pNewFld->GetTyp()->UpdateFlds();

            break;

        case RES_DBFLD:
            {
                // JP 10.02.96: ChgValue aufrufen, damit
                //die Format- aenderung den ContentString
                //richtig setzt
                SwDBField* pDBFld = (SwDBField*)pNewFld;
                if (pDBFld->IsInitialized())
                    pDBFld->ChgValue( pDBFld->GetValue(), sal_True );

                pDBFld->ClearInitialized();
                pDBFld->InitContent();
            }
            // kein break;

        default:
            pDstFmtFld->ModifyNotification( 0, pMsgHnt );
        }

        // Die Felder die wir berechnen koennen werden hier expli.
        // zum Update angestossen.
        if( nFldWhich == RES_USERFLD )
            UpdateUsrFlds();
    }

    return bTblSelBreak;
}

bool SwDoc::PutValueToField(const SwPosition & rPos,
                            const Any& rVal, sal_uInt16 nWhich)
{
    Any aOldVal;
    SwField * pField = GetFieldAtPos(rPos);


    if (GetIDocumentUndoRedo().DoesUndo() &&
        pField->QueryValue(aOldVal, nWhich))
    {
        SwUndo *const pUndo(new SwUndoFieldFromAPI(rPos, aOldVal, rVal, nWhich));
        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    return pField->PutValue(rVal, nWhich);
}
