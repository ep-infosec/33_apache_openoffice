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



#ifndef _SB_SBSTAR_HXX
#define _SB_SBSTAR_HXX

#include "basic/basicdllapi.h"
#include <basic/sbx.hxx>
#include <basic/sbxobj.hxx>
#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif
#include <osl/mutex.hxx>

#include <basic/sbdef.hxx>
#include <basic/sberrors.hxx>
#include <com/sun/star/script/ModuleInfo.hpp>
#include <com/sun/star/frame/XModel.hpp>

class SbModule;                     // completed module
class SbiInstance;                  // runtime instance
class SbiRuntime;                   // currently running procedure
class SbiImage;                     // compiled image
class BasicLibInfo;                 // info block for basic manager
class SbTextPortions;
class SbMethod;
class BasicManager;
class DocBasicItem;

class StarBASICImpl;

class BASIC_DLLPUBLIC StarBASIC : public SbxObject
{
	friend class SbiScanner;
	friend class SbiExpression; // Access to RTL
	friend class SbiInstance;
	friend class SbiRuntime;
	friend class DocBasicItem;

	StarBASICImpl*	mpStarBASICImpl;

	SbxArrayRef		pModules;               // List of all modules
	SbxObjectRef	pRtl;				// Runtime Library
	SbxArrayRef		xUnoListeners;          // Listener handled by CreateUnoListener
	
   // Handler-Support:
	Link			aErrorHdl;              // Error handler
	Link			aBreakHdl;              // Breakpoint handler
	sal_Bool			bNoRtl;                 // if sal_True: do not search RTL
	sal_Bool			bBreak;                 // if sal_True: Break, otherwise Step
	sal_Bool			bDocBasic;
	sal_Bool			bVBAEnabled;
	BasicLibInfo*	pLibInfo;			// Info block for basic manager
	SbLanguageMode	eLanguageMode;		// LanguageMode of the basic object
	sal_Bool			bQuit;

	SbxObjectRef pVBAGlobals; 
	SbxObject* getVBAGlobals( );

	void implClearDependingVarsOnDelete( StarBASIC* pDeletedBasic );

protected:
	sal_Bool 			CError( SbError, const String&, xub_StrLen, xub_StrLen, xub_StrLen );
private:
	sal_Bool 			RTError( SbError, xub_StrLen, xub_StrLen, xub_StrLen );
	sal_Bool 			RTError( SbError, const String& rMsg, xub_StrLen, xub_StrLen, xub_StrLen );
	sal_uInt16 			BreakPoint( xub_StrLen nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
	sal_uInt16 			StepPoint( xub_StrLen nLine, xub_StrLen nCol1, xub_StrLen nCol2 );
	virtual sal_Bool LoadData( SvStream&, sal_uInt16 );
	virtual sal_Bool StoreData( SvStream& ) const;

protected:

	virtual	sal_Bool  	ErrorHdl();
	virtual	sal_uInt16	BreakHdl();
	virtual ~StarBASIC();

public:

	SBX_DECL_PERSIST_NODATA(SBXCR_SBX,SBXID_BASIC,1);
	TYPEINFO();

	StarBASIC( StarBASIC* pParent = NULL, sal_Bool bIsDocBasic = sal_False );

	// #51727 SetModified overridden so that the Modfied-State is
        // not delivered to Parent.
	virtual void SetModified( sal_Bool );

	void* operator 	new( size_t );
	void operator 	delete( void* );

	virtual void    Insert( SbxVariable* );
	using SbxObject::Remove;
	virtual void    Remove( SbxVariable* );
	virtual void	Clear();

	BasicLibInfo*	GetLibInfo()					{ return pLibInfo;	}
	void			SetLibInfo( BasicLibInfo* p )   { pLibInfo = p;		}

	// Compiler-Interface
	SbModule*   	MakeModule( const String& rName, const String& rSrc );
    SbModule*   	MakeModule32( const String& rName, const ::rtl::OUString& rSrc );
    SbModule*       MakeModule32( const String& rName, const com::sun::star::script::ModuleInfo& mInfo, const ::rtl::OUString& rSrc );
	sal_Bool			Compile( SbModule* );
	sal_Bool 			Disassemble( SbModule*, String& rText );
	static void 	Stop();
	static void 	Error( SbError );
	static void 	Error( SbError, const String& rMsg );
	static void 	FatalError( SbError );
	static void 	FatalError( SbError, const String& rMsg );
	static sal_Bool 	IsRunning();
	static SbError 	GetErrBasic();
	// #66536 make additional message accessible by RTL function Error
	static String	GetErrorMsg();
	static xub_StrLen GetErl();
	// Highlighting
	void 			Highlight( const String& rSrc, SbTextPortions& rList );

	virtual SbxVariable* Find( const String&, SbxClassType );
	virtual sal_Bool Call( const String&, SbxArray* = NULL );

	SbxArray*		GetModules() { return pModules; }
	SbxObject*		GetRtl()	 { return pRtl;		}
	SbModule*		FindModule( const String& );
	// Run init code of all modules (including the inserted Doc-Basics)
	void			InitAllModules( StarBASIC* pBasicNotToInit = NULL );
	void			DeInitAllModules( void );
	void			ClearAllModuleVars( void );
	void			ActivateObject( const String*, sal_Bool );
	sal_Bool 			LoadOldModules( SvStream& );

	// #43011 For TestTool; deletes global vars
	void			ClearGlobalVars( void );

	// Calls for error and break handler
	static sal_uInt16	GetLine();
	static sal_uInt16	GetCol1();
	static sal_uInt16	GetCol2();
	static void		SetErrorData( SbError nCode, sal_uInt16 nLine,
								  sal_uInt16 nCol1, sal_uInt16 nCol2 );

	// Specific to error handler
	static void		MakeErrorText( SbError, const String& aMsg );
	static const	String&	GetErrorText();
	static SbError	GetErrorCode();
	static sal_Bool		IsCompilerError();
	static sal_uInt16	GetVBErrorCode( SbError nError );
	static SbError	GetSfxFromVBError( sal_uInt16 nError );
	static void		SetGlobalLanguageMode( SbLanguageMode eLangMode );
	static SbLanguageMode GetGlobalLanguageMode();
	// Local settings
	void SetLanguageMode( SbLanguageMode eLangMode )
		{ eLanguageMode = eLangMode; }
	SbLanguageMode GetLanguageMode();

	// Specific for break handler
	sal_Bool   			IsBreak() const				{ return bBreak; }

	static Link 	GetGlobalErrorHdl();
	static void 	SetGlobalErrorHdl( const Link& rNewHdl );
	Link 			GetErrorHdl() const { return aErrorHdl; }
	void 			SetErrorHdl( const Link& r ) { aErrorHdl = r; }

	static Link 	GetGlobalBreakHdl();
	static void 	SetGlobalBreakHdl( const Link& rNewHdl );
	Link 			GetBreakHdl() const { return aBreakHdl; }
	void 			SetBreakHdl( const Link& r ) { aBreakHdl = r; }

    SbxArrayRef		getUnoListeners( void );

	static SbxBase*	FindSBXInCurrentScope( const String& rName );
	static SbxVariable*	FindVarInCurrentScopy
					( const String& rName, sal_uInt16& rStatus );
	static SbMethod* GetActiveMethod( sal_uInt16 nLevel = 0 );
	static SbModule* GetActiveModule();
	void SetVBAEnabled( sal_Bool bEnabled );
	sal_Bool isVBAEnabled();

	// #60175 sal_True: SFX-Resource is not displayed on basic errors
	static void StaticSuppressSfxResource( sal_Bool bSuppress );

	// #91147 sal_True: Reschedule is enabled (default>, sal_False: No reschedule
	static void StaticEnableReschedule( sal_Bool bReschedule );

	SbxObjectRef getRTL( void ) { return pRtl; }
	sal_Bool IsDocBasic() { return bDocBasic; }
	SbxVariable* VBAFind( const String& rName, SbxClassType t );
	bool GetUNOConstant( const sal_Char* _pAsciiName, ::com::sun::star::uno::Any& aOut );
	void QuitAndExitApplication();
	sal_Bool IsQuitApplication() { return bQuit; };

    static ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >
        GetModelFromBasic( SbxObject* pBasic );
};

#ifndef __SB_SBSTARBASICREF_HXX
#define __SB_SBSTARBASICREF_HXX

SV_DECL_IMPL_REF(StarBASIC)

#endif

#endif

