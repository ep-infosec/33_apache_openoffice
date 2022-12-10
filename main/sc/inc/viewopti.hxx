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



#ifndef SC_VIEWOPTI_HXX
#define SC_VIEWOPTI_HXX

#include <svx/optgrid.hxx>

#ifndef _SVX_SVXIDS_HRC //autogen
#include <svx/svxids.hrc>
#endif
#include "scdllapi.h"
#include "optutil.hxx"
#include "global.hxx"

//==================================================================
// View-Optionen
//==================================================================

enum ScViewOption
{
	VOPT_FORMULAS = 0,
	VOPT_NULLVALS,
	VOPT_SYNTAX,
	VOPT_NOTES,
	VOPT_VSCROLL,
	VOPT_HSCROLL,
	VOPT_TABCONTROLS,
	VOPT_OUTLINER,
	VOPT_HEADER,
	VOPT_GRID,
	VOPT_HELPLINES,
	VOPT_ANCHOR,
	VOPT_PAGEBREAKS,
	VOPT_SOLIDHANDLES,
	VOPT_CLIPMARKS,
	VOPT_BIGHANDLES
};

enum ScVObjType
{
	VOBJ_TYPE_OLE = 0,
	VOBJ_TYPE_CHART,
	VOBJ_TYPE_DRAW
};

#define MAX_OPT				(sal_uInt16)VOPT_BIGHANDLES+1
#define MAX_TYPE			(sal_uInt16)VOBJ_TYPE_DRAW+1

#define SC_STD_GRIDCOLOR	COL_LIGHTGRAY

//==================================================================
// SvxGrid-Optionen mit Standard-Operatoren
//==================================================================

class ScGridOptions : public SvxOptionsGrid
{
public:
				ScGridOptions() : SvxOptionsGrid() {}
				ScGridOptions( const SvxOptionsGrid& rOpt ) : SvxOptionsGrid( rOpt ) {}

	void					SetDefaults();
	const ScGridOptions&	operator=  ( const ScGridOptions& rCpy );
	int						operator== ( const ScGridOptions& rOpt ) const;
	int						operator!= ( const ScGridOptions& rOpt ) const { return !(operator==(rOpt)); }
};

//==================================================================
// Einstellungen - Ansicht
//==================================================================

class SC_DLLPUBLIC ScViewOptions
{
public:
				ScViewOptions();
				ScViewOptions( const ScViewOptions& rCpy );
				~ScViewOptions();

	void					SetDefaults();

	void					SetOption( ScViewOption eOpt, sal_Bool bNew = sal_True )	{ aOptArr[eOpt] = bNew; }
	sal_Bool					GetOption( ScViewOption eOpt ) const 				{ return aOptArr[eOpt]; }

	void					SetObjMode( ScVObjType eObj, ScVObjMode eMode )	{ aModeArr[eObj] = eMode; }
	ScVObjMode				GetObjMode( ScVObjType eObj ) const				{ return aModeArr[eObj]; }

	void					SetGridColor( const Color& rCol, const String& rName ) { aGridCol = rCol; aGridColName = rName;}
	Color					GetGridColor( String* pStrName = NULL ) const;

	const ScGridOptions&	GetGridOptions() const 						{ return aGridOpt; }
	void					SetGridOptions( const ScGridOptions& rNew ) { aGridOpt = rNew; }
	SvxGridItem*			CreateGridItem( sal_uInt16 nId = SID_ATTR_GRID_OPTIONS ) const;

	const ScViewOptions&	operator=  ( const ScViewOptions& rCpy );
	int						operator== ( const ScViewOptions& rOpt ) const;
	int						operator!= ( const ScViewOptions& rOpt ) const { return !(operator==(rOpt)); }

private:
	sal_Bool			aOptArr		[MAX_OPT];
	ScVObjMode		aModeArr	[MAX_TYPE];
	Color			aGridCol;
	String			aGridColName;
	ScGridOptions	aGridOpt;
};

//==================================================================
// Item fuer Einstellungsdialog - Ansicht
//==================================================================

class SC_DLLPUBLIC ScTpViewItem : public SfxPoolItem
{
public:
				TYPEINFO();
//UNUSED2008-05  ScTpViewItem( sal_uInt16 nWhich );
				ScTpViewItem( sal_uInt16 nWhich, const ScViewOptions& rOpt );
				ScTpViewItem( const ScTpViewItem&  rItem );
				~ScTpViewItem();

	virtual String          GetValueText() const;
	virtual int             operator==( const SfxPoolItem& ) const;
	virtual SfxPoolItem*    Clone( SfxItemPool *pPool = 0 ) const;

	const ScViewOptions&	GetViewOptions() const { return theOptions; }

private:
	ScViewOptions	theOptions;
};


//==================================================================
// CfgItem fuer View-Optionen
//==================================================================

class ScViewCfg : public ScViewOptions
{
	ScLinkConfigItem	aLayoutItem;
	ScLinkConfigItem	aDisplayItem;
	ScLinkConfigItem	aGridItem;

	DECL_LINK( LayoutCommitHdl, void* );
	DECL_LINK( DisplayCommitHdl, void* );
	DECL_LINK( GridCommitHdl, void* );

	com::sun::star::uno::Sequence<rtl::OUString> GetLayoutPropertyNames();
	com::sun::star::uno::Sequence<rtl::OUString> GetDisplayPropertyNames();
	com::sun::star::uno::Sequence<rtl::OUString> GetGridPropertyNames();

public:
			ScViewCfg();

	void			SetOptions( const ScViewOptions& rNew );
};


#endif
