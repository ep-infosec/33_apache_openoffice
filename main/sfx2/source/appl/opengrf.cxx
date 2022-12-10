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
#include "precompiled_sfx2.hxx"

#include <tools/urlobj.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/CommonFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/FilePreviewImageFormats.hpp>
#include <com/sun/star/ui/dialogs/ListboxControlActions.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerListener.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerNotifier.hpp>
#include <com/sun/star/ui/dialogs/XFilePreview.hpp>
#include <com/sun/star/ui/dialogs/XFilterManager.hpp>
#include <svl/urihelper.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <svtools/transfer.hxx>
#include <sot/formats.hxx>
#include <vcl/msgbox.hxx>
#include <sfx2/filedlghelper.hxx>
#include <sfx2/docfile.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/opengrf.hxx>
#include "app.hrc"
#include "sfx2/sfxresid.hxx"

//-----------------------------------------------------------------------------

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::ui::dialogs;
using namespace ::com::sun::star::uno;
using namespace ::rtl;
using namespace ::cppu;


//-----------------------------------------------------------------------------

sal_uInt16	SvxOpenGrfErr2ResId(	short	err		)
{
	switch( err )
	{
		case GRFILTER_OPENERROR:
			return RID_SVXSTR_GRFILTER_OPENERROR;
		case GRFILTER_IOERROR:
			return RID_SVXSTR_GRFILTER_IOERROR;
		case GRFILTER_VERSIONERROR:
			return RID_SVXSTR_GRFILTER_VERSIONERROR;
		case GRFILTER_FILTERERROR:
			return RID_SVXSTR_GRFILTER_FILTERERROR;
		case GRFILTER_FORMATERROR:
		default:
			return RID_SVXSTR_GRFILTER_FORMATERROR;
	}
}


struct SvxOpenGrf_Impl
{
	SvxOpenGrf_Impl			();

	sfx2::FileDialogHelper					aFileDlg;
    uno::Reference < XFilePickerControlAccess >	xCtrlAcc;
};


SvxOpenGrf_Impl::SvxOpenGrf_Impl() :
	aFileDlg(SFXWB_GRAPHIC)
{
	uno::Reference < XFilePicker > xFP = aFileDlg.GetFilePicker();
    xCtrlAcc = uno::Reference < XFilePickerControlAccess >(xFP, UNO_QUERY);
}


SvxOpenGraphicDialog::SvxOpenGraphicDialog( const String& rTitle ) :
	mpImpl( new SvxOpenGrf_Impl )
{
    mpImpl->aFileDlg.SetTitle(rTitle);
}


SvxOpenGraphicDialog::~SvxOpenGraphicDialog()
{
}


short SvxOpenGraphicDialog::Execute()
{
	sal_uInt16 	nImpRet;
	sal_Bool	bQuitLoop(sal_False);

	while( bQuitLoop == sal_False &&
		   mpImpl->aFileDlg.Execute() == ERRCODE_NONE )
	{
		if( GetPath().Len() )
		{
			GraphicFilter*	pFilter = GraphicFilter::GetGraphicFilter();
			INetURLObject aObj( GetPath() );

			// check whether we can load the graphic
			String	aCurFilter( GetCurrentFilter() );
			sal_uInt16 	nFormatNum = pFilter->GetImportFormatNumber( aCurFilter );
			sal_uInt16 	nRetFormat = 0;
			sal_uInt16	nFound = USHRT_MAX;

			// non-local?
			if ( INET_PROT_FILE != aObj.GetProtocol() )
			{
				SfxMedium aMed( aObj.GetMainURL( INetURLObject::NO_DECODE ), STREAM_READ, sal_True );
				aMed.DownLoad();
				SvStream* pStream = aMed.GetInStream();

				if( pStream )
					nImpRet = pFilter->CanImportGraphic( aObj.GetMainURL( INetURLObject::NO_DECODE ), *pStream, nFormatNum, &nRetFormat );
				else
					nImpRet = pFilter->CanImportGraphic( aObj, nFormatNum, &nRetFormat );

				if ( GRFILTER_OK != nImpRet )
				{
					if ( !pStream )
						nImpRet = pFilter->CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
					else
						nImpRet = pFilter->CanImportGraphic( aObj.GetMainURL( INetURLObject::NO_DECODE ), *pStream,
															 GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
				}
			}
			else
			{
				if( (nImpRet=pFilter->CanImportGraphic( aObj, nFormatNum, &nRetFormat )) != GRFILTER_OK )
					nImpRet = pFilter->CanImportGraphic( aObj, GRFILTER_FORMAT_DONTKNOW, &nRetFormat );
			}

			if ( GRFILTER_OK == nImpRet )
				nFound = nRetFormat;

			// could not load?
			if ( nFound == USHRT_MAX )
			{
				WarningBox aWarningBox( NULL, WB_3DLOOK | WB_RETRY_CANCEL, String( SfxResId( SvxOpenGrfErr2ResId(nImpRet) ) ) );
				bQuitLoop = aWarningBox.Execute()==RET_RETRY ? sal_False : sal_True;
			}
			else
			{
				// setup appropriate filter (so next time, it will work)
				if( pFilter->GetImportFormatCount() )
				{
					String	aFormatName(pFilter->GetImportFormatName(nFound));
					SetCurrentFilter(aFormatName);
				}

				return nImpRet;
			}
		}
	}

	// cancel
	return -1;
}


void SvxOpenGraphicDialog::SetPath( const String& rPath )
{
	mpImpl->aFileDlg.SetDisplayDirectory(rPath);
}

void SvxOpenGraphicDialog::SetPath( const String& rPath, sal_Bool bLinkState )
{
	SetPath(rPath);
	AsLink(bLinkState);
}


void SvxOpenGraphicDialog::EnableLink( sal_Bool	 state	)
{
	if( mpImpl->xCtrlAcc.is() )
	{
		try
		{
			mpImpl->xCtrlAcc->enableControl( ExtendedFilePickerElementIds::CHECKBOX_LINK, state );
		}
		catch(IllegalArgumentException)
		{
#ifdef DBG_UTIL
			DBG_ERROR( "Cannot enable \"link\" checkbox" );
#endif
		}
	}
}


void SvxOpenGraphicDialog::AsLink(sal_Bool	bState)
{
	if( mpImpl->xCtrlAcc.is() )
	{
		try
		{
			Any	aAny; aAny <<= bState;
			mpImpl->xCtrlAcc->setValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0, aAny );
		}
		catch(IllegalArgumentException)
		{
#ifdef DBG_UTIL
			DBG_ERROR( "Cannot check \"link\" checkbox" );
#endif
		}
	}
}


sal_Bool SvxOpenGraphicDialog::IsAsLink() const
{
	try
	{
		if( mpImpl->xCtrlAcc.is() )
		{
			Any aVal = mpImpl->xCtrlAcc->getValue( ExtendedFilePickerElementIds::CHECKBOX_LINK, 0 );
			DBG_ASSERT(aVal.hasValue(), "Value CBX_INSERT_AS_LINK not found");
			return aVal.hasValue() ? *(sal_Bool*) aVal.getValue() : sal_False;
		}
	}
	catch(IllegalArgumentException)
	{
#ifdef DBG_UTIL
		DBG_ERROR( "Cannot access \"link\" checkbox" );
#endif
	}

	return sal_False;
}


int SvxOpenGraphicDialog::GetGraphic(Graphic& rGraphic) const
{
	return mpImpl->aFileDlg.GetGraphic(rGraphic);
}


String SvxOpenGraphicDialog::GetPath() const
{
	return mpImpl->aFileDlg.GetPath();
}


String SvxOpenGraphicDialog::GetCurrentFilter() const
{
	return mpImpl->aFileDlg.GetCurrentFilter();
}


void SvxOpenGraphicDialog::SetCurrentFilter(const String&	rStr)
{
	mpImpl->aFileDlg.SetCurrentFilter(rStr);
}

void SvxOpenGraphicDialog::SetControlHelpIds( const sal_Int16* _pControlId, const char** _pHelpId )
{
	mpImpl->aFileDlg.SetControlHelpIds( _pControlId, _pHelpId );
}


