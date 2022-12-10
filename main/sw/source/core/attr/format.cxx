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

#include <hintids.hxx> 			// fuer RES_..
#include <frame.hxx>			// fuer AttrCache
#include <format.hxx>
#include <hints.hxx>			// fuer SwFmtChg
#include <doc.hxx>
#include <paratr.hxx>			// fuer SwParaFmt - SwHyphenBug
#include <swcache.hxx>
#include <fmtcolfunc.hxx>

//UUUU
#include <svx/sdr/attribute/sdrallfillattributeshelper.hxx>
#include <svx/unobrushitemhelper.hxx>
#include <svx/xdef.hxx>
#include <frmatr.hxx>

TYPEINIT1( SwFmt, SwClient );	//rtti fuer SwFmt

/*************************************************************************
|*    SwFmt::SwFmt
*************************************************************************/


SwFmt::SwFmt( SwAttrPool& rPool, const sal_Char* pFmtNm,
			const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich )
	: SwModify( pDrvdFrm ),
	aSet( rPool, pWhichRanges ),
	nWhichId( nFmtWhich ),
	nFmtId( 0 ),
	nPoolFmtId( USHRT_MAX ),
	nPoolHelpId( USHRT_MAX ),
	nPoolHlpFileId( UCHAR_MAX )
{
	aFmtName.AssignAscii( pFmtNm );
    bWritten = bFmtInDTOR = bAutoUpdateFmt = sal_False; // LAYER_IMPL
	bAutoFmt = sal_True;

	if( pDrvdFrm )
		aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( SwAttrPool& rPool, const String &rFmtNm,
			const sal_uInt16* pWhichRanges, SwFmt *pDrvdFrm, sal_uInt16 nFmtWhich )
	: SwModify( pDrvdFrm ),
	aFmtName( rFmtNm ),
	aSet( rPool, pWhichRanges ),
	nWhichId( nFmtWhich ),
	nFmtId( 0 ),
	nPoolFmtId( USHRT_MAX ),
	nPoolHelpId( USHRT_MAX ),
	nPoolHlpFileId( UCHAR_MAX )
{
    bWritten = bFmtInDTOR = bAutoUpdateFmt = sal_False; // LAYER_IMPL
	bAutoFmt = sal_True;

	if( pDrvdFrm )
		aSet.SetParent( &pDrvdFrm->aSet );
}


SwFmt::SwFmt( const SwFmt& rFmt )
	: SwModify( rFmt.DerivedFrom() ),
	aFmtName( rFmt.aFmtName ),
	aSet( rFmt.aSet ),
	nWhichId( rFmt.nWhichId ),
	nFmtId( 0 ),
	nPoolFmtId( rFmt.GetPoolFmtId() ),
	nPoolHelpId( rFmt.GetPoolHelpId() ),
	nPoolHlpFileId( rFmt.GetPoolHlpFileId() )
{
    bWritten = bFmtInDTOR = sal_False; // LAYER_IMPL
	bAutoFmt = rFmt.bAutoFmt;
	bAutoUpdateFmt = rFmt.bAutoUpdateFmt;

	if( rFmt.DerivedFrom() )
		aSet.SetParent( &rFmt.DerivedFrom()->aSet );
	// einige Sonderbehandlungen fuer Attribute
	aSet.SetModifyAtAttr( this );
}

/*************************************************************************
|*    SwFmt &SwFmt::operator=(const SwFmt& aFmt)
|*
|*    Beschreibung		Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


SwFmt &SwFmt::operator=(const SwFmt& rFmt)
{
	nWhichId = rFmt.nWhichId;
	nPoolFmtId = rFmt.GetPoolFmtId();
	nPoolHelpId = rFmt.GetPoolHelpId();
	nPoolHlpFileId = rFmt.GetPoolHlpFileId();

	if ( IsInCache() )
	{
		SwFrm::GetCache().Delete( this );
		SetInCache( sal_False );
	}
	SetInSwFntCache( sal_False );

	// kopiere nur das Attribut-Delta Array
	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
				aNew( *aSet.GetPool(), aSet.GetRanges() );
	aSet.Intersect_BC( rFmt.aSet, &aOld, &aNew );
	aSet.Put_BC( rFmt.aSet, &aOld, &aNew );

	// einige Sonderbehandlungen fuer Attribute
	aSet.SetModifyAtAttr( this );

	// PoolItem-Attr erzeugen fuers Modify !!!
	if( aOld.Count() )
	{
		SwAttrSetChg aChgOld( aSet, aOld );
		SwAttrSetChg aChgNew( aSet, aNew );
		ModifyNotification( &aChgOld, &aChgNew );        // alle veraenderten werden verschickt
	}

	if( GetRegisteredIn() != rFmt.GetRegisteredIn() )
	{
		if( GetRegisteredIn() )
			GetRegisteredInNonConst()->Remove(this);
		if(rFmt.GetRegisteredIn())
		{
			const_cast<SwFmt&>(rFmt).GetRegisteredInNonConst()->Add(this);
			aSet.SetParent( &rFmt.aSet );
		}
		else
			aSet.SetParent( 0 );
	}
	bAutoFmt = rFmt.bAutoFmt;
	bAutoUpdateFmt = rFmt.bAutoUpdateFmt;
	return *this;
}

void SwFmt::SetName( const String& rNewName, sal_Bool bBroadcast )
{
	ASSERT(!IsDefault(), "SetName: Defaultformat" );
	if( bBroadcast )
	{
		SwStringMsgPoolItem aOld( RES_NAME_CHANGED, aFmtName );
		SwStringMsgPoolItem aNew( RES_NAME_CHANGED, rNewName );
		aFmtName = rNewName;
		ModifyNotification( &aOld, &aNew );
	}
	else
	{
		aFmtName = rNewName;
	}
}

/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * diese Funktion wird in jedem Copy-Ctor gerufen, damit die
 * Attribute kopiert werden. Diese koennen erst kopiert werden,
 * wenn die abgeleitet Klasse existiert, denn beim Setzen der
 * Attribute wird die Which()-Methode gerufen, die hier in der
 * Basis-Klasse auf 0 defaultet ist.
 *
 * Zusatz: JP 8.4.1994
 * 	Wird ueber Dokumentgrenzen kopiert, so muss das neue Dokument
 *	mit angeben werden, in dem this steht. Z.Z. ist das fuers
 *	DropCaps wichtig, dieses haelt Daten, die tief kopiert werden
 *	muessen !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */


void SwFmt::CopyAttrs( const SwFmt& rFmt, sal_Bool bReplace )
{
	// kopiere nur das Attribut-Delta Array

	if ( IsInCache() )
	{
		SwFrm::GetCache().Delete( this );
		SetInCache( sal_False );
	}
	SetInSwFntCache( sal_False );

	// Sonderbehandlung fuer einige Attribute
	SwAttrSet* pChgSet = (SwAttrSet*)&rFmt.aSet;

	if( !bReplace )		// nur die neu, die nicht gesetzt sind ??
	{
		if( pChgSet == (SwAttrSet*)&rFmt.aSet )		// Set hier kopieren
			pChgSet = new SwAttrSet( rFmt.aSet );
		pChgSet->Differentiate( aSet );
	}

	// kopiere nur das Attribut-Delta Array
	if( pChgSet->GetPool() != aSet.GetPool() )
		pChgSet->CopyToModify( *this );
	else
	{
		SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
				  aNew( *aSet.GetPool(), aSet.GetRanges() );

		if ( 0 != aSet.Put_BC( *pChgSet, &aOld, &aNew ) )
		{
			// einige Sonderbehandlungen fuer Attribute
			aSet.SetModifyAtAttr( this );

			SwAttrSetChg aChgOld( aSet, aOld );
			SwAttrSetChg aChgNew( aSet, aNew );
			ModifyNotification( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
		}
	}

	if( pChgSet != (SwAttrSet*)&rFmt.aSet )		// Set hier angelegt ?
		delete pChgSet;
}

/*************************************************************************
|*    SwFmt::~SwFmt()
|*
|*    Beschreibung		Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 14.02.91
*************************************************************************/


SwFmt::~SwFmt()
{
	/* das passiert bei der ObjectDying Message */
	/* alle Abhaengigen auf DerivedFrom umhaengen */
	if( GetDepends() )
	{
		ASSERT(DerivedFrom(), "SwFmt::~SwFmt: Def Abhaengige!" );

		bFmtInDTOR = sal_True;

		SwFmt *pParentFmt = DerivedFrom();
		if (!pParentFmt)		// see #112405#
		{
			DBG_ERROR( "~SwFmt: parent format missing" );
		}
		else
		{
			while( GetDepends() && pParentFmt)
			{
				SwFmtChg aOldFmt(this);
				SwFmtChg aNewFmt(pParentFmt);
				SwClient * pDepend = (SwClient*)GetDepends();
				pParentFmt->Add(pDepend);
				pDepend->ModifyNotification(&aOldFmt, &aNewFmt);
			}
		}
	}
}


/*************************************************************************
|*    void SwFmt::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
|*
|*    Beschreibung		Dokument 1.14
|*    Ersterstellung    JP 22.11.90
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


void SwFmt::Modify( const SfxPoolItem* pOldValue, const SfxPoolItem* pNewValue )
{
	sal_Bool bWeiter = sal_True;	// sal_True = Propagierung an die Abhaengigen

	sal_uInt16 nWhich = pOldValue ? pOldValue->Which() :
					pNewValue ? pNewValue->Which() : 0 ;
	switch( nWhich )
	{
	case 0:		break;			// Which-Id von 0 ???

	case RES_OBJECTDYING :
		{
			// ist das sterbende Object das "Parent"-Format von diesen Format,
			// dann haengt sich dieses Format an den Parent vom Parent
			SwFmt * pFmt = (SwFmt *) ((SwPtrMsgPoolItem *)pNewValue)->pObject;

			// nicht umhaengen wenn dieses das oberste Format ist !!
			if( GetRegisteredIn() && GetRegisteredIn() == pFmt )
			{
				if( pFmt->GetRegisteredIn() )
				{
					// wenn Parent, dann im neuen Parent wieder anmelden
					pFmt->DerivedFrom()->Add( this );
					aSet.SetParent( &DerivedFrom()->aSet );
				}
				else
				{
					// sonst auf jeden Fall beim sterbenden abmelden
					DerivedFrom()->Remove( this );
					aSet.SetParent( 0 );
				}
			}
		} // OBJECTDYING
		break;

	case RES_ATTRSET_CHG:
		if( ((SwAttrSetChg*)pOldValue)->GetTheChgdSet() != &aSet )
		{
			//nur die weiter geben, die hier nicht gesetzt sind !!
			SwAttrSetChg aOld( *(SwAttrSetChg*)pOldValue );
			SwAttrSetChg aNew( *(SwAttrSetChg*)pNewValue );

			aOld.GetChgSet()->Differentiate( aSet );
			aNew.GetChgSet()->Differentiate( aSet );

			if( aNew.Count() )
				// keine mehr gesetzt, dann Ende !!
            NotifyClients( &aOld, &aNew );
			bWeiter = sal_False;
		}
		break;
	case RES_FMT_CHG:
		// falls mein Format Parent umgesetzt wird, dann melde ich
		// meinen Attrset beim Neuen an.

		// sein eigenes Modify ueberspringen !!
		if( ((SwFmtChg*)pOldValue)->pChangedFmt != this &&
			((SwFmtChg*)pNewValue)->pChangedFmt == DerivedFrom() )
		{
			// den Set an den neuen Parent haengen
			aSet.SetParent( DerivedFrom() ? &DerivedFrom()->aSet : 0 );
		}
		break;

	case RES_RESET_FMTWRITTEN:
		{
			// IsWritten-Flag zuruecksetzen. Hint nur an abhanegige
			// Formate (und keine Frames) propagieren.
            // mba: the code does the opposite from what is written in the comment!
			ResetWritten();
            // mba: here we don't use the additional stuff from NotifyClients().
            // should we?!
            // mba: move the code that ignores this event to the clients
            ModifyBroadcast( pOldValue, pNewValue, TYPE(SwFmt) );
			bWeiter = sal_False;
		}
		break;

	default:
		{
			// Ist das Attribut in diesem Format definiert, dann auf
			// NICHT weiter propagieren !!
			if( SFX_ITEM_SET == aSet.GetItemState( nWhich, sal_False ))
			{
// wie finde ich heraus, ob nicht ich die Message versende ??
// aber wer ruft das hier ????
//ASSERT( sal_False, "Modify ohne Absender verschickt" );
//JP 11.06.96: DropCaps koennen hierher kommen
ASSERT( RES_PARATR_DROP == nWhich, "Modify ohne Absender verschickt" );
				bWeiter = sal_False;
			}

		} // default
	} // switch

	if( bWeiter )
	{
		// laufe durch alle abhaengigen Formate
        NotifyClients( pOldValue, pNewValue );
	}

}


sal_Bool SwFmt::SetDerivedFrom(SwFmt *pDerFrom)
{
	if ( pDerFrom )
	{
		// Zyklus?
		const SwFmt* pFmt = pDerFrom;
		while ( pFmt != 0 )
		{
			if ( pFmt == this )
				return sal_False;

			pFmt=pFmt->DerivedFrom();
		}
	}
	else
	{
		// Nichts angegeben, Dflt-Format suchen
		pDerFrom = this;
		while ( pDerFrom->DerivedFrom() )
			pDerFrom = pDerFrom->DerivedFrom();
	}
	if ( (pDerFrom == DerivedFrom()) || (pDerFrom == this) )
		return sal_False;

	ASSERT( Which()==pDerFrom->Which()
			|| ( Which()==RES_CONDTXTFMTCOLL && pDerFrom->Which()==RES_TXTFMTCOLL)
			|| ( Which()==RES_TXTFMTCOLL && pDerFrom->Which()==RES_CONDTXTFMTCOLL)
			|| ( Which()==RES_FLYFRMFMT && pDerFrom->Which()==RES_FRMFMT ),
			"SetDerivedFrom: Aepfel von Birnen ableiten?");

	if ( IsInCache() )
	{
		SwFrm::GetCache().Delete( this );
		SetInCache( sal_False );
	}
	SetInSwFntCache( sal_False );

	pDerFrom->Add(this);
	aSet.SetParent( &pDerFrom->aSet );

	SwFmtChg aOldFmt(this);
	SwFmtChg aNewFmt(this);
	ModifyNotification( &aOldFmt, &aNewFmt );

	return sal_True;
}


const SfxPoolItem& SwFmt::GetFmtAttr( sal_uInt16 nWhich, sal_Bool bInParents ) const
{
    if(RES_BACKGROUND == nWhich && supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        OSL_ENSURE(false, "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");
        static SvxBrushItem aSvxBrushItem(RES_BACKGROUND);

        // fill the local static SvxBrushItem from the current ItemSet so that
        // the fill attributes [XATTR_FILL_FIRST .. XATTR_FILL_LAST] are used
        // as good as possible to create a fallback representation and return that
        aSvxBrushItem = getSvxBrushItemFromSourceSet(aSet, RES_BACKGROUND, bInParents);

        return aSvxBrushItem;
    }

    return aSet.Get( nWhich, bInParents );
}


SfxItemState SwFmt::GetItemState( sal_uInt16 nWhich, sal_Bool bSrchInParent, const SfxPoolItem **ppItem ) const
{
    if(RES_BACKGROUND == nWhich && supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        OSL_ENSURE(false, "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");
        const drawinglayer::attribute::SdrAllFillAttributesHelperPtr aFill = getSdrAllFillAttributesHelper();

        // check if the new fill attributes are used
        if(aFill.get() && aFill->isUsed())
        {
            // if yes, fill the local SvxBrushItem using the new fill attributes
            // as good as possible to have an instance for the pointer to point 
            // to and return as state that it is set
            static SvxBrushItem aSvxBrushItem(RES_BACKGROUND);

            aSvxBrushItem = getSvxBrushItemFromSourceSet(aSet, RES_BACKGROUND, bSrchInParent);
            if( ppItem )
                *ppItem = &aSvxBrushItem;

            return SFX_ITEM_SET;
        }

        // if not, reset pointer and return SFX_ITEM_DEFAULT to signal that
        // the item is not set
        if( ppItem )
            *ppItem = NULL;

        return SFX_ITEM_DEFAULT;
    }

    return aSet.GetItemState( nWhich, bSrchInParent, ppItem );
}


sal_Bool SwFmt::SetFmtAttr(const SfxPoolItem& rAttr )
{
	if ( IsInCache() || IsInSwFntCache() )
	{
		const sal_uInt16 nWhich = rAttr.Which();
		CheckCaching( nWhich );
	}

	sal_Bool bRet = sal_False;

    //UUUU
    if(RES_BACKGROUND == rAttr.Which() && supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        OSL_ENSURE(false, "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");
        SfxItemSet aTempSet(*aSet.GetPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST, 0, 0);
        const SvxBrushItem& rSource = static_cast< const SvxBrushItem& >(rAttr);

        // fill a local ItemSet with the attributes corresponding as good as possible
        // to the new fill properties [XATTR_FILL_FIRST .. XATTR_FILL_LAST] and set these
        // as ItemSet
        setSvxBrushItemAsFillAttributesToTargetSet(rSource, aTempSet);

        if(IsModifyLocked())
        {
            if( 0 != ( bRet = (0 != aSet.Put( aTempSet ))) )
            {
                aSet.SetModifyAtAttr( this );
            }
        }
        else
        {
            SwAttrSet aOld(*aSet.GetPool(), aSet.GetRanges()), aNew(*aSet.GetPool(), aSet.GetRanges());

            bRet = 0 != aSet.Put_BC(aTempSet, &aOld, &aNew);

            if(bRet)
            {
                aSet.SetModifyAtAttr(this);

                SwAttrSetChg aChgOld(aSet, aOld);
                SwAttrSetChg aChgNew(aSet, aNew);

                ModifyNotification(&aChgOld, &aChgNew);
            }
        }

        return bRet;
    }

	// wenn Modify gelockt ist, werden keine Modifies verschickt;
	// fuer FrmFmt's immer das Modify verschicken!
    const sal_uInt16 nFmtWhich = Which();
	if( IsModifyLocked() || (!GetDepends() &&
        (RES_GRFFMTCOLL == nFmtWhich  ||
         RES_TXTFMTCOLL == nFmtWhich ) ) )
	{
		if( 0 != ( bRet = (0 != aSet.Put( rAttr ))) )
			aSet.SetModifyAtAttr( this );
        if ( nFmtWhich == RES_TXTFMTCOLL && rAttr.Which() == RES_PARATR_NUMRULE )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
	}
	else
	{
		// kopiere nur das Attribut-Delta Array
		SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
					aNew( *aSet.GetPool(), aSet.GetRanges() );

		bRet = 0 != aSet.Put_BC( rAttr, &aOld, &aNew );
		if( bRet )
		{
			// einige Sonderbehandlungen fuer Attribute
			aSet.SetModifyAtAttr( this );

			SwAttrSetChg aChgOld( aSet, aOld );
			SwAttrSetChg aChgNew( aSet, aNew );
			ModifyNotification( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
		}
	}
	return bRet;
}


sal_Bool SwFmt::SetFmtAttr( const SfxItemSet& rSet )
{
	if( !rSet.Count() )
		return sal_False;

	if ( IsInCache() )
	{
		SwFrm::GetCache().Delete( this );
		SetInCache( sal_False );
	}
	SetInSwFntCache( sal_False );

	sal_Bool bRet = sal_False;

    //UUUU Use local copy to be able to apply needed changes, e.g. call
    // CheckForUniqueItemForLineFillNameOrIndex which is needed for NameOrIndex stuff
    SfxItemSet aTempSet(rSet);

    //UUUU Need to check for unique item for DrawingLayer items of type NameOrIndex
    // and evtl. correct that item to ensure unique names for that type. This call may
    // modify/correct entries inside of the given SfxItemSet
    if(GetDoc())
    {
        GetDoc()->CheckForUniqueItemForLineFillNameOrIndex(aTempSet);
    }

    //UUUU   FlyFrame              PageStyle
    if(supportsFullDrawingLayerFillAttributeSet())
    {
        const SfxPoolItem* pSource = 0;

        if(SFX_ITEM_SET == aTempSet.GetItemState(RES_BACKGROUND, sal_False, &pSource))
        {
            //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
            OSL_ENSURE(false, "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");

            // copy all items to be set anyways to a local ItemSet with is also prepared for the new
            // fill attribute ranges [XATTR_FILL_FIRST .. XATTR_FILL_LAST]. Add the attributes 
            // corresponding as good as possible to the new fill properties and set the whole ItemSet
            const SvxBrushItem& rSource(static_cast< const SvxBrushItem& >(*pSource));
            setSvxBrushItemAsFillAttributesToTargetSet(rSource, aTempSet);

            if(IsModifyLocked())
            {
                if( 0 != ( bRet = (0 != aSet.Put( aTempSet ))) )
                {
                    aSet.SetModifyAtAttr( this );
                }
            }
            else
            {
                SwAttrSet aOld(*aSet.GetPool(), aSet.GetRanges()), aNew(*aSet.GetPool(), aSet.GetRanges());

                bRet = 0 != aSet.Put_BC(aTempSet, &aOld, &aNew);

                if(bRet)
                {
                    aSet.SetModifyAtAttr(this);

                    SwAttrSetChg aChgOld(aSet, aOld);
                    SwAttrSetChg aChgNew(aSet, aNew);

                    ModifyNotification(&aChgOld, &aChgNew);
                }
            }

            return bRet;
        }
    }

    // wenn Modify gelockt ist, werden keine Modifies verschickt;
	// fuer FrmFmt's immer das Modify verschicken!
    const sal_uInt16 nFmtWhich = Which();
    if ( IsModifyLocked() ||
         ( !GetDepends() &&
           ( RES_GRFFMTCOLL == nFmtWhich ||
             RES_TXTFMTCOLL == nFmtWhich ) ) )
	{
		if( 0 != ( bRet = (0 != aSet.Put( aTempSet ))) )
			aSet.SetModifyAtAttr( this );
        if ( nFmtWhich == RES_TXTFMTCOLL )
        {
            TxtFmtCollFunc::CheckTxtFmtCollForDeletionOfAssignmentToOutlineStyle( this );
        }
	}
	else
	{
		SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
					aNew( *aSet.GetPool(), aSet.GetRanges() );
		bRet = 0 != aSet.Put_BC( aTempSet, &aOld, &aNew );
		if( bRet )
		{
			// einige Sonderbehandlungen fuer Attribute
			aSet.SetModifyAtAttr( this );
			SwAttrSetChg aChgOld( aSet, aOld );
			SwAttrSetChg aChgNew( aSet, aNew );
			ModifyNotification( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
		}
	}
	return bRet;
}

// Nimmt den Hint mit nWhich aus dem Delta-Array


sal_Bool SwFmt::ResetFmtAttr( sal_uInt16 nWhich1, sal_uInt16 nWhich2 )
{
	if( !aSet.Count() )
		return sal_False;

	if( !nWhich2 || nWhich2 < nWhich1 )
		nWhich2 = nWhich1;		// dann setze auf 1. Id, nur dieses Item

	if ( IsInCache() || IsInSwFntCache() )
	{
		for( sal_uInt16 n = nWhich1; n < nWhich2; ++n )
			CheckCaching( n );
	}

	// wenn Modify gelockt ist, werden keine Modifies verschickt
	if( IsModifyLocked() )
		return 0 != (( nWhich2 == nWhich1 )
				? aSet.ClearItem( nWhich1 )
				: aSet.ClearItem_BC( nWhich1, nWhich2 ));

	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
				aNew( *aSet.GetPool(), aSet.GetRanges() );
	sal_Bool bRet = 0 != aSet.ClearItem_BC( nWhich1, nWhich2, &aOld, &aNew );

	if( bRet )
	{
		SwAttrSetChg aChgOld( aSet, aOld );
		SwAttrSetChg aChgNew( aSet, aNew );
		ModifyNotification( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
	}
	return bRet;
}



// --> OD 2007-01-24 #i73790#
// method renamed
sal_uInt16 SwFmt::ResetAllFmtAttr()
// <--
{
	if( !aSet.Count() )
		return 0;

	if ( IsInCache() )
	{
		SwFrm::GetCache().Delete( this );
		SetInCache( sal_False );
	}
	SetInSwFntCache( sal_False );

	// wenn Modify gelockt ist, werden keine Modifies verschickt
	if( IsModifyLocked() )
		return aSet.ClearItem( 0 );

	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
				aNew( *aSet.GetPool(), aSet.GetRanges() );
	sal_Bool bRet = 0 != aSet.ClearItem_BC( 0, &aOld, &aNew );

	if( bRet )
	{
		SwAttrSetChg aChgOld( aSet, aOld );
		SwAttrSetChg aChgNew( aSet, aNew );
		ModifyNotification( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
	}
	return aNew.Count();
}


/*************************************************************************
|*    void SwFmt::GetInfo( const SfxPoolItem& ) const
|*
|*    Beschreibung
|*    Ersterstellung    JP 18.04.94
|*    Letzte Aenderung  JP 05.08.94
*************************************************************************/


sal_Bool SwFmt::GetInfo( SfxPoolItem& rInfo ) const
{
	sal_Bool bRet = SwModify::GetInfo( rInfo );
	return bRet;
}


void SwFmt::DelDiffs( const SfxItemSet& rSet )
{
	if( !aSet.Count() )
		return;

	if ( IsInCache() )
	{
		SwFrm::GetCache().Delete( this );
		SetInCache( sal_False );
	}
	SetInSwFntCache( sal_False );

	// wenn Modify gelockt ist, werden keine Modifies verschickt
	if( IsModifyLocked() )
	{
		aSet.Intersect( rSet );
		return;
	}

	SwAttrSet aOld( *aSet.GetPool(), aSet.GetRanges() ),
				aNew( *aSet.GetPool(), aSet.GetRanges() );
	sal_Bool bRet = 0 != aSet.Intersect_BC( rSet, &aOld, &aNew );

	if( bRet )
	{
		SwAttrSetChg aChgOld( aSet, aOld );
		SwAttrSetChg aChgNew( aSet, aNew );
		ModifyNotification( &aChgOld, &aChgNew );		// alle veraenderten werden verschickt
	}
}

/** SwFmt::IsBackgroundTransparent - for feature #99657#

    OD 22.08.2002
    Virtual method to determine, if background of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent background.

    @author OD

    @return false, default implementation
*/
sal_Bool SwFmt::IsBackgroundTransparent() const
{
    return sal_False;
}

/** SwFmt::IsShadowTransparent - for feature #99657#

    OD 22.08.2002
    Virtual method to determine, if shadow of format is transparent.
    Default implementation returns false. Thus, subclasses have to overload
    method, if the specific subclass can have a transparent shadow.

    @author OD

    @return false, default implementation
*/
sal_Bool SwFmt::IsShadowTransparent() const
{
    return sal_False;
}

/*
 * Document Interface Access
 */
const IDocumentSettingAccess* SwFmt::getIDocumentSettingAccess() const { return GetDoc(); }
const IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() const { return GetDoc(); }
IDocumentDrawModelAccess* SwFmt::getIDocumentDrawModelAccess() { return GetDoc(); }
const IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() const { return GetDoc(); }
IDocumentLayoutAccess* SwFmt::getIDocumentLayoutAccess() { return GetDoc(); }
IDocumentTimerAccess* SwFmt::getIDocumentTimerAccess() { return GetDoc(); }
IDocumentFieldsAccess* SwFmt::getIDocumentFieldsAccess() { return GetDoc(); }
IDocumentChartDataProviderAccess* SwFmt::getIDocumentChartDataProviderAccess() { return GetDoc(); }

//UUUU
const SvxBrushItem& SwFmt::GetBackground(sal_Bool bInP) const
{ 
    //UUUU   FlyFrame              PageStyle
    if(supportsFullDrawingLayerFillAttributeSet())
    {
        //UUUU FALLBACKBREAKHERE should not be used; instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST]
        OSL_ENSURE(false, "Do no longer use SvxBrushItem, instead use [XATTR_FILL_FIRST .. XATTR_FILL_LAST] FillAttributes (simple fallback is in place and used)");
        static SvxBrushItem aSvxBrushItem(RES_BACKGROUND);

        // fill the local static SvxBrushItem from the current ItemSet so that
        // the fill attributes [XATTR_FILL_FIRST .. XATTR_FILL_LAST] are used
        // as good as possible to create a fallback representation and return that
        aSvxBrushItem = getSvxBrushItemFromSourceSet(aSet, RES_BACKGROUND, bInP);

        return aSvxBrushItem;
    }

    return aSet.GetBackground(bInP); 
}

//UUUU
bool SwFmt::supportsFullDrawingLayerFillAttributeSet() const
{
    // base definition - probably not completely correct, e.g. for Table FillStyles
    //UUUU  FlyFrame                    PageStyle
    return (RES_FLYFRMFMT == Which() || RES_FRMFMT == Which());
}

//UUUU 
drawinglayer::attribute::SdrAllFillAttributesHelperPtr SwFmt::getSdrAllFillAttributesHelper() const
{
    return drawinglayer::attribute::SdrAllFillAttributesHelperPtr();
}

// eof
