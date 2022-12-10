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



#ifndef _SD_PUBDLG_HXX
#define _SD_PUBDLG_HXX

#include <com/sun/star/beans/PropertyValue.hpp>
#include <vcl/fixed.hxx>
#ifndef _GROUP_HXX //autogen
#include <vcl/group.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include "resltn.hxx"       // enum PublishingResolution
#include "pres.hxx"
#include "assclass.hxx"

#include <boost/scoped_ptr.hpp>

#define NOOFPAGES 6

enum HtmlPublishMode { PUBLISH_HTML, PUBLISH_FRAMES, PUBLISH_WEBCAST, PUBLISH_KIOSK };											

class SfxItemSet;

class FixedText;
class RadioButton;
class ListBox;
class ComboBox;
class Edit;
class MultiLineEdit;
class ValueSet;
class SdHtmlAttrPreview;
class List;
class SdPublishingDesign;
class ButtonSet;

// *********************************************************************
// Html-Export Autopilot
// *********************************************************************

class SdPublishingDlg : public ModalDialog
{
private:
	// page 1 controls
	FixedBitmap*	pPage1_Bmp;
	FixedLine*		pPage1_Titel;
	RadioButton*	pPage1_NewDesign;
	RadioButton*	pPage1_OldDesign;
	ListBox*		pPage1_Designs;
	PushButton*		pPage1_DelDesign;
	FixedText*		pPage1_Desc;

	// page 2 controls
	FixedBitmap*	pPage2_Bmp;
	FixedLine*		pPage2_Titel;
	RadioButton*	pPage2_Standard;
	RadioButton*	pPage2_Frames;
	RadioButton*	pPage2_Kiosk;
	RadioButton*	pPage2_WebCast;
	FixedBitmap*	pPage2_Standard_FB;
	FixedBitmap*	pPage2_Frames_FB;
	FixedBitmap*	pPage2_Kiosk_FB;
	FixedBitmap*	pPage2_WebCast_FB;

	FixedLine*		pPage2_Titel_Html;
	CheckBox*		pPage2_Content;
	CheckBox*		pPage2_Notes;

	FixedLine*		pPage2_Titel_WebCast;
	RadioButton*	pPage2_ASP;
	RadioButton*	pPage2_PERL;
	FixedText*		pPage2_URL_txt;
	Edit*			pPage2_URL;
	FixedText*		pPage2_CGI_txt;
	Edit*			pPage2_CGI;
	FixedText*		pPage2_Index_txt;
	Edit*			pPage2_Index;
    FixedLine*      pPage2_Vert;
	FixedLine*		pPage2_Titel_Kiosk;
	RadioButton*	pPage2_ChgDefault;
	RadioButton*	pPage2_ChgAuto;
	FixedText*		pPage2_Duration_txt;
	TimeField*		pPage2_Duration;
	CheckBox*		pPage2_Endless;

	// page 3 controls
	FixedBitmap*	pPage3_Bmp;
	FixedLine*		pPage3_Titel1;
	RadioButton*	pPage3_Png;
	RadioButton*	pPage3_Gif;
	RadioButton*	pPage3_Jpg;
	FixedText*		pPage3_Quality_txt;
	ComboBox*		pPage3_Quality;
    FixedLine*      pPage3_Vert;
	FixedLine*		pPage3_Titel2;
	RadioButton*	pPage3_Resolution_1;
	RadioButton*	pPage3_Resolution_2;
	RadioButton*	pPage3_Resolution_3;
	FixedLine*		pPage3_Titel3;
	CheckBox*		pPage3_SldSound;
	CheckBox*		pPage3_HiddenSlides;
	// page 4 controls

	FixedBitmap*	pPage4_Bmp;
	FixedLine*		pPage4_Titel1;
	FixedText*		pPage4_Author_txt;
	Edit*			pPage4_Author;
	FixedText*		pPage4_Email_txt;
	Edit*			pPage4_Email;
	FixedText*		pPage4_WWW_txt;
	Edit*			pPage4_WWW;
	FixedText*		pPage4_Titel2;
	MultiLineEdit*	pPage4_Misc;
	CheckBox*		pPage4_Download;
//-/	CheckBox*		pPage4_Created;

	// page 5 controls

	FixedBitmap*	pPage5_Bmp;
	FixedLine*		pPage5_Titel;
	CheckBox*		pPage5_TextOnly;
	ValueSet*		pPage5_Buttons;

	// page 6 controls

	FixedBitmap*	pPage6_Bmp;
	FixedLine*		pPage6_Titel;
	RadioButton*	pPage6_Default;
	RadioButton*	pPage6_User;
	PushButton*		pPage6_Back;
	PushButton*		pPage6_Text;
	PushButton*		pPage6_Link;
	PushButton*		pPage6_VLink;
	PushButton*		pPage6_ALink;
	RadioButton*	pPage6_DocColors;
	SdHtmlAttrPreview*	pPage6_Preview;

	boost::scoped_ptr< ButtonSet > mpButtonSet;

//	CheckBox*		pPage6_Sound;

	// standard controls
	FixedLine		aBottomLine;
	HelpButton		aHelpButton;
	CancelButton	aCancelButton;
	PushButton		aLastPageButton;
	PushButton		aNextPageButton;
	OKButton		aFinishButton;

	Assistent		aAssistentFunc;

	sal_Bool			m_bImpress;
	sal_Bool			m_bButtonsDirty;

	void SetDefaults();
	void CreatePages();
	void RemovePages();

	Color m_aBackColor, m_aTextColor, m_aLinkColor;
	Color m_aVLinkColor, m_aALinkColor;

	void	ChangePage();
	void	UpdatePage();

	List*	m_pDesignList;
	sal_Bool	m_bDesignListDirty;
	SdPublishingDesign* m_pDesign;
	sal_Bool	Load();
	sal_Bool	Save();

	void	GetDesign( SdPublishingDesign* pDesign );
	void	SetDesign( SdPublishingDesign* pDesign );

	void	LoadPreviewButtons();

	DECL_LINK( FinishHdl, OKButton * );
	DECL_LINK( NextPageHdl, PushButton * );
	DECL_LINK( LastPageHdl, PushButton * );

	DECL_LINK( DesignHdl, RadioButton * );
	DECL_LINK( DesignSelectHdl, ListBox * );
	DECL_LINK( DesignDeleteHdl, PushButton * );
	DECL_LINK( BaseHdl, RadioButton * );
	DECL_LINK( ContentHdl, RadioButton * );
	DECL_LINK( GfxFormatHdl, RadioButton * );
	DECL_LINK( ResolutionHdl, RadioButton * );
	DECL_LINK( ButtonsHdl, ValueSet* );
	DECL_LINK( ColorHdl, PushButton * );
	DECL_LINK( WebServerHdl, RadioButton * );
	DECL_LINK( SlideChgHdl, RadioButton* );

public:

	SdPublishingDlg(Window* pWindow, DocumentType eDocType);
	~SdPublishingDlg();

	void GetParameterSequence( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rParams );
};

#endif // _SD_PUBDLG_HXX
