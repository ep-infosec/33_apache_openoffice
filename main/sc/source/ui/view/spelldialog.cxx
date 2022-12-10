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
#include "precompiled_sc.hxx"
#include "spelldialog.hxx"

#include <sfx2/app.hxx>
#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <svx/svxids.hrc>
#include <editeng/editstat.hxx>
#include <editeng/editview.hxx>
#include <editeng/unolingu.hxx>
#include "selectionstate.hxx"

#include "spelleng.hxx"
#include "tabvwsh.hxx"
#include "docsh.hxx"
#include "scmod.hxx"
#include "editable.hxx"
#include "undoblk.hxx"

// ============================================================================

SFX_IMPL_CHILDWINDOW( ScSpellDialogChildWindow, SID_SPELL_DIALOG )

ScSpellDialogChildWindow::ScSpellDialogChildWindow( Window* pParentP, sal_uInt16 nId,
        SfxBindings* pBindings, SfxChildWinInfo* pInfo ) :
    ::svx::SpellDialogChildWindow( pParentP, nId, pBindings, pInfo ),
    mpViewShell( 0 ),
    mpViewData( 0 ),
    mpDocShell( 0 ),
    mpDoc( 0 ),
    mbNeedNextObj( false ),
    mbOldIdleDisabled( false )
{
    Init();
}

ScSpellDialogChildWindow::~ScSpellDialogChildWindow()
{
    Reset();
}

SfxChildWinInfo ScSpellDialogChildWindow::GetInfo() const
{
	return ::svx::SpellDialogChildWindow::GetInfo();
}

void ScSpellDialogChildWindow::InvalidateSpellDialog()
{
    ::svx::SpellDialogChildWindow::InvalidateSpellDialog();
}

// protected ------------------------------------------------------------------

::svx::SpellPortions ScSpellDialogChildWindow::GetNextWrongSentence( bool /*bRecheck*/ )
{
    ::svx::SpellPortions aPortions;
    if( mxEngine.get() && mpViewData )
    {
        if( EditView* pEditView = mpViewData->GetSpellingView() )
        {
            // edit engine handles cell iteration internally
            do
            {
                if( mbNeedNextObj )
                    mxEngine->SpellNextDocument();
                mbNeedNextObj = !mxEngine->IsFinished() && !mxEngine->SpellSentence( *pEditView, aPortions, false );
            }
            while( mbNeedNextObj );
        }

        // finished? - close the spelling dialog
        if( mxEngine->IsFinished() )
            GetBindings().GetDispatcher()->Execute( SID_SPELL_DIALOG, SFX_CALLMODE_ASYNCHRON );
    }
    return aPortions;
}

void ScSpellDialogChildWindow::ApplyChangedSentence( const ::svx::SpellPortions& rChanged, bool bRecheck )
{
    if( mxEngine.get() && mpViewData )
        if( EditView* pEditView = mpViewData->GetSpellingView() )
            mxEngine->ApplyChangedSentence( *pEditView, rChanged, bRecheck );
}

void ScSpellDialogChildWindow::GetFocus()
{
    if( IsSelectionChanged() )
    {
        Reset();
        InvalidateSpellDialog();
        Init();
    }
}

void ScSpellDialogChildWindow::LoseFocus()
{
}

// private --------------------------------------------------------------------

void ScSpellDialogChildWindow::Reset()
{
    if( mpViewShell && (mpViewShell == PTR_CAST( ScTabViewShell, SfxViewShell::Current() )) )
    {
        if( mxEngine.get() && mxEngine->IsAnyModified() )
        {
            const ScAddress& rCursor = mxOldSel->GetCellCursor();
            SCTAB nTab = rCursor.Tab();
            SCCOL nOldCol = rCursor.Col();
            SCROW nOldRow = rCursor.Row();
            SCCOL nNewCol = mpViewData->GetCurX();
            SCROW nNewRow = mpViewData->GetCurY();
            mpDocShell->GetUndoManager()->AddUndoAction( new ScUndoConversion(
                mpDocShell, mpViewData->GetMarkData(),
                nOldCol, nOldRow, nTab, mxUndoDoc.release(),
                nNewCol, nNewRow, nTab, mxRedoDoc.release(),
                ScConversionParam( SC_CONVERSION_SPELLCHECK ) ) );
            mpDoc->SetDirty();
            mpDocShell->SetDocumentModified();
        }

        mpViewData->SetSpellingView( 0 );
        mpViewShell->KillEditView( sal_True );
        mpDocShell->PostPaintGridAll();
        mpViewShell->UpdateInputHandler();
        mpDoc->DisableIdle( mbOldIdleDisabled );
    }
    mxEngine.reset();
    mxUndoDoc.reset();
    mxRedoDoc.reset();
    mxOldSel.reset();
    mpViewShell = 0;
    mpViewData = 0;
    mpDocShell = 0;
    mpDoc = 0;
    mbNeedNextObj = false;
    mbOldIdleDisabled = false;
}

void ScSpellDialogChildWindow::Init()
{
    if( mpViewShell )
        return;
    if( (mpViewShell = PTR_CAST( ScTabViewShell, SfxViewShell::Current() )) == 0 )
        return;

    mpViewData = mpViewShell->GetViewData();

    // exit edit mode - TODO support spelling in edit mode
    if( mpViewData->HasEditView( mpViewData->GetActivePart() ) )
        SC_MOD()->InputEnterHandler();

    mxOldSel.reset( new ScSelectionState( *mpViewData ) );

    mpDocShell = mpViewData->GetDocShell();
    mpDoc = mpDocShell->GetDocument();

    const ScAddress& rCursor = mxOldSel->GetCellCursor();
    SCCOL nCol = rCursor.Col();
    SCROW nRow = rCursor.Row();
    SCTAB nTab = rCursor.Tab();

    ScMarkData& rMarkData = mpViewData->GetMarkData();
    rMarkData.MarkToMulti();

    switch( mxOldSel->GetSelectionType() )
    {
        case SC_SELECTTYPE_NONE:
        case SC_SELECTTYPE_SHEET:
        {
            // test if there is something editable
            ScEditableTester aTester( mpDoc, rMarkData );
            if( !aTester.IsEditable() )
            {
                // #i85751# Don't show a ErrorMessage here, because the vcl
                // parent of the InfoBox is not fully initialized yet.
                // This leads to problems in the modality behaviour of the
                // ScSpellDialogChildWindow.

                //mpViewShell->ErrorMessage( aTester.GetMessageId() );
                return;
            }
        }
        break;

        // edit mode exited, see TODO above
//        case SC_SELECTTYPE_EDITCELL:
//        break;

        default:
            DBG_ERRORFILE( "ScSpellDialogChildWindow::Init - unknown selection type" );
    }

    mbOldIdleDisabled = mpDoc->IsIdleDisabled();
    mpDoc->DisableIdle( sal_True );   // #42726# stop online spelling

    // *** create Undo/Redo documents *** -------------------------------------

    mxUndoDoc.reset( new ScDocument( SCDOCMODE_UNDO ) );
    mxUndoDoc->InitUndo( mpDoc, nTab, nTab );
    mxRedoDoc.reset( new ScDocument( SCDOCMODE_UNDO ) );
    mxRedoDoc->InitUndo( mpDoc, nTab, nTab );

    if ( rMarkData.GetSelectCount() > 1 )
    {
        SCTAB nTabCount = mpDoc->GetTableCount();
        for( SCTAB nOtherTab = 0; nOtherTab < nTabCount; ++nOtherTab )
        {
            if( rMarkData.GetTableSelect( nOtherTab ) && (nOtherTab != nTab) )
            {
                mxUndoDoc->AddUndoTab( nOtherTab, nOtherTab );
                mxRedoDoc->AddUndoTab( nOtherTab, nOtherTab );
            }
        }
    }

    // *** create and init the edit engine *** --------------------------------

    mxEngine.reset( new ScSpellingEngine(
        mpDoc->GetEnginePool(), *mpViewData, mxUndoDoc.get(), mxRedoDoc.get(), LinguMgr::GetSpellChecker() ) );
    mxEngine->SetRefDevice( mpViewData->GetActiveWin() );

    mpViewShell->MakeEditView( mxEngine.get(), nCol, nRow );
    EditView* pEditView = mpViewData->GetEditView( mpViewData->GetActivePart() );
    mpViewData->SetSpellingView( pEditView );
    Rectangle aRect( Point( 0, 0 ), Point( 0, 0 ) );
    pEditView->SetOutputArea( aRect );
    mxEngine->SetControlWord( EE_CNTRL_USECHARATTRIBS );
    mxEngine->EnableUndo( sal_False );
    mxEngine->SetPaperSize( aRect.GetSize() );
    mxEngine->SetText( EMPTY_STRING );
    mxEngine->ClearModifyFlag();

    mbNeedNextObj = true;
}

bool ScSpellDialogChildWindow::IsSelectionChanged()
{
    if( !mxOldSel.get() || !mpViewShell || (mpViewShell != PTR_CAST( ScTabViewShell, SfxViewShell::Current() )) )
        return true;

    if( EditView* pEditView = mpViewData->GetSpellingView() )
        if( pEditView->GetEditEngine() != mxEngine.get() )
            return true;

    ScSelectionState aNewSel( *mpViewData );
    return mxOldSel->GetSheetSelection() != aNewSel.GetSheetSelection();
}

// ============================================================================

