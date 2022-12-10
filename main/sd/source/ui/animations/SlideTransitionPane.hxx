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


#ifndef SD_SLIDETRANSITIONPANE_HXX
#define SD_SLIDETRANSITIONPANE_HXX

#include "EventMultiplexer.hxx"

#include "SlideSorterViewShell.hxx"

#include <vcl/ctrl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/field.hxx>
#include <com/sun/star/drawing/XDrawView.hpp>
#include <com/sun/star/frame/XModel.hpp>

#include <vector>
#include <map>

class SdDrawDocument;
class SdPage;

namespace sd
{

class ViewShellBase;

namespace impl
{
    struct TransitionEffect;
}

class SlideTransitionPane : public Control
{
public:
    explicit SlideTransitionPane(
        ::Window * pParent,
        ViewShellBase & rBase,
        const Size& rMinSize,
        SdDrawDocument* pDoc );
	virtual ~SlideTransitionPane();

    // Window
    virtual void Resize();
    virtual void DataChanged (const DataChangedEvent& rEvent);

	void onSelectionChanged();
	void onChangeCurrentPage();

private:
    void updateLayout();
	void updateControls();
    void updateControlState();

    void updateSoundList();
    void openSoundFileDialog();

    impl::TransitionEffect getTransitionEffectFromControls() const;

    void applyToSelectedPages();
    void playCurrentEffect();

	void addListener();
	void removeListener();

    ::sd::slidesorter::SharedPageSelection getSelectedPages (void) const;

    void UpdateLook (void);

    DECL_LINK( ApplyToAllButtonClicked, void * );
    DECL_LINK( PlayButtonClicked, void * );
    DECL_LINK( SlideShowButtonClicked, void * );
	DECL_LINK( AutoPreviewClicked, void * );

    DECL_LINK( TransitionSelected, void * );
    DECL_LINK( AdvanceSlideRadioButtonToggled, void * );
    DECL_LINK( AdvanceTimeModified, void * );
    DECL_LINK( SpeedListBoxSelected, void * );
    DECL_LINK( SoundListBoxSelected, void * );
    DECL_LINK( LoopSoundBoxChecked, void * );
    DECL_LINK(EventMultiplexerListener, tools::EventMultiplexerEvent*);
    DECL_LINK(LateInitCallback, Timer*);

    ViewShellBase &   mrBase;
    SdDrawDocument *  mpDrawDoc;
    Size              maMinSize;

    FixedLine    maFL_APPLY_TRANSITION;
    ListBox      maLB_SLIDE_TRANSITIONS;
    FixedLine    maFL_MODIFY_TRANSITION;
    FixedText    maFT_SPEED;
    ListBox      maLB_SPEED;
    FixedText    maFT_SOUND;
    ListBox      maLB_SOUND;
    CheckBox     maCB_LOOP_SOUND;
    FixedLine    maFL_ADVANCE_SLIDE;
    RadioButton  maRB_ADVANCE_ON_MOUSE;
    RadioButton  maRB_ADVANCE_AUTO;
    MetricField  maMF_ADVANCE_AUTO_AFTER;
    FixedLine    maFL_EMPTY1;
    PushButton   maPB_APPLY_TO_ALL;
    PushButton   maPB_PLAY;
    PushButton   maPB_SLIDE_SHOW;
    FixedLine    maFL_EMPTY2;
    CheckBox     maCB_AUTO_PREVIEW;

    String       maSTR_NO_TRANSITION;

	::com::sun::star::uno::Reference< ::com::sun::star::drawing::XDrawView >             mxView;
	::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel >                  mxModel;

    bool         mbHasSelection;
    bool         mbUpdatingControls;
    bool         mbIsMainViewChangePending;

    typedef ::std::vector< String > tSoundListType;
    tSoundListType  maSoundList;
    mutable String  maCurrentSoundFile;

    typedef ::std::map< sal_uInt16, sal_uInt16 > tPresetIndexesType;
    tPresetIndexesType m_aPresetIndexes;

    Timer maLateInitTimer;
};

} //  namespace sd

// SD_SLIDETRANSITIONPANE_HXX
#endif
