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


#ifndef _SWDOCSH_HXX
#define _SWDOCSH_HXX

#include <rtl/ref.hxx>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/uno/Sequence.h>
#include <vcl/timer.hxx>
#include <sfx2/docfac.hxx>
#include <sfx2/objsh.hxx>
#include "swdllapi.h"
#include <swdll.hxx>
#include <shellid.hxx>

#include <svl/lstner.hxx>
#include <svtools/embedhlp.hxx>

class SwDoc;
class SfxDocumentInfoDialog;
class SfxStyleSheetBasePool;
class SfxInPlaceClient;
class FontList;
class SwView;
class SwWrtShell;
class SwFEShell;
class Reader;
class SwReader;
class SwCrsrShell;
class SwSrcView;
class SwPaM;
class SwgReaderOption;
class SwOLEObj;
class IDocumentDeviceAccess;
class IDocumentSettingAccess;
class IDocumentTimerAccess;
class IDocumentChartDataProviderAccess;
class SwDocShell;
class SwDrawModel;

// initialize DrawModel (in form of a SwDrawModel) and DocShell (in form of a SwDocShell)
// as needed, one or both parameters may be zero
void SW_DLLPRIVATE InitDrawModelAndDocShell(SwDocShell* pSwDocShell, SwDrawModel* pSwDrawModel);

class SW_DLLPUBLIC SwDocShell: public SfxObjectShell, public SfxListener
{
    SwDoc* mpDoc; // Document

    rtl::Reference< SfxStyleSheetBasePool > mxBasePool; // Durchreiche fuer Formate

    FontList* mpFontList; // aktuelle FontListe
    bool mbInUpdateFontList; //prevent nested calls of UpdateFontList

    // Nix geht ohne die WrtShell (historische Gruende)
    // RuekwaertsPointer auf die View (historische Gruende)
    // Dieser gilt solange bis im Activate ein neuer gesetzt wird
    // oder dieser im Dtor der View geloescht wird
    //
    SwView* mpView;
    SwWrtShell* mpWrtShell;

    Timer aFinishedTimer;	// Timer fuers ueberpriefen der
                            // Grafik-Links. Sind alle da,
                            // dann ist Doc voll. geladen

    comphelper::EmbeddedObjectContainer* mpOLEChildList;
    sal_Int16 mnUpdateDocMode; // contains the com::sun::star::document::UpdateDocMode

    // Methoden fuer den Zugriff aufs Doc
	SW_DLLPRIVATE void					AddLink();
	SW_DLLPRIVATE void					RemoveLink();

	// Hint abfangen fuer DocInfo
	SW_DLLPRIVATE virtual void			Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

	// FileIO
    SW_DLLPRIVATE virtual sal_Bool InitNew( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );
    SW_DLLPRIVATE virtual sal_Bool Load( SfxMedium& rMedium );
    SW_DLLPRIVATE virtual sal_Bool LoadFrom( SfxMedium& rMedium );
    SW_DLLPRIVATE virtual sal_Bool ConvertFrom( SfxMedium &rMedium );
    SW_DLLPRIVATE virtual sal_Bool ConvertTo( SfxMedium &rMedium );
    SW_DLLPRIVATE virtual sal_Bool SaveAs( SfxMedium& rMedium );
    SW_DLLPRIVATE virtual sal_Bool SaveCompleted( const ::com::sun::star::uno::Reference< ::com::sun::star::embed::XStorage >& xStorage );

	SW_DLLPRIVATE virtual sal_uInt16			PrepareClose( sal_Bool bUI = sal_True, sal_Bool bForBrowsing = sal_False );

	// DocInfo dem Doc melden
	//
	SW_DLLPRIVATE virtual SfxDocumentInfoDialog* CreateDocumentInfoDialog(
									Window *pParent, const SfxItemSet &);
	// OLE-Geraffel
	SW_DLLPRIVATE virtual void			Draw( OutputDevice*, const JobSetup&, sal_uInt16);

    // Methoden fuer StyleSheets

    // @param nSlot
    // Only used for nFamily == SFX_STYLE_FAMILY_PAGE. Identifies optional Slot by which the edit is triggered.
    // Used to activate certain dialog pane
    SW_DLLPRIVATE sal_uInt16 Edit(
        const String &rName,
        const String& rParent,
        const sal_uInt16 nFamily,
        sal_uInt16 nMask,
        const sal_Bool bNew,
        const sal_uInt16 nSlot = 0,
        SwWrtShell* pActShell = 0,
        const sal_Bool bBasic = sal_False );

	SW_DLLPRIVATE sal_uInt16					Delete(const String &rName, sal_uInt16 nFamily);

    SW_DLLPRIVATE sal_uInt16 ApplyStyles(
        const String &rName,
        const sal_uInt16 nFamily,
        SwWrtShell* pShell = 0,
        const sal_uInt16 nMode = 0 );

	SW_DLLPRIVATE sal_uInt16					DoWaterCan( const String &rName, sal_uInt16 nFamily);
	SW_DLLPRIVATE sal_uInt16					UpdateStyle(const String &rName, sal_uInt16 nFamily, SwWrtShell* pShell = 0);
	SW_DLLPRIVATE sal_uInt16					MakeByExample(const String &rName,
											sal_uInt16 nFamily, sal_uInt16 nMask, SwWrtShell* pShell = 0);

	SW_DLLPRIVATE void					SubInitNew();   // fuer InitNew und HtmlSourceModus

	SW_DLLPRIVATE void 					RemoveOLEObjects();
	SW_DLLPRIVATE void					CalcLayoutForOLEObjects();

    SW_DLLPRIVATE void                    Init_Impl();
    SW_DLLPRIVATE DECL_STATIC_LINK( SwDocShell, IsLoadFinished, void* );


    using SfxObjectShell::GetVisArea;
    using SfxObjectShell::GetStyleFamilyBitmap;

protected:
    /// override to update text fields
    virtual void                DoFlushDocInfo();

    // override <SfxObjectShell>'s method which is called in <SfxObjectShell::ImportFrom(..)>.
    // <SfxObjectShell::ImportFrom(..)> is used by current import of Microsoft Word documents in OOXML file format.
    virtual void BeforeLoading( SfxMedium&, const ::rtl::OUString &, const ::rtl::OUString & );

public:
    using SotObject::GetInterface;

	// aber selbst implementieren
	SFX_DECL_INTERFACE(SW_DOCSHELL)
    SFX_DECL_OBJECTFACTORY()
	TYPEINFO();

    static SfxInterface *_GetInterface() { return GetStaticInterface(); }

	static rtl::OUString GetEventName( sal_Int32 nId );

	//Das Doc wird fuer SO-Datenaustausch benoetigt!
	SwDocShell( SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED );
	SwDocShell( const sal_uInt64 i_nSfxCreationFlags );
	SwDocShell( SwDoc *pDoc, SfxObjectCreateMode eMode = SFX_CREATE_MODE_STANDARD );
	~SwDocShell();

	// OLE 2.0-Benachrichtigung
	DECL_LINK( Ole2ModifiedHdl, void * );

	// OLE-Geraffel
	virtual void	  SetVisArea( const Rectangle &rRect );
	virtual Rectangle GetVisArea( sal_uInt16 nAspect ) const;
	virtual Printer  *GetDocumentPrinter();
    virtual OutputDevice* GetDocumentRefDev();
	virtual void	  OnDocumentPrinterChanged( Printer * pNewPrinter );
	virtual sal_uLong	  GetMiscStatus() const;

	virtual void			PrepareReload();
	virtual void			SetModified( sal_Bool = sal_True );

	// Dispatcher
	void					Execute(SfxRequest &);
	void					ExecStyleSheet(SfxRequest&);
	void					ExecDB(SfxRequest&);

	void					GetState(SfxItemSet &);
	void					StateAlways(SfxItemSet &);
	void					StateStyleSheet(SfxItemSet&, SwWrtShell* pSh = 0 );

	// Doc rausreichen aber VORSICHT
    inline SwDoc*                   GetDoc() { return mpDoc; }
    inline const SwDoc*             GetDoc() const { return mpDoc; }
    IDocumentDeviceAccess*          getIDocumentDeviceAccess();
    const IDocumentSettingAccess*   getIDocumentSettingAccess() const;
    IDocumentChartDataProviderAccess*       getIDocumentChartDataProviderAccess();


    void                    UpdateFontList();
	void					UpdateChildWindows();

	// globaler IO
	virtual sal_Bool			Save();

	// fuer VorlagenPI
	virtual SfxStyleSheetBasePool*	GetStyleSheetPool();

	// Fuer Organizer
	virtual sal_Bool Insert(SfxObjectShell &rSource,
						sal_uInt16	nSourceIdx1,
						sal_uInt16	nSourceIdx2,
						sal_uInt16	nSourceIdx3,
						sal_uInt16& nIdx1,
						sal_uInt16& nIdx2,
						sal_uInt16& nIdx3,
						sal_uInt16& nRemovedIdx);

	virtual sal_Bool Remove(sal_uInt16 nIdx1,
						sal_uInt16 nIdx2 = INDEX_IGNORE,
						sal_uInt16 nIdx3 = INDEX_IGNORE);

	virtual Bitmap 		GetStyleFamilyBitmap( SfxStyleFamily eFamily, BmpColorMode eColorMode );

	// View setzen fuer Aktionen ueber Shell
	void 		  SetView(SwView* pVw);
	const SwView *GetView() const { return mpView; }
    SwView       *GetView()       { return mpView; }

	// Zugriff auf die zur SwView gehoerige SwWrtShell
		  SwWrtShell *GetWrtShell() 	  { return mpWrtShell; }
	const SwWrtShell *GetWrtShell() const { return mpWrtShell; }

	// fuer die Core - die kennt die DocShell aber keine WrtShell!
		  SwFEShell *GetFEShell();
	const SwFEShell *GetFEShell() const
				{ return ((SwDocShell*)this)->GetFEShell(); }


	// Fuer Einfuegen Dokument
	Reader* StartConvertFrom(SfxMedium& rMedium, SwReader** ppRdr,
							SwCrsrShell* pCrsrSh = 0, SwPaM* pPaM = 0);

	virtual long DdeGetData( const String& rItem, const String& rMimeType,
							 ::com::sun::star::uno::Any & rValue );
	virtual long DdeSetData( const String& rItem, const String& rMimeType,
								const ::com::sun::star::uno::Any & rValue );
    virtual ::sfx2::SvLinkSource* DdeCreateLinkSource( const String& rItem );
	virtual void FillClass( SvGlobalName * pClassName,
								   sal_uInt32 * pClipFormat,
								   String * pAppName,
								   String * pLongUserName,
                                   String * pUserName,
                                   sal_Int32 nFileFormat,
                                   sal_Bool bTemplate = sal_False ) const;

	virtual void LoadStyles( SfxObjectShell& rSource );

    void _LoadStyles( SfxObjectShell& rSource, sal_Bool bPreserveCurrentDocument );

    // Show page style format dialog
    // @param nSlot
    // Identifies slot by which the dialog is triggered. Used to activate certain dialog pane
    void FormatPage(
        const String& rPage,
        const sal_uInt16 nSlot,
        SwWrtShell& rActShell );

    // --> OD 2006-11-07 #i59688#
    // linked graphics are now loaded on demand.
    // Thus, loading of linked graphics no longer needed and necessary for
    // the load of document being finished.
//    // Timer starten fuers ueberpruefen der Grafik-Links. Sind alle
//    // vollstaendig geladen, dann ist das Doc fertig
//    void StartLoadFinishedTimer();
    void LoadingFinished();
    // <--

	// eine Uebertragung wird abgebrochen (wird aus dem SFX gerufen)
	virtual void CancelTransfers();

	// Doc aus Html-Source neu laden
	void	ReloadFromHtml( const String& rStreamName, SwSrcView* pSrcView );

    sal_Int16   GetUpdateDocMode() const {return mnUpdateDocMode;}

	void ToggleBrowserMode(sal_Bool bOn, SwView* pView);

	sal_uLong LoadStylesFromFile( const String& rURL, SwgReaderOption& rOpt,
								sal_Bool bUnoCall );
	void InvalidateModel();
	void ReactivateModel();

	virtual ::com::sun::star::uno::Sequence< ::rtl::OUString >	GetEventNames();

    // --> FME 2004-08-05 #i20883# Digital Signatures and Encryption
    virtual sal_uInt16 GetHiddenInformationState( sal_uInt16 nStates );
    // <--

    // --> FME 2005-02-25 #i42634# Overwrites SfxObjectShell::UpdateLinks
    // This new function is necessary to trigger update of links in docs
    // read by the binary filter:
    virtual void UpdateLinks();
    // <--
	virtual void setDocAccTitle( const String& rTitle );
	virtual const String getDocAccTitle() const;

	void setDocReadOnly( sal_Bool bReadOnly);
	sal_Bool getDocReadOnly() const;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >
                                GetController();

    SfxInPlaceClient* GetIPClient( const ::svt::EmbeddedObjectRef& xObjRef );

    virtual const ::sfx2::IXmlIdRegistry* GetXmlIdRegistry() const;

    // passwword protection for Writer (derived from SfxObjectShell)
    // see also:    FN_REDLINE_ON, FN_REDLINE_ON
    virtual bool    IsChangeRecording() const;
    virtual bool    HasChangeRecordProtection() const;
    virtual void    SetChangeRecording( bool bActivate );
    virtual bool    SetProtectionPassword( const String &rPassword );
    virtual bool    GetProtectionHash( /*out*/ ::com::sun::star::uno::Sequence< sal_Int8 > &rPasswordHash );
};

class Graphic;
//implemented in source/ui/docvw/romenu.cxx
String ExportGraphic( const Graphic &rGraphic, const String &rGrfName );

#endif
