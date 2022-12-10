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
#ifndef _SVX_SIDEBAR_LINE_SPACING_CONTROL_HXX_
#define _SVX_SIDEBAR_LINE_SPACING_CONTROL_HXX_
 
#include "svx/sidebar/PopupControl.hxx"
#include "svx/sidebar/ValueSetWithTextControl.hxx"
#include <sfx2/bindings.hxx>
#include <svtools/ctrlbox.hxx>
#include <svtools/ctrltool.hxx>
#include "ParaPropertyPanel.hxx"
#include <vcl/fixed.hxx>
#include <svl/poolitem.hxx>
#include <editeng/lspcitem.hxx>
#include <sfx2/sidebar/EnumContext.hxx>


namespace svx { namespace sidebar {

#define LSP_POS_GLOBAL_VALUE	String("Line_Spacing_Pos", 16, RTL_TEXTENCODING_ASCII_US)
#define LSP_LV_GLOBAL_VALUE	    String("Line_Spacing_Lv", 15, RTL_TEXTENCODING_ASCII_US)
#define BEGIN_VALUE            28
class ParaLineSpacingControl:public svx::sidebar::PopupControl
{
public:
	ParaLineSpacingControl(Window* pParent, svx::sidebar::ParaPropertyPanel& rPanel);
	~ParaLineSpacingControl();
	void Rearrange(SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem,const ::sfx2::sidebar::EnumContext currentContext);
	//virtual void Paint(const Rectangle& rect);
	
	//add 
	short GetLastCustomState();
	long  GetLastCustomValue();
	//add end

	void ExecuteLineSpace();
	void SetLineSpace( SvxLineSpacingItem& rLineSpace,
						int eSpace, long lValue = 0 );

	void ExecuteLineSpacing( sal_uInt16 aIsCustom, sal_uInt16 aEntry );
	void SetAllNoSel();
	void PopupModeEndCallback();

private:	
	bool					mbUseLineSPCustom;
	bool					mbLineSPDisable;
	SfxMapUnit                      m_eLNSpaceUnit;
	ParaPropertyPanel&     mrParaPropertyPanel;
	SfxBindings*		mpBindings;

	long 							nMinFixDist;
	Edit*							pActLineDistFld;

	ValueSetWithTextControl	maLineSpacing;

	FixedText maCustomFT;	
	FixedText maLSpacingFT;
	ListBox					        aLineDist;
	
	FixedText maOfFT;
	MetricField			        	aLineDistAtPercentBox;
	MetricField	        			aLineDistAtMetricBox;

	Image  maSpacing1;
	Image  maSpacing115;
	Image  maSpacing15;
	Image  maSpacing2;

	Image  maSelSpacing1;
	Image  maSelSpacing115;
	Image  maSelSpacing15;
	Image  maSelSpacing2;
	Image  maSelSelHanging;
	Image				maImgCus;
	Image				maImgCusGrey;
	XubString			maStrCus;

	Image*				mpImg;
	Image*				mpImgSel;
	XubString*			mpStr;
	XubString*			mpStrTip;
	String				maLine;
	String				maOf;

	sal_Int64             maValue;
	sal_uInt16                maPos;
private:	
	void initial();
	DECL_LINK(LineSPDistHdl_Impl, ListBox*);
	DECL_LINK(LineSPDistAtHdl_Impl, MetricField*);
	DECL_LINK(VSSelHdl, void*);
};
}}

#endif

