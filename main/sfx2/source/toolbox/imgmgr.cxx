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

#include <stdio.h>
#include <hash_map>

#include "sfx2/imgmgr.hxx"
#include <sfx2/sfx.hrc>
#include <sfx2/app.hxx>
#include "sfx2/sfxresid.hxx"
#include <sfx2/bindings.hxx>
#include "statcach.hxx"
#include <sfx2/module.hxx>
#include <vcl/bitmap.hxx>
#include <vcl/toolbox.hxx>

#include <tools/rcid.h>
#include <tools/link.hxx>
#include <svtools/miscopt.hxx>
#include <vos/mutex.hxx>

#ifndef GCC
#endif

#ifndef _UNOTOOLS_PROCESSFACTORY_HXX
#include <comphelper/processfactory.hxx>
#endif

const sal_uInt32 IMAGELIST_COUNT = 4; // small, small-hi, large, large-hi

struct ToolBoxInf_Impl
{
    ToolBox* pToolBox;
    sal_uInt16   nFlags;
};

class SfxImageManager_Impl
{
public:
    sal_Int16                       m_nSymbolsSize;
    SvtMiscOptions		            m_aOpt;
    std::vector< ToolBoxInf_Impl* > m_aToolBoxes;
    ImageList*                      m_pImageList[IMAGELIST_COUNT];
    SfxModule*                      m_pModule;

    ImageList*              GetImageList( sal_Bool bBig, sal_Bool bHiContrast );
    Image                   GetImage( sal_uInt16 nId, sal_Bool bBig, sal_Bool bHiContrast );
    void                    SetSymbolsSize_Impl( sal_Int16 );
    
    DECL_LINK( OptionsChanged_Impl, void* );
    DECL_LINK( SettingsChanged_Impl, void* );

    
    SfxImageManager_Impl( SfxModule* pModule );
	~SfxImageManager_Impl();
};

typedef std::hash_map< sal_Int64, sal_Int64 > SfxImageManagerMap;

// global image lists
static SfxImageManager_Impl* pGlobalImageManager = 0;
static SfxImageManagerMap    m_ImageManager_ImplMap;
static SfxImageManagerMap    m_ImageManagerMap;
static ImageList*            pImageListSmall=0;
static ImageList*            pImageListBig=0;
static ImageList*            pImageListHiSmall=0;
static ImageList*            pImageListHiBig=0;

static SfxImageManager_Impl* GetImageManager( SfxModule* pModule )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    if ( pModule == 0 )
    {
        if ( !pGlobalImageManager )
            pGlobalImageManager = new SfxImageManager_Impl( 0 );
        return pGlobalImageManager;
    }
    else
    {
        SfxImageManager_Impl* pImpl( 0 );
        SfxImageManagerMap::const_iterator pIter = m_ImageManager_ImplMap.find( sal::static_int_cast< sal_Int64>( reinterpret_cast< sal_IntPtr >( pModule )));
        if ( pIter != m_ImageManager_ImplMap.end() )
            pImpl = reinterpret_cast< SfxImageManager_Impl* >( sal::static_int_cast< sal_IntPtr >( pIter->second ));
        else
        {
            pImpl = new SfxImageManager_Impl( pModule );
            m_ImageManager_ImplMap.insert( 
                SfxImageManagerMap::value_type( 
                    sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( pModule )), 
                    sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( pImpl )) ));
        }
        return pImpl;
    }
}

// Global image list
static ImageList* GetImageList( sal_Bool bBig, sal_Bool bHiContrast )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    
    // Has to be changed if we know how the IDs are named!!!
    ImageList*& rpList = bBig ? ( bHiContrast ? pImageListHiBig : pImageListBig ) :
                                ( bHiContrast ? pImageListHiSmall : pImageListSmall );
    if ( !rpList )
    {
        ResMgr *pResMgr = SfxApplication::GetOrCreate()->GetOffResManager_Impl();

        ResId aResId( bBig ? ( bHiContrast ? RID_DEFAULTIMAGELIST_LCH : RID_DEFAULTIMAGELIST_LC ) :
                             ( bHiContrast ? RID_DEFAULTIMAGELIST_SCH : RID_DEFAULTIMAGELIST_SC ), *pResMgr);

        aResId.SetRT( RSC_IMAGELIST );

        DBG_ASSERT( pResMgr->IsAvailable(aResId), "No default ImageList!" );

        if ( pResMgr->IsAvailable(aResId) )
            rpList = new ImageList( aResId );
        else
            rpList = new ImageList();
	}

    return rpList;
}

static sal_Int16 impl_convertBools( sal_Bool bLarge, sal_Bool bHiContrast )
{
    sal_Int16 nIndex( 0 );
    if ( bLarge  )
        nIndex += 1;
    if ( bHiContrast )
        nIndex += 2;
    return nIndex;
}

//=========================================================================

SfxImageManager_Impl::SfxImageManager_Impl( SfxModule* pModule ) :
    m_nSymbolsSize( SvtMiscOptions().GetCurrentSymbolsSize() ),
    m_pModule( pModule )
{
    for ( sal_uInt32 i = 0; i < IMAGELIST_COUNT; i++ )
        m_pImageList[i] = 0;
    
    m_aOpt.AddListenerLink( LINK( this, SfxImageManager_Impl, OptionsChanged_Impl ) );
    Application::AddEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) );
}

//-------------------------------------------------------------------------

SfxImageManager_Impl::~SfxImageManager_Impl()
{
    m_aOpt.RemoveListenerLink( LINK( this, SfxImageManager_Impl, OptionsChanged_Impl ) );
    Application::RemoveEventListener( LINK( this, SfxImageManager_Impl, SettingsChanged_Impl ) ); 

    for ( sal_uInt32 i = 0; i < m_aToolBoxes.size(); i++ )
        delete m_aToolBoxes[i];
}

//-------------------------------------------------------------------------

ImageList* SfxImageManager_Impl::GetImageList( sal_Bool bBig, sal_Bool bHiContrast )
{
    sal_Int32 nIndex = impl_convertBools( bBig, bHiContrast );
    if ( !m_pImageList[nIndex] )
    {
        if ( !m_pModule )
            m_pImageList[nIndex] = ::GetImageList( bBig, bHiContrast );
        else
            m_pImageList[nIndex] = m_pModule->GetImageList_Impl( bBig, bHiContrast );
    }
    
    return m_pImageList[nIndex];
}

//-------------------------------------------------------------------------

Image SfxImageManager_Impl::GetImage( sal_uInt16 nId, sal_Bool bBig, sal_Bool bHiContrast )
{
	ImageList* pImageList = GetImageList( bBig, bHiContrast );
    if ( pImageList )
	    return pImageList->GetImage( nId );
    return Image();
}

//-------------------------------------------------------------------------

void SfxImageManager_Impl::SetSymbolsSize_Impl( sal_Int16 nNewSymbolsSize )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    
    if ( nNewSymbolsSize != m_nSymbolsSize )
	{
        m_nSymbolsSize = nNewSymbolsSize;
        sal_Bool bLarge( m_nSymbolsSize == SFX_SYMBOLS_SIZE_LARGE );

        for ( sal_uInt32 n=0; n < m_aToolBoxes.size(); n++ )
        {
            ToolBoxInf_Impl *pInf = m_aToolBoxes[n];
            if ( pInf->nFlags & SFX_TOOLBOX_CHANGESYMBOLSET )
            {
                ToolBox *pBox       = pInf->pToolBox;
                sal_Bool    bHiContrast = pBox->GetSettings().GetStyleSettings().GetHighContrastMode();
                sal_uInt16  nCount      = pBox->GetItemCount();
                for ( sal_uInt16 nPos=0; nPos<nCount; nPos++ )
                {
                    sal_uInt16 nId = pBox->GetItemId( nPos );
                    if ( pBox->GetItemType(nPos) == TOOLBOXITEM_BUTTON )
                    {
                        pBox->SetItemImage( nId, GetImage( nId, bLarge, bHiContrast ) );
                        SfxStateCache *pCache = SfxViewFrame::Current()->GetBindings().GetStateCache( nId );
                        if ( pCache )
                            pCache->SetCachedState();
                    }
                }

                if ( !pBox->IsFloatingMode() )
                {
                    Size aActSize( pBox->GetSizePixel() );
                    Size aSize( pBox->CalcWindowSizePixel() );
                    if ( pBox->IsHorizontal() )
                        aSize.Width() = aActSize.Width();
                    else
                        aSize.Height() = aActSize.Height();

                    pBox->SetSizePixel( aSize );
                }                
            }            
        }
    }
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxImageManager_Impl, OptionsChanged_Impl, void*, EMPTYARG )
{
    SetSymbolsSize_Impl( SvtMiscOptions().GetCurrentSymbolsSize() );
    return 0L;
}

//-------------------------------------------------------------------------

IMPL_LINK( SfxImageManager_Impl, SettingsChanged_Impl, void*, EMPTYARG )
{
    // Check if toolbar button size have changed and we have to use system settings
    sal_Int16 nSymbolsSize = SvtMiscOptions().GetCurrentSymbolsSize();
    if ( m_nSymbolsSize != nSymbolsSize )
        SetSymbolsSize_Impl( nSymbolsSize );
    return 0L;
}

//-------------------------------------------------------------------------

//=========================================================================

SfxImageManager::SfxImageManager( SfxModule* pModule )
{
    pImp = ::GetImageManager( pModule );
}

//-------------------------------------------------------------------------

SfxImageManager::~SfxImageManager()
{
}

//-------------------------------------------------------------------------

SfxImageManager* SfxImageManager::GetImageManager( SfxModule* pModule )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    
    SfxImageManagerMap::const_iterator pIter = 
        m_ImageManagerMap.find( sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( pModule )));
    if ( pIter != m_ImageManagerMap.end() )
        return reinterpret_cast< SfxImageManager* >( sal::static_int_cast< sal_IntPtr >( pIter->second ));
    else
    {
        SfxImageManager* pSfxImageManager = new SfxImageManager( pModule );
        m_ImageManagerMap.insert( SfxImageManagerMap::value_type( 
            sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( pModule )), 
            sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( pSfxImageManager )) ));
        return pSfxImageManager;
    }
}

//-------------------------------------------------------------------------

Image SfxImageManager::GetImage( sal_uInt16 nId, sal_Bool bBig, sal_Bool bHiContrast ) const
{
    ImageList* pImageList = pImp->GetImageList( bBig, bHiContrast );
    if ( pImageList && pImageList->HasImageAtPos( nId ) )
        return pImageList->GetImage( nId );
    return Image();
}

//-------------------------------------------------------------------------

Image SfxImageManager::GetImage( sal_uInt16 nId, sal_Bool bHiContrast ) const
{
    sal_Bool bLarge = SvtMiscOptions().AreCurrentSymbolsLarge();
    return GetImage( nId, bLarge, bHiContrast );
}

//-------------------------------------------------------------------------

Image SfxImageManager::SeekImage( sal_uInt16 nId, sal_Bool bBig, sal_Bool bHiContrast ) const
{
    sal_Bool bGlobal = ( pImp->m_pModule == 0 );
    ImageList* pImageList = pImp->GetImageList( bBig, bHiContrast );
    if ( pImageList && pImageList->HasImageAtPos( nId ) )
	    return pImageList->GetImage( nId );
    else if ( !bGlobal )
    {
        pImageList = ::GetImageManager( 0 )->GetImageList( bBig, bHiContrast );
        if ( pImageList )
	        return pImageList->GetImage( nId );
    }
    return Image();
}

//-------------------------------------------------------------------------

Image SfxImageManager::SeekImage( sal_uInt16 nId, sal_Bool bHiContrast ) const
{
    sal_Bool bLarge = SvtMiscOptions().AreCurrentSymbolsLarge();
    return SeekImage( nId, bLarge, bHiContrast );
}

//-------------------------------------------------------------------------

void SfxImageManager::RegisterToolBox( ToolBox *pBox, sal_uInt16 nFlags )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
	
    ToolBoxInf_Impl* pInf = new ToolBoxInf_Impl;
    pInf->pToolBox = pBox;
    pInf->nFlags   = nFlags;
    pImp->m_aToolBoxes.push_back( pInf );
}

//-------------------------------------------------------------------------

void SfxImageManager::ReleaseToolBox( ToolBox *pBox )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    
    for ( sal_uInt32 n=0; n < pImp->m_aToolBoxes.size(); n++ )
    {
        if ((pImp->m_aToolBoxes[n])->pToolBox == pBox )
        {
            delete pImp->m_aToolBoxes[n];
            pImp->m_aToolBoxes.erase( pImp->m_aToolBoxes.begin() + n );
            return;
        }
    }
}

//-------------------------------------------------------------------------

void SfxImageManager::SetImages( ToolBox& rToolBox, sal_Bool bHiContrast, sal_Bool bLarge )
{
    SetImagesForceSize( rToolBox, bLarge, bHiContrast );
}

//-------------------------------------------------------------------------

void SfxImageManager::SetImagesForceSize( ToolBox& rToolBox, sal_Bool bHiContrast, sal_Bool bLarge )
{
    ImageList* pImageList = pImp->GetImageList( bLarge, bHiContrast );
	
    sal_uInt16 nCount = rToolBox.GetItemCount();
    for (sal_uInt16 n=0; n<nCount; n++)
    {
        sal_uInt16 nId = rToolBox.GetItemId(n);
        switch ( rToolBox.GetItemType(n) )
        {
            case TOOLBOXITEM_BUTTON:
            {
                if ( pImageList && pImageList->HasImageAtPos( nId ) )
                    rToolBox.SetItemImage( nId, pImageList->GetImage( nId ));
                else
                    rToolBox.SetItemImage( nId, Image() );
            }

            case TOOLBOXITEM_SEPARATOR:
            case TOOLBOXITEM_SPACE:
            case TOOLBOXITEM_BREAK:
            default:
                break;
        }
    }
}

void SfxImageManager::SetImages( ToolBox& rToolBox )
{
    sal_Bool bLarge = ( pImp->m_nSymbolsSize == SFX_SYMBOLS_SIZE_LARGE );
    sal_Bool bHiContrast = rToolBox.GetSettings().GetStyleSettings().GetHighContrastMode();
    SetImagesForceSize( rToolBox, bHiContrast, bLarge );
}
