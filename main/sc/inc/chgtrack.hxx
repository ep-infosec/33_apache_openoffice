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



#ifndef SC_CHGTRACK_HXX
#define SC_CHGTRACK_HXX


#include <tools/string.hxx>
#include <tools/datetime.hxx>
#include <tools/table.hxx>
#include <tools/stack.hxx>
#include <tools/queue.hxx>
#include <tools/mempool.hxx>
#include <tools/link.hxx>
#include <unotools/options.hxx>
#include "global.hxx"
#include "bigrange.hxx"
#include "collect.hxx"
#include "scdllapi.h"

#ifdef SC_CHGTRACK_CXX
// core/inc
#include "refupdat.hxx"
#endif

#define DEBUG_CHANGETRACK 0
class ScChangeAction;
class ScAppOptions;
class ScActionColorChanger
{
private:
	const ScAppOptions&		rOpt;
	const ScStrCollection&	rUsers;
	String					aLastUserName;
	sal_uInt16					nLastUserIndex;
	ColorData				nColor;

public:
				ScActionColorChanger( const ScChangeTrack& rTrack );
				~ScActionColorChanger() {}
				void		Update( const ScChangeAction& rAction );
				ColorData	GetColor() const	{ return nColor; }
};
class ScBaseCell;
class ScDocument;


enum ScChangeActionType
{
	SC_CAT_NONE,
	SC_CAT_INSERT_COLS,
	SC_CAT_INSERT_ROWS,
	SC_CAT_INSERT_TABS,
	SC_CAT_DELETE_COLS,
	SC_CAT_DELETE_ROWS,
	SC_CAT_DELETE_TABS,
	SC_CAT_MOVE,
	SC_CAT_CONTENT,
	SC_CAT_REJECT
};


enum ScChangeActionState
{
	SC_CAS_VIRGIN,
	SC_CAS_ACCEPTED,
	SC_CAS_REJECTED
};


enum ScChangeActionClipMode
{
	SC_CACM_NONE,
	SC_CACM_CUT,
	SC_CACM_COPY,
	SC_CACM_PASTE
};

class SvStream;

// --- ScChangeActionLinkEntry ---------------------------------------------

// Fuegt sich selbst am Beginn einer Kette ein, bzw. vor einem anderen
// LinkEntry, on delete selbstaendiges ausklinken auch des gelinkten.
// ppPrev == &previous->pNext oder Adresse des Pointers auf Beginn der Kette,
// *ppPrev == this

class ScChangeAction;

class ScChangeActionLinkEntry
{
								// not implemented, prevent usage
								ScChangeActionLinkEntry(
									const ScChangeActionLinkEntry& );
	ScChangeActionLinkEntry&	operator=( const ScChangeActionLinkEntry& );

protected:

	ScChangeActionLinkEntry*	pNext;
	ScChangeActionLinkEntry**	ppPrev;
	ScChangeAction*				pAction;
	ScChangeActionLinkEntry*	pLink;

public:

	DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionLinkEntry )

								ScChangeActionLinkEntry(
										ScChangeActionLinkEntry** ppPrevP,
										ScChangeAction* pActionP )
									:	pNext( *ppPrevP ),
										ppPrev( ppPrevP ),
										pAction( pActionP ),
										pLink( NULL )
									{
										if ( pNext )
											pNext->ppPrev = &pNext;
										*ppPrevP = this;
									}

	virtual						~ScChangeActionLinkEntry()
									{
										ScChangeActionLinkEntry* p = pLink;
										UnLink();
										Remove();
										if ( p )
											delete p;
									}

			void				SetLink( ScChangeActionLinkEntry* pLinkP )
									{
										UnLink();
										if ( pLinkP )
										{
											pLink = pLinkP;
											pLinkP->pLink = this;
										}
									}

			void				UnLink()
									{
										if ( pLink )
										{
											pLink->pLink = NULL;
											pLink = NULL;
										}
									}

			void				Remove()
									{
										if ( ppPrev )
										{
                                            if ( ( *ppPrev = pNext ) != NULL )
												pNext->ppPrev = ppPrev;
											ppPrev = NULL;	// not inserted
										}
									}

			void				Insert( ScChangeActionLinkEntry** ppPrevP )
									{
										if ( !ppPrev )
										{
											ppPrev = ppPrevP;
											if ( (pNext = *ppPrevP) )
												pNext->ppPrev = &pNext;
											*ppPrevP = this;
										}
									}

	const ScChangeActionLinkEntry*	GetLink() const		{ return pLink; }
	ScChangeActionLinkEntry*		GetLink()			{ return pLink; }
	const ScChangeActionLinkEntry*	GetNext() const		{ return pNext; }
	ScChangeActionLinkEntry*		GetNext()			{ return pNext; }
	const ScChangeAction*			GetAction() const	{ return pAction; }
	ScChangeAction*					GetAction()			{ return pAction; }
#if DEBUG_CHANGETRACK
    String                          ToString() const;
#endif // DEBUG_CHANGETRACK
};

// --- ScChangeActionCellListEntry -----------------------------------------
// this is only for the XML Export in the hxx
class ScChangeActionContent;

class ScChangeActionCellListEntry
{
	friend class ScChangeAction;
	friend class ScChangeActionDel;
	friend class ScChangeActionMove;
	friend class ScChangeTrack;

			ScChangeActionCellListEntry*	pNext;
			ScChangeActionContent*			pContent;

								ScChangeActionCellListEntry(
									ScChangeActionContent* pContentP,
									ScChangeActionCellListEntry* pNextP )
									:	pNext( pNextP ),
										pContent( pContentP )
									{}

public:
	const ScChangeActionCellListEntry* GetNext() const { return pNext; } // this is only for the XML Export public
	const ScChangeActionContent* GetContent() const { return pContent; } // this is only for the XML Export public

	DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionCellListEntry )
};

// --- ScChangeAction -------------------------------------------------------

class ScChangeTrack;
class ScChangeActionIns;
class ScChangeActionDel;
class ScChangeActionContent;

class ScChangeAction
{
	friend class ScChangeTrack;
	friend class ScChangeActionIns;
	friend class ScChangeActionDel;
	friend class ScChangeActionMove;
	friend class ScChangeActionContent;

								// not implemented, prevent usage
								ScChangeAction( const ScChangeAction& );
			ScChangeAction&		operator=( const ScChangeAction& );

protected:

			ScBigRange	  		aBigRange;		 	// Ins/Del/MoveTo/ContentPos
			DateTime			aDateTime;			//! UTC
			String				aUser;				// wer war's
			String				aComment;			// Benutzerkommentar
			ScChangeAction*		pNext;				// naechster in Kette
			ScChangeAction*		pPrev;				// vorheriger in Kette
			ScChangeActionLinkEntry*	pLinkAny;	// irgendwelche Links
			ScChangeActionLinkEntry*	pLinkDeletedIn;	// Zuordnung zu
													// geloeschten oder
													// druebergemoveten oder
													// rejecteten Insert
													// Bereichen
			ScChangeActionLinkEntry*	pLinkDeleted;	// Links zu geloeschten
			ScChangeActionLinkEntry*	pLinkDependent;	// Links zu abhaengigen
			sal_uLong				nAction;
			sal_uLong				nRejectAction;
			ScChangeActionType	eType;
			ScChangeActionState	eState;


								ScChangeAction( ScChangeActionType,
												const ScRange& );

								// only to be used in the XML import
								ScChangeAction( ScChangeActionType,
												const ScBigRange&,
												const sal_uLong nAction,
												const sal_uLong nRejectAction,
												const ScChangeActionState eState,
												const DateTime& aDateTime,
												const String& aUser,
												const String& aComment );
								// only to be used in the XML import
								ScChangeAction( ScChangeActionType,
												const ScBigRange&,
												const sal_uLong nAction);

	virtual						~ScChangeAction();

			String				GetRefString( const ScBigRange&,
									ScDocument*, sal_Bool bFlag3D = sal_False ) const;

			void				SetActionNumber( sal_uLong n ) { nAction = n; }
			void				SetRejectAction( sal_uLong n ) { nRejectAction = n; }
			void				SetUser( const String& r ) { aUser = r; }
			void				SetType( ScChangeActionType e ) { eType = e; }
			void				SetState( ScChangeActionState e ) { eState = e; }
			void				SetRejected();

			ScBigRange& 		GetBigRange() { return aBigRange; }

			ScChangeActionLinkEntry*	AddLink( ScChangeAction* p,
											ScChangeActionLinkEntry* pL )
									{
										ScChangeActionLinkEntry* pLnk =
											new ScChangeActionLinkEntry(
											&pLinkAny, p );
										pLnk->SetLink( pL );
										return pLnk;
									}
			void				RemoveAllAnyLinks();

	virtual	ScChangeActionLinkEntry*	GetDeletedIn() const
											{ return pLinkDeletedIn; }
	virtual	ScChangeActionLinkEntry**	GetDeletedInAddress()
											{ return &pLinkDeletedIn; }
			ScChangeActionLinkEntry*	AddDeletedIn( ScChangeAction* p )
									{
										return new ScChangeActionLinkEntry(
											GetDeletedInAddress(), p );
									}
			sal_Bool				RemoveDeletedIn( const ScChangeAction* );
			void				SetDeletedIn( ScChangeAction* );

			ScChangeActionLinkEntry*	AddDeleted( ScChangeAction* p )
									{
										return new ScChangeActionLinkEntry(
											&pLinkDeleted, p );
									}
			void				RemoveAllDeleted();

			ScChangeActionLinkEntry*	AddDependent( ScChangeAction* p )
									{
										return new ScChangeActionLinkEntry(
											&pLinkDependent, p );
									}
			void				RemoveAllDependent();

			void				RemoveAllLinks();

	virtual	void				AddContent( ScChangeActionContent* ) = 0;
	virtual	void				DeleteCellEntries() = 0;

	virtual	void 				UpdateReference( const ScChangeTrack*,
									UpdateRefMode, const ScBigRange&,
									sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

			void				Accept();
	virtual	sal_Bool				Reject( ScDocument* ) = 0;
			void				RejectRestoreContents( ScChangeTrack*,
									SCsCOL nDx, SCsROW nDy );

								// used in Reject() instead of IsRejectable()
			sal_Bool				IsInternalRejectable() const;

                                // Derived classes that hold a pointer to the
                                // ChangeTrack must return that. Otherwise NULL.
    virtual const ScChangeTrack*    GetChangeTrack() const = 0;

public:

			sal_Bool				IsInsertType() const
									{
										return eType == SC_CAT_INSERT_COLS ||
											eType == SC_CAT_INSERT_ROWS ||
											eType == SC_CAT_INSERT_TABS;
									}
			sal_Bool				IsDeleteType() const
									{
										return eType == SC_CAT_DELETE_COLS ||
											eType == SC_CAT_DELETE_ROWS ||
											eType == SC_CAT_DELETE_TABS;
									}
			sal_Bool				IsVirgin() const
									{ return eState == SC_CAS_VIRGIN; }
			sal_Bool				IsAccepted() const
									{ return eState == SC_CAS_ACCEPTED; }
			sal_Bool				IsRejected() const
									{ return eState == SC_CAS_REJECTED; }

								// Action rejects another Action
			sal_Bool				IsRejecting() const
									{ return nRejectAction != 0; }

								// ob Action im Dokument sichtbar ist
			sal_Bool				IsVisible() const;

								// ob Action anfassbar ist
			sal_Bool				IsTouchable() const;

								// ob Action ein Eintrag in Dialog-Root ist
			sal_Bool				IsDialogRoot() const;

								// ob ein Eintrag im Dialog aufklappbar sein soll
			sal_Bool				IsDialogParent() const;

								// ob Action ein Delete ist, unter dem
								// aufgeklappt mehrere einzelne Deletes sind
			sal_Bool				IsMasterDelete() const;

								// ob Action akzeptiert/selektiert/abgelehnt
								// werden kann
			sal_Bool				IsClickable() const;

								// ob Action abgelehnt werden kann
			sal_Bool				IsRejectable() const;

			const ScBigRange& 	GetBigRange() const { return aBigRange; }
			SC_DLLPUBLIC DateTime			GetDateTime() const;		// local time
			const DateTime&		GetDateTimeUTC() const		// UTC time
									{ return aDateTime; }
			const String&		GetUser() const { return aUser; }
			const String&		GetComment() const { return aComment; }
			ScChangeActionType	GetType() const { return eType; }
			ScChangeActionState	GetState() const { return eState; }
			sal_uLong				GetActionNumber() const { return nAction; }
			sal_uLong				GetRejectAction() const { return nRejectAction; }

			ScChangeAction*		GetNext() const { return pNext; }
			ScChangeAction*		GetPrev() const { return pPrev; }

			sal_Bool				IsDeletedIn() const
									{ return GetDeletedIn() != NULL; }
			sal_Bool				IsDeleted() const
									{ return IsDeleteType() || IsDeletedIn(); }
			sal_Bool				IsDeletedIn( const ScChangeAction* ) const;
			sal_Bool				IsDeletedInDelType( ScChangeActionType ) const;
            void                RemoveAllDeletedIn();

			const ScChangeActionLinkEntry* GetFirstDeletedEntry() const
									{ return pLinkDeleted; }
			const ScChangeActionLinkEntry* GetFirstDependentEntry() const
									{ return pLinkDependent; }
			sal_Bool				HasDependent() const
									{ return pLinkDependent != NULL; }
			sal_Bool				HasDeleted() const
									{ return pLinkDeleted != NULL; }

								// Description wird an String angehaengt.
								// Mit bSplitRange wird bei Delete nur
								// eine Spalte/Zeile beruecksichtigt (fuer
								// Auflistung der einzelnen Eintraege).
	virtual	void				GetDescription( String&, ScDocument*,
									sal_Bool bSplitRange = sal_False, bool bWarning = true ) const;

	virtual void				GetRefString( String&, ScDocument*,
									sal_Bool bFlag3D = sal_False ) const;

								// fuer DocumentMerge altes Datum einer anderen
								// Action setzen, mit GetDateTimeUTC geholt
			void				SetDateTimeUTC( const DateTime& rDT )
									{ aDateTime = rDT; }

								// Benutzerkommentar setzen
			void				SetComment( const String& rStr )
									{ aComment = rStr; }

								// only to be used in the XML import
			void				SetDeletedInThis( sal_uLong nActionNumber,
										const ScChangeTrack* pTrack );
								// only to be used in the XML import
			void				AddDependent( sal_uLong nActionNumber,
										const ScChangeTrack* pTrack );
#if DEBUG_CHANGETRACK
            String              ToString( ScDocument* pDoc ) const;
#endif // DEBUG_CHANGETRACK
};


// --- ScChangeActionIns ----------------------------------------------------

class ScChangeActionIns : public ScChangeAction
{
	friend class ScChangeTrack;

								ScChangeActionIns( const ScRange& rRange );
	virtual						~ScChangeActionIns();

	virtual	void				AddContent( ScChangeActionContent* ) {}
	virtual	void				DeleteCellEntries() {}

	virtual	sal_Bool				Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return 0; }

public:
								ScChangeActionIns(const sal_uLong nActionNumber,
										const ScChangeActionState eState,
										const sal_uLong nRejectingNumber,
										const ScBigRange& aBigRange,
										const String& aUser,
										const DateTime& aDateTime,
										const String &sComment,
										const ScChangeActionType eType); // only to use in the XML import

	virtual	void				GetDescription( String&, ScDocument*,
									sal_Bool bSplitRange = sal_False, bool bWarning = true ) const;
};


// --- ScChangeActionDel ----------------------------------------------------

class ScChangeActionMove;

class ScChangeActionDelMoveEntry : public ScChangeActionLinkEntry
{
	friend class ScChangeActionDel;
	friend class ScChangeTrack;

			short		   		nCutOffFrom;
			short		   		nCutOffTo;


								ScChangeActionDelMoveEntry(
                                    ScChangeActionDelMoveEntry** ppPrevP,
									ScChangeActionMove* pMove,
									short nFrom, short nTo )
									:	ScChangeActionLinkEntry(
											(ScChangeActionLinkEntry**)
                                                ppPrevP,
											(ScChangeAction*) pMove ),
										nCutOffFrom( nFrom ),
										nCutOffTo( nTo )
									{}

			ScChangeActionDelMoveEntry*	GetNext()
									{
										return (ScChangeActionDelMoveEntry*)
										ScChangeActionLinkEntry::GetNext();
									}
			ScChangeActionMove*	GetMove()
									{
										return (ScChangeActionMove*)
										ScChangeActionLinkEntry::GetAction();
									}

public:
			const ScChangeActionDelMoveEntry*	GetNext() const
									{
										return (const ScChangeActionDelMoveEntry*)
										ScChangeActionLinkEntry::GetNext();
									}
			const ScChangeActionMove*	GetMove() const
									{
										return (const ScChangeActionMove*)
										ScChangeActionLinkEntry::GetAction();
									}
			short				GetCutOffFrom() const { return nCutOffFrom; }
			short				GetCutOffTo() const { return nCutOffTo; }
};


class ScChangeActionDel : public ScChangeAction
{
	friend class ScChangeTrack;
	friend void ScChangeAction::Accept();

			ScChangeTrack*		pTrack;
			ScChangeActionCellListEntry* pFirstCell;
			ScChangeActionIns*	pCutOff;		// abgeschnittener Insert
			short				nCutOff;		// +: Start  -: End
			ScChangeActionDelMoveEntry* pLinkMove;
			SCsCOL				nDx;
			SCsROW				nDy;

								ScChangeActionDel( const ScRange& rRange,
									SCsCOL nDx, SCsROW nDy, ScChangeTrack* );
	virtual						~ScChangeActionDel();

			ScChangeActionIns*	GetCutOffInsert() { return pCutOff; }

	virtual	void				AddContent( ScChangeActionContent* );
	virtual	void				DeleteCellEntries();

			void				UndoCutOffMoves();
			void				UndoCutOffInsert();

	virtual	void 				UpdateReference( const ScChangeTrack*,
									UpdateRefMode, const ScBigRange&,
									sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

	virtual	sal_Bool				Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return pTrack; }

public:
								ScChangeActionDel(const sal_uLong nActionNumber,
												const ScChangeActionState eState,
												const sal_uLong nRejectingNumber,
												const ScBigRange& aBigRange,
												const String& aUser,
												const DateTime& aDateTime,
												const String &sComment,
												const ScChangeActionType eType,
												const SCsCOLROW nD,
												ScChangeTrack* pTrack); // only to use in the XML import
																		// which of nDx and nDy is set is depend on the type

								// ob dieses das unterste einer Reihe (oder
								// auch einzeln) ist
			sal_Bool				IsBaseDelete() const;

								// ob dieses das oberste einer Reihe (oder
								// auch einzeln) ist
			sal_Bool				IsTopDelete() const;

								// ob dieses ein Teil einer Reihe ist
			sal_Bool				IsMultiDelete() const;

								// ob es eine Col ist, die zu einem TabDelete gehoert
			sal_Bool				IsTabDeleteCol() const;

			SCsCOL				GetDx() const { return nDx; }
			SCsROW				GetDy() const { return nDy; }
			ScBigRange			GetOverAllRange() const;	// BigRange + (nDx, nDy)

			const ScChangeActionCellListEntry* GetFirstCellEntry() const
									{ return pFirstCell; }
			const ScChangeActionDelMoveEntry* GetFirstMoveEntry() const
									{ return pLinkMove; }
			const ScChangeActionIns*	GetCutOffInsert() const { return pCutOff; }
			short				GetCutOffCount() const { return nCutOff; }

	virtual	void				GetDescription( String&, ScDocument*,
									sal_Bool bSplitRange = sal_False, bool bWarning = true ) const;
			void				SetCutOffInsert( ScChangeActionIns* p, short n )
									{ pCutOff = p; nCutOff = n; }	// only to use in the XML import
																	// this should be protected, but for the XML import it is public
			// only to use in the XML import
			// this should be protected, but for the XML import it is public
			ScChangeActionDelMoveEntry*	AddCutOffMove( ScChangeActionMove* pMove,
										short nFrom, short nTo )
									{
										return new ScChangeActionDelMoveEntry(
										&pLinkMove, pMove, nFrom, nTo );
									}
};


// --- ScChangeActionMove ---------------------------------------------------

class ScChangeActionMove : public ScChangeAction
{
	friend class ScChangeTrack;
	friend class ScChangeActionDel;

			ScBigRange			aFromRange;
			ScChangeTrack*		pTrack;
			ScChangeActionCellListEntry* pFirstCell;
			sal_uLong				nStartLastCut;	// fuer PasteCut Undo
			sal_uLong				nEndLastCut;

								ScChangeActionMove( const ScRange& rFromRange,
									const ScRange& rToRange,
									ScChangeTrack* pTrackP )
									: ScChangeAction( SC_CAT_MOVE, rToRange ),
										aFromRange( rFromRange ),
										pTrack( pTrackP ),
										pFirstCell( NULL ),
										nStartLastCut(0),
										nEndLastCut(0)
									{}
	virtual						~ScChangeActionMove();

	virtual	void				AddContent( ScChangeActionContent* );
	virtual	void				DeleteCellEntries();

			ScBigRange&			GetFromRange() { return aFromRange; }

			void				SetStartLastCut( sal_uLong nVal ) { nStartLastCut = nVal; }
			sal_uLong				GetStartLastCut() const { return nStartLastCut; }
			void				SetEndLastCut( sal_uLong nVal )	{ nEndLastCut = nVal; }
			sal_uLong				GetEndLastCut() const { return nEndLastCut; }

	virtual	void 				UpdateReference( const ScChangeTrack*,
									UpdateRefMode, const ScBigRange&,
									sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

	virtual	sal_Bool				Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return pTrack; }

protected:
    using ScChangeAction::GetRefString;

public:
								ScChangeActionMove(const sal_uLong nActionNumber,
												const ScChangeActionState eState,
												const sal_uLong nRejectingNumber,
												const ScBigRange& aToBigRange,
												const String& aUser,
												const DateTime& aDateTime,
												const String &sComment,
												const ScBigRange& aFromBigRange,
												ScChangeTrack* pTrack); // only to use in the XML import
			const ScChangeActionCellListEntry* GetFirstCellEntry() const
									{ return pFirstCell; } // only to use in the XML export

			const ScBigRange&	GetFromRange() const { return aFromRange; }
	SC_DLLPUBLIC		void				GetDelta( sal_Int32& nDx, sal_Int32& nDy, sal_Int32& nDz ) const;

	virtual	void				GetDescription( String&, ScDocument*,
									sal_Bool bSplitRange = sal_False, bool bWarning = true ) const;

	virtual void				GetRefString( String&, ScDocument*,
									sal_Bool bFlag3D = sal_False ) const;
};


// --- ScChangeActionContent ------------------------------------------------

enum ScChangeActionContentCellType
{
	SC_CACCT_NONE = 0,
	SC_CACCT_NORMAL,
	SC_CACCT_MATORG,
	SC_CACCT_MATREF
};

class Stack;

class ScChangeActionContent : public ScChangeAction
{
	friend class ScChangeTrack;

			String				aOldValue;
			String				aNewValue;
			ScBaseCell*			pOldCell;
			ScBaseCell*			pNewCell;
		ScChangeActionContent*	pNextContent;	// an gleicher Position
		ScChangeActionContent*	pPrevContent;
		ScChangeActionContent*	pNextInSlot;	// in gleichem Slot
		ScChangeActionContent**	ppPrevInSlot;

			void				InsertInSlot( ScChangeActionContent** pp )
									{
										if ( !ppPrevInSlot )
										{
											ppPrevInSlot = pp;
                                            if ( ( pNextInSlot = *pp ) != NULL )
												pNextInSlot->ppPrevInSlot = &pNextInSlot;
											*pp = this;
										}
									}
			void				RemoveFromSlot()
									{
										if ( ppPrevInSlot )
										{
                                            if ( ( *ppPrevInSlot = pNextInSlot ) != NULL )
												pNextInSlot->ppPrevInSlot = ppPrevInSlot;
											ppPrevInSlot = NULL;	// not inserted
										}
									}
		ScChangeActionContent*	GetNextInSlot() { return pNextInSlot; }

			void				ClearTrack();

	static	void				GetStringOfCell( String& rStr,
									const ScBaseCell* pCell,
									const ScDocument* pDoc,
									const ScAddress& rPos );

	static	void				GetStringOfCell( String& rStr,
									const ScBaseCell* pCell,
									const ScDocument* pDoc,
									sal_uLong nFormat );

	static	void				SetValue( String& rStr, ScBaseCell*& pCell,
									const ScAddress& rPos,
									const ScBaseCell* pOrgCell,
									const ScDocument* pFromDoc,
									ScDocument* pToDoc );

	static	void				SetValue( String& rStr, ScBaseCell*& pCell,
									sal_uLong nFormat,
									const ScBaseCell* pOrgCell,
									const ScDocument* pFromDoc,
									ScDocument* pToDoc );

	static	void				SetCell( String& rStr, ScBaseCell* pCell,
									sal_uLong nFormat, const ScDocument* pDoc );

	static	sal_Bool				NeedsNumberFormat( const ScBaseCell* );

			void				SetValueString( String& rValue,
									ScBaseCell*& pCell,	const String& rStr,
									ScDocument* pDoc );

			void				GetValueString( String& rStr,
									const String& rValue,
									const ScBaseCell* pCell ) const;

			void				GetFormulaString( String& rStr,
									const ScFormulaCell* pCell ) const;

	virtual	void				AddContent( ScChangeActionContent* ) {}
	virtual	void				DeleteCellEntries() {}

	virtual	void 				UpdateReference( const ScChangeTrack*,
									UpdateRefMode, const ScBigRange&,
									sal_Int32 nDx, sal_Int32 nDy, sal_Int32 nDz );

	virtual	sal_Bool				Reject( ScDocument* );

    virtual const ScChangeTrack*    GetChangeTrack() const { return 0; }

								// pRejectActions!=NULL: reject actions get
								// stacked, no SetNewValue, no Append
			sal_Bool				Select( ScDocument*, ScChangeTrack*,
									sal_Bool bOldest, Stack* pRejectActions );

			void				PutValueToDoc( ScBaseCell*, const String&,
									ScDocument*, SCsCOL nDx, SCsROW nDy ) const;

protected:
    using ScChangeAction::GetRefString;

public:

	DECL_FIXEDMEMPOOL_NEWDEL( ScChangeActionContent )

								ScChangeActionContent( const ScRange& rRange )
									: ScChangeAction( SC_CAT_CONTENT, rRange ),
										pOldCell( NULL ),
										pNewCell( NULL ),
										pNextContent( NULL ),
										pPrevContent( NULL ),
										pNextInSlot( NULL ),
										ppPrevInSlot( NULL )
									{}
								ScChangeActionContent(const sal_uLong nActionNumber,
												const ScChangeActionState eState,
												const sal_uLong nRejectingNumber,
												const ScBigRange& aBigRange,
												const String& aUser,
												const DateTime& aDateTime,
												const String &sComment,
												ScBaseCell* pOldCell,
												ScDocument* pDoc,
												const String& sOldValue); // to use for XML Import
								ScChangeActionContent(const sal_uLong nActionNumber,
												ScBaseCell* pNewCell,
												const ScBigRange& aBigRange,
												ScDocument* pDoc,
                                                const String& sNewValue); // to use for XML Import of Generated Actions
	virtual						~ScChangeActionContent();

		ScChangeActionContent*	GetNextContent() const { return pNextContent; }
		ScChangeActionContent*	GetPrevContent() const { return pPrevContent; }
		ScChangeActionContent*	GetTopContent() const;
			sal_Bool				IsTopContent() const
									{ return pNextContent == NULL; }

	virtual	ScChangeActionLinkEntry*  	GetDeletedIn() const;
	virtual	ScChangeActionLinkEntry**	GetDeletedInAddress();

			void				PutOldValueToDoc( ScDocument*,
									SCsCOL nDx, SCsROW nDy ) const;
			void				PutNewValueToDoc( ScDocument*,
									SCsCOL nDx, SCsROW nDy ) const;

			void				SetOldValue( const ScBaseCell*,
									const ScDocument* pFromDoc,
									ScDocument* pToDoc,
									sal_uLong nFormat );
			void				SetOldValue( const ScBaseCell*,
									const ScDocument* pFromDoc,
									ScDocument* pToDoc );
			void				SetNewValue( const ScBaseCell*,	ScDocument* );

								// Used in import filter AppendContentOnTheFly,
								// takes ownership of cells.
			void				SetOldNewCells( ScBaseCell* pOldCell,
									sal_uLong nOldFormat, ScBaseCell* pNewCell,
									sal_uLong nNewFormat, ScDocument* pDoc );

								// Use this only in the XML import,
								// takes ownership of cell.
			void				SetNewCell( ScBaseCell* pCell, ScDocument* pDoc, const String& rFormatted );

								// These functions should be protected but for
								// the XML import they are public.
			void				SetNextContent( ScChangeActionContent* p )
									{ pNextContent = p; }
			void				SetPrevContent( ScChangeActionContent* p )
									{ pPrevContent = p; }

								// moeglichst nicht verwenden,
								// setzt nur String bzw. generiert Formelzelle
			void				SetOldValue( const String& rOld, ScDocument* );
			void				SetNewValue( const String& rNew, ScDocument* );

			void				GetOldString( String& ) const;
			void				GetNewString( String& ) const;
			const ScBaseCell*	GetOldCell() const { return pOldCell; }
			const ScBaseCell*	GetNewCell() const { return pNewCell; }
	virtual	void				GetDescription( String&, ScDocument*,
									sal_Bool bSplitRange = sal_False, bool bWarning = true ) const;
	virtual void				GetRefString( String&, ScDocument*,
									sal_Bool bFlag3D = sal_False ) const;

	static	ScChangeActionContentCellType	GetContentCellType( const ScBaseCell* );

								// NewCell
			sal_Bool				IsMatrixOrigin() const
									{
										return GetContentCellType( GetNewCell() )
											== SC_CACCT_MATORG;
									}
			sal_Bool				IsMatrixReference() const
									{
										return GetContentCellType( GetNewCell() )
											== SC_CACCT_MATREF;
									}
								// OldCell
			sal_Bool				IsOldMatrixOrigin() const
									{
										return GetContentCellType( GetOldCell() )
											== SC_CACCT_MATORG;
									}
			sal_Bool				IsOldMatrixReference() const
									{
										return GetContentCellType( GetOldCell() )
											== SC_CACCT_MATREF;
									}

};


// --- ScChangeActionReject -------------------------------------------------

class Stack;

class ScChangeActionReject : public ScChangeAction
{
	friend class ScChangeTrack;
	friend class ScChangeActionContent;

								ScChangeActionReject( sal_uLong nReject )
									: ScChangeAction( SC_CAT_REJECT, ScRange() )
									{
										SetRejectAction( nReject );
										SetState( SC_CAS_ACCEPTED );
									}

	virtual	void				AddContent( ScChangeActionContent* ) {}
	virtual	void				DeleteCellEntries() {}

	virtual	sal_Bool				Reject( ScDocument* ) { return sal_False; }

    virtual const ScChangeTrack*    GetChangeTrack() const { return 0; }

public:
								ScChangeActionReject(const sal_uLong nActionNumber,
												const ScChangeActionState eState,
												const sal_uLong nRejectingNumber,
												const ScBigRange& aBigRange,
												const String& aUser,
												const DateTime& aDateTime,
												const String &sComment); // only to use in the XML import
};


// --- ScChangeTrack --------------------------------------------------------

enum ScChangeTrackMsgType
{
	SC_CTM_NONE,
	SC_CTM_APPEND,		// Actions angehaengt
	SC_CTM_REMOVE,		// Actions weggenommen
	SC_CTM_CHANGE,		// Actions geaendert
	SC_CTM_PARENT		// war kein Parent und ist jetzt einer
};

struct ScChangeTrackMsgInfo
{
	DECL_FIXEDMEMPOOL_NEWDEL( ScChangeTrackMsgInfo )

	ScChangeTrackMsgType	eMsgType;
	sal_uLong					nStartAction;
	sal_uLong					nEndAction;
};

// MsgQueue fuer Benachrichtigung via ModifiedLink
DECLARE_QUEUE( ScChangeTrackMsgQueue, ScChangeTrackMsgInfo* )
DECLARE_STACK( ScChangeTrackMsgStack, ScChangeTrackMsgInfo* )

enum ScChangeTrackMergeState
{
	SC_CTMS_NONE,
	SC_CTMS_PREPARE,
	SC_CTMS_OWN,
    SC_CTMS_UNDO,
	SC_CTMS_OTHER
};

// zusaetzlich zu pFirst/pNext/pLast/pPrev eine Table, um schnell sowohl
// per ActionNumber als auch ueber Liste zugreifen zu koennen
DECLARE_TABLE( ScChangeActionTable, ScChangeAction* )

// Intern generierte Actions beginnen bei diesem Wert (fast alle Bits gesetzt)
// und werden runtergezaehlt, um sich in einer Table wertemaessig nicht mit den
// "normalen" Actions in die Quere zu kommen.
#define SC_CHGTRACK_GENERATED_START	((sal_uInt32) 0xfffffff0)

class ScChangeTrack : public utl::ConfigurationListener
{
	friend void ScChangeAction::RejectRestoreContents( ScChangeTrack*, SCsCOL, SCsROW );
	friend sal_Bool ScChangeActionDel::Reject( ScDocument* pDoc );
	friend void ScChangeActionDel::DeleteCellEntries();
	friend void ScChangeActionMove::DeleteCellEntries();
	friend sal_Bool ScChangeActionMove::Reject( ScDocument* pDoc );

    static	const SCROW         nContentRowsPerSlot;
    static	const SCSIZE        nContentSlots;

	com::sun::star::uno::Sequence< sal_Int8 >	aProtectPass;
			ScChangeActionTable	aTable;
			ScChangeActionTable	aGeneratedTable;
			ScChangeActionTable	aPasteCutTable;
		ScChangeTrackMsgQueue	aMsgQueue;
		ScChangeTrackMsgStack	aMsgStackTmp;
		ScChangeTrackMsgStack	aMsgStackFinal;
			ScStrCollection		aUserCollection;
			String				aUser;
			Link				aModifiedLink;
			ScRange				aInDeleteRange;
			DateTime			aFixDateTime;
			ScChangeAction*		pFirst;
			ScChangeAction*		pLast;
		ScChangeActionContent*	pFirstGeneratedDelContent;
		ScChangeActionContent**	ppContentSlots;
		ScChangeActionMove*		pLastCutMove;
	ScChangeActionLinkEntry*	pLinkInsertCol;
	ScChangeActionLinkEntry*	pLinkInsertRow;
	ScChangeActionLinkEntry*	pLinkInsertTab;
	ScChangeActionLinkEntry*	pLinkMove;
		ScChangeTrackMsgInfo*	pBlockModifyMsg;
			ScDocument*			pDoc;
			sal_uLong				nActionMax;
			sal_uLong				nGeneratedMin;
			sal_uLong				nMarkLastSaved;
			sal_uLong				nStartLastCut;
			sal_uLong				nEndLastCut;
			sal_uLong				nLastMerge;
		ScChangeTrackMergeState	eMergeState;
			sal_uInt16				nLoadedFileFormatVersion;
			sal_Bool				bLoadSave;
			sal_Bool				bInDelete;
			sal_Bool				bInDeleteUndo;
			sal_Bool				bInDeleteTop;
			sal_Bool				bInPasteCut;
			sal_Bool				bUseFixDateTime;
            sal_Bool                bTime100thSeconds;

								// not implemented, prevent usage
								ScChangeTrack( const ScChangeTrack& );
			ScChangeTrack&		operator=( const ScChangeTrack& );

#ifdef SC_CHGTRACK_CXX
	static	SCROW				InitContentRowsPerSlot();

								// sal_True if one is MM_FORMULA and the other is
								// not, or if both are and range differs
	static	sal_Bool				IsMatrixFormulaRangeDifferent(
									const ScBaseCell* pOldCell,
									const ScBaseCell* pNewCell );

			void				Init();
			void				DtorClear();
			void				SetLoadSave( sal_Bool bVal ) { bLoadSave = bVal; }
			void				SetInDeleteRange( const ScRange& rRange )
									{ aInDeleteRange = rRange; }
			void				SetInDelete( sal_Bool bVal )
									{ bInDelete = bVal; }
			void				SetInDeleteTop( sal_Bool bVal )
									{ bInDeleteTop = bVal; }
			void				SetInDeleteUndo( sal_Bool bVal )
									{ bInDeleteUndo = bVal; }
			void				SetInPasteCut( sal_Bool bVal )
									{ bInPasteCut = bVal; }
			void				SetMergeState( ScChangeTrackMergeState eState )
									{ eMergeState = eState; }
		ScChangeTrackMergeState	GetMergeState() const { return eMergeState; }
			void				SetLastMerge( sal_uLong nVal ) { nLastMerge = nVal; }
			sal_uLong				GetLastMerge() const { return nLastMerge; }

			void				SetLastCutMoveRange( const ScRange&, ScDocument* );

								// ModifyMsg blockweise und nicht einzeln erzeugen
			void				StartBlockModify( ScChangeTrackMsgType,
									sal_uLong nStartAction );
			void				EndBlockModify( sal_uLong nEndAction );

			void				AddDependentWithNotify( ScChangeAction* pParent,
									ScChangeAction* pDependent );

			void				Dependencies( ScChangeAction* );
			void				UpdateReference( ScChangeAction*, sal_Bool bUndo );
			void				UpdateReference( ScChangeAction** ppFirstAction,
									ScChangeAction* pAct, sal_Bool bUndo );
			void				Append( ScChangeAction* pAppend, sal_uLong nAction );
	SC_DLLPUBLIC		void				AppendDeleteRange( const ScRange&,
									ScDocument* pRefDoc, SCsTAB nDz,
									sal_uLong nRejectingInsert );
			void				AppendOneDeleteRange( const ScRange& rOrgRange,
									ScDocument* pRefDoc,
									SCsCOL nDx, SCsROW nDy, SCsTAB nDz,
									sal_uLong nRejectingInsert );
			void				LookUpContents( const ScRange& rOrgRange,
									ScDocument* pRefDoc,
									SCsCOL nDx, SCsROW nDy, SCsTAB nDz );
			void				Remove( ScChangeAction* );
			void				MasterLinks( ScChangeAction* );

								// Content on top an Position
		ScChangeActionContent*	SearchContentAt( const ScBigAddress&,
									ScChangeAction* pButNotThis ) const;
			void				DeleteGeneratedDelContent(
									ScChangeActionContent* );
		ScChangeActionContent*	GenerateDelContent( const ScAddress&,
									const ScBaseCell*,
									const ScDocument* pFromDoc );
			void				DeleteCellEntries(
									ScChangeActionCellListEntry*&,
									ScChangeAction* pDeletor );

								// Action und alle abhaengigen rejecten,
								// Table stammt aus vorherigem GetDependents,
								// ist nur bei Insert und Move (MasterType)
								// noetig, kann ansonsten NULL sein.
								// bRecursion == Aufruf aus Reject mit Table
			sal_Bool				Reject( ScChangeAction*,
									ScChangeActionTable*, sal_Bool bRecursion );

#endif	// SC_CHGTRACK_CXX

			void				ClearMsgQueue();
    virtual void                ConfigurationChanged( utl::ConfigurationBroadcaster*, sal_uInt32 );

public:

	static	SCSIZE				ComputeContentSlot( sal_Int32 nRow )
									{
										if ( nRow < 0 || nRow > MAXROW )
											return nContentSlots - 1;
                                        return static_cast< SCSIZE >( nRow / nContentRowsPerSlot );
									}

            SC_DLLPUBLIC        ScChangeTrack( ScDocument* );
								ScChangeTrack( ScDocument*,
											const ScStrCollection& ); // only to use in the XML import
            SC_DLLPUBLIC virtual ~ScChangeTrack();
			void				Clear();

			ScChangeActionContent*	GetFirstGenerated() const { return pFirstGeneratedDelContent; }
			ScChangeAction*		GetFirst() const { return pFirst; }
			ScChangeAction*		GetLast() const	{ return pLast; }
			sal_uLong				GetActionMax() const { return nActionMax; }
			sal_Bool				IsGenerated( sal_uLong nAction ) const
									{ return nAction >= nGeneratedMin; }
			ScChangeAction*		GetAction( sal_uLong nAction ) const
									{ return aTable.Get( nAction ); }
			ScChangeAction*		GetGenerated( sal_uLong nGenerated ) const
									{ return aGeneratedTable.Get( nGenerated ); }
			ScChangeAction*		GetActionOrGenerated( sal_uLong nAction ) const
									{
										return IsGenerated( nAction ) ?
											GetGenerated( nAction ) :
											GetAction( nAction );
									}
			sal_uLong				GetLastSavedActionNumber() const
									{ return nMarkLastSaved; }
            void                SetLastSavedActionNumber(sal_uLong nNew)
                                    { nMarkLastSaved = nNew; }
			ScChangeAction*		GetLastSaved() const
									{ return aTable.Get( nMarkLastSaved ); }
		ScChangeActionContent**	GetContentSlots() const { return ppContentSlots; }

			sal_Bool				IsLoadSave() const { return bLoadSave; }
			const ScRange&		GetInDeleteRange() const
									{ return aInDeleteRange; }
			sal_Bool				IsInDelete() const { return bInDelete; }
			sal_Bool				IsInDeleteTop() const { return bInDeleteTop; }
			sal_Bool				IsInDeleteUndo() const { return bInDeleteUndo; }
			sal_Bool				IsInPasteCut() const { return bInPasteCut; }
	SC_DLLPUBLIC		void				SetUser( const String& );
			const String&		GetUser() const { return aUser; }
			const ScStrCollection&	GetUserCollection() const
									{ return aUserCollection; }
			ScDocument*			GetDocument() const { return pDoc; }
								// for import filter
			const DateTime&		GetFixDateTime() const { return aFixDateTime; }

								// set this if the date/time set with
								// SetFixDateTime...() shall be applied to
								// appended actions
			void				SetUseFixDateTime( sal_Bool bVal )
									{ bUseFixDateTime = bVal; }
								// for MergeDocument, apply original date/time as UTC
			void				SetFixDateTimeUTC( const DateTime& rDT )
									{ aFixDateTime = rDT; }
								// for import filter, apply original date/time as local time
			void				SetFixDateTimeLocal( const DateTime& rDT )
									{ aFixDateTime = rDT; aFixDateTime.ConvertToUTC(); }

			void				Append( ScChangeAction* );

								// pRefDoc may be NULL => no lookup of contents
								// => no generation of deleted contents
	SC_DLLPUBLIC		void				AppendDeleteRange( const ScRange&,
									ScDocument* pRefDoc,
									sal_uLong& nStartAction, sal_uLong& nEndAction,
									SCsTAB nDz = 0 );
									// nDz: Multi-TabDel, LookUpContent ist
									// um -nDz verschoben zu suchen

								// nachdem neuer Wert im Dokument gesetzt wurde,
								// alter Wert aus RefDoc/UndoDoc
			void				AppendContent( const ScAddress& rPos,
									ScDocument* pRefDoc );
								// nachdem neue Werte im Dokument gesetzt wurden,
								// alte Werte aus RefDoc/UndoDoc
			void				AppendContentRange( const ScRange& rRange,
									ScDocument* pRefDoc,
									sal_uLong& nStartAction, sal_uLong& nEndAction,
									ScChangeActionClipMode eMode = SC_CACM_NONE );
								// nachdem neuer Wert im Dokument gesetzt wurde,
								// alter Wert aus pOldCell, nOldFormat,
								// RefDoc==NULL => Doc
			void				AppendContent( const ScAddress& rPos,
									const ScBaseCell* pOldCell,
									sal_uLong nOldFormat, ScDocument* pRefDoc = NULL );
								// nachdem neuer Wert im Dokument gesetzt wurde,
								// alter Wert aus pOldCell, Format aus Doc
			void				AppendContent( const ScAddress& rPos,
									const ScBaseCell* pOldCell );
								// nachdem neue Werte im Dokument gesetzt wurden,
								// alte Werte aus RefDoc/UndoDoc.
								// Alle Contents, wo im RefDoc eine Zelle steht.
			void				AppendContentsIfInRefDoc( ScDocument* pRefDoc,
									sal_uLong& nStartAction, sal_uLong& nEndAction );

								// Meant for import filter, creates and inserts
								// an unconditional content action of the two
								// cells without querying the document, not
								// even for number formats (though the number
								// formatter of the document may be used).
								// The action is returned and may be used to
								// set user name, description, date/time et al.
								// Takes ownership of the cells!
	SC_DLLPUBLIC	ScChangeActionContent*	AppendContentOnTheFly( const ScAddress& rPos,
									ScBaseCell* pOldCell,
									ScBaseCell* pNewCell,
									sal_uLong nOldFormat = 0,
									sal_uLong nNewFormat = 0 );

								// die folgenden beiden nur benutzen wenn's
								// nicht anders geht (setzen nur String fuer
								// NewValue bzw. Formelerzeugung)

								// bevor neuer Wert im Dokument gesetzt wird
			void				AppendContent( const ScAddress& rPos,
									const String& rNewValue,
									ScBaseCell* pOldCell );

	SC_DLLPUBLIC		void				AppendInsert( const ScRange& );

								// pRefDoc may be NULL => no lookup of contents
								// => no generation of deleted contents
	SC_DLLPUBLIC		void				AppendMove( const ScRange& rFromRange,
									const ScRange& rToRange,
									ScDocument* pRefDoc );

								// Cut to Clipboard
			void				ResetLastCut()
									{
										nStartLastCut = nEndLastCut = 0;
										if ( pLastCutMove )
										{
											delete pLastCutMove;
											pLastCutMove = NULL;
										}
									}
			sal_Bool				HasLastCut() const
									{
										return nEndLastCut > 0 &&
											nStartLastCut <= nEndLastCut &&
											pLastCutMove;
									}

	SC_DLLPUBLIC		void				Undo( sal_uLong nStartAction, sal_uLong nEndAction, bool bMerge = false );

								// fuer MergeDocument, Referenzen anpassen,
								//! darf nur in einem temporaer geoeffneten
								//! Dokument verwendet werden, der Track
								//! ist danach verhunzt
			void				MergePrepare( ScChangeAction* pFirstMerge, bool bShared = false );
			void				MergeOwn( ScChangeAction* pAct, sal_uLong nFirstMerge, bool bShared = false );
	static	sal_Bool				MergeIgnore( const ScChangeAction&, sal_uLong nFirstMerge );

								// Abhaengige in Table einfuegen.
								// Bei Insert sind es echte Abhaengige,
								// bei Move abhaengige Contents im FromRange
								// und geloeschte im ToRange bzw. Inserts in
								// FromRange oder ToRange,
								// bei Delete eine Liste der geloeschten,
								// bei Content andere Contents an gleicher
								// Position oder MatrixReferences zu MatrixOrigin.
								// Mit bListMasterDelete werden unter einem
								// MasterDelete alle zu diesem Delete gehoerenden
								// Deletes einer Reihe gelistet.
								// Mit bAllFlat werden auch alle Abhaengigen
								// der Abhaengigen flach eingefuegt.
	SC_DLLPUBLIC		void				GetDependents( ScChangeAction*,
									ScChangeActionTable&,
									sal_Bool bListMasterDelete = sal_False,
									sal_Bool bAllFlat = sal_False ) const;

								// Reject visible Action (und abhaengige)
            sal_Bool                Reject( ScChangeAction*, bool bShared = false );

								// Accept visible Action (und abhaengige)
	SC_DLLPUBLIC		sal_Bool				Accept( ScChangeAction* );

			void				AcceptAll();	// alle Virgins
			sal_Bool				RejectAll();	// alle Virgins

								// Selektiert einen Content von mehreren an
								// gleicher Position und akzeptiert diesen und
								// die aelteren, rejected die neueren.
								// Mit bOldest==sal_True wird der erste OldValue
								// einer Virgin-Content-Kette restauriert.
			sal_Bool				SelectContent( ScChangeAction*,
									sal_Bool bOldest = sal_False );

								// wenn ModifiedLink gesetzt, landen
								// Aenderungen in ScChangeTrackMsgQueue
			void				SetModifiedLink( const Link& r )
									{ aModifiedLink = r; ClearMsgQueue(); }
			const Link&			GetModifiedLink() const { return aModifiedLink; }
			ScChangeTrackMsgQueue& GetMsgQueue() { return aMsgQueue; }

			void				NotifyModified( ScChangeTrackMsgType eMsgType,
									sal_uLong nStartAction, sal_uLong nEndAction );

			sal_uInt16				GetLoadedFileFormatVersion() const
									{ return nLoadedFileFormatVersion; }

			sal_uLong				AddLoadedGenerated(ScBaseCell* pOldCell,
												const ScBigRange& aBigRange, const String& sNewValue ); // only to use in the XML import
			void				AppendLoaded( ScChangeAction* pAppend ); // this is only for the XML import public, it should be protected
			void				SetActionMax(sal_uLong nTempActionMax)
									{ nActionMax = nTempActionMax; } // only to use in the XML import

            void                SetProtection( const com::sun::star::uno::Sequence< sal_Int8 >& rPass )
                                    { aProtectPass = rPass; }
    com::sun::star::uno::Sequence< sal_Int8 >   GetProtection() const
                                    { return aProtectPass; }
            sal_Bool                IsProtected() const
                                    { return aProtectPass.getLength() != 0; }

                                // If time stamps of actions of this
                                // ChangeTrack and a second one are to be
                                // compared including 100th seconds.
            void                SetTime100thSeconds( sal_Bool bVal )
                                    { bTime100thSeconds = bVal; }
            sal_Bool                IsTime100thSeconds() const
                                    { return bTime100thSeconds; }

            void                AppendCloned( ScChangeAction* pAppend );
    SC_DLLPUBLIC ScChangeTrack* Clone( ScDocument* pDocument ) const;
            void                MergeActionState( ScChangeAction* pAct, const ScChangeAction* pOtherAct );
#if DEBUG_CHANGETRACK
            String              ToString() const;
#endif // DEBUG_CHANGETRACK
};


#endif


