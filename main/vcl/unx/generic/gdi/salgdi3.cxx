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
#include "precompiled_vcl.hxx"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "sal/alloca.h"
#include "sal/types.h"

#include "rtl/tencinfo.h"

#include "osl/file.hxx"

#include "tools/string.hxx"
#include "tools/debug.hxx"
#include "tools/stream.hxx"

#include "basegfx/polygon/b2dpolypolygon.hxx"

#include "i18npool/mslangid.hxx"

#include <vcl/sysdata.hxx>
#include "printergfx.hxx"
#include "vcl/fontmanager.hxx"
#include "vcl/jobdata.hxx"
#include "vcl/printerinfomanager.hxx"
#include "vcl/svapp.hxx"

#include "unx/salunx.h"
#include "unx/saldata.hxx"
#include "unx/saldisp.hxx"
#include "unx/salgdi.h"
#include "unx/pspgraphics.h"
#include "unx/salvd.h"

#include "salcvt.hxx"
#include "gcach_xpeer.hxx"
#include "xrender_peer.hxx"
#include "impfont.hxx"
#include "salframe.hxx"
#include "outdev.h"


#include <hash_set>

#ifdef ENABLE_GRAPHITE
#include <graphite_layout.hxx>
#include <graphite_serverfont.hxx>
#endif

struct cairo_surface_t;
struct cairo_t;
struct cairo_font_face_t;
typedef void* FT_Face;
struct cairo_matrix_t {
    double xx; double yx;
    double xy; double yy;
    double x0; double y0;
};
struct cairo_glyph_t
{
    unsigned long index;
    double x;
    double y;
};
struct BOX
{
    short x1, x2, y1, y2;
};
struct _XRegion
{
    long size;
    long numRects;
    BOX *rects;
    BOX extents;
};
using namespace rtl;

// ===========================================================================

// PspKernInfo allows on-demand-querying of psprint provided kerning info (#i29881#)
class PspKernInfo : public ExtraKernInfo
{
public:
    PspKernInfo( int nFontId ) : ExtraKernInfo(nFontId) {}
protected:
    virtual void Initialize() const;
};

//--------------------------------------------------------------------------

void PspKernInfo::Initialize() const
{
    mbInitialized = true;

    // get the kerning pairs from psprint
    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    typedef std::list< psp::KernPair > PspKernPairs;
    const PspKernPairs& rKernPairs = rMgr.getKernPairs( mnFontId );
    if( rKernPairs.empty() )
        return;

    // feed psprint's kerning list into a lookup-friendly container
    maUnicodeKernPairs.rehash( rKernPairs.size() );
    PspKernPairs::const_iterator it = rKernPairs.begin();
    for(; it != rKernPairs.end(); ++it )
    {
        ImplKernPairData aKernPair = { it->first, it->second, it->kern_x };
        maUnicodeKernPairs.insert( aKernPair );
    }
}

// ----------------------------------------------------------------------------
//
// X11SalGraphics
//
// ----------------------------------------------------------------------------

GC
X11SalGraphics::GetFontGC()
{
	Display *pDisplay = GetXDisplay();

	if( !pFontGC_ )
	{
		XGCValues values;
		values.subwindow_mode		= ClipByChildren;
		values.fill_rule			= EvenOddRule;		// Pict import/ Gradient
		values.graphics_exposures	= False;
		values.foreground			= nTextPixel_;
        pFontGC_ = XCreateGC( pDisplay, hDrawable_,
                              GCSubwindowMode | GCFillRule
                              | GCGraphicsExposures | GCForeground,
                              &values );
	}
	if( !bFontGC_ )
	{
		XSetForeground( pDisplay, pFontGC_, nTextPixel_ );
		SetClipRegion( pFontGC_ );
		bFontGC_ = sal_True;
	}

	return pFontGC_;
}

//--------------------------------------------------------------------------

bool X11SalGraphics::setFont( const ImplFontSelectData *pEntry, int nFallbackLevel )
{
#ifdef HDU_DEBUG
    ByteString aReqName( "NULL" );
    if( pEntry )
        aReqName = ByteString( pEntry->maName, RTL_TEXTENCODING_UTF8 );
    ByteString aUseName( "NULL" );
    if( pEntry && pEntry->mpFontData )
        aUseName = ByteString( pEntry->mpFontData->GetFamilyName(), RTL_TEXTENCODING_UTF8 );
    fprintf( stderr, "SetFont(lvl=%d,\"%s\", %d*%d, naa=%d,b=%d,i=%d) => \"%s\"\n",
        nFallbackLevel, aReqName.GetBuffer(),
	!pEntry?-1:pEntry->mnWidth, !pEntry?-1:pEntry->mnHeight,
        !pEntry?-1:pEntry->mbNonAntialiased,
	!pEntry?-1:pEntry->meWeight, !pEntry?-1:pEntry->meItalic,
        aUseName.GetBuffer() );
#endif

    // release all no longer needed font resources
    for( int i = nFallbackLevel; i < MAX_FALLBACK; ++i )
    {
        if( mpServerFont[i] != NULL )
        {
            // old server side font is no longer referenced
            GlyphCache::GetInstance().UncacheFont( *mpServerFont[i] );
            mpServerFont[i] = NULL;
        }
    }

    // return early if there is no new font
    if( !pEntry )
	return false;

    bFontVertical_ = pEntry->mbVertical;

    // return early if this is not a valid font for this graphics
    if( !pEntry->mpFontData )
        return false;

    // handle the request for a non-native X11-font => use the GlyphCache
    ServerFont* pServerFont = GlyphCache::GetInstance().CacheFont( *pEntry );
    if( pServerFont != NULL )
    {
        // ignore fonts with e.g. corrupted font files
        if( !pServerFont->TestFont() )
        {
            GlyphCache::GetInstance().UncacheFont( *pServerFont );
            return false;
        }

        // register to use the font
        mpServerFont[ nFallbackLevel ] = pServerFont;

        // apply font specific-hint settings if needed 
        // TODO: also disable it for reference devices
	if( !bPrinter_ )
	{
	    ImplServerFontEntry* pSFE = static_cast<ImplServerFontEntry*>( pEntry->mpFontEntry );
	    pSFE->HandleFontOptions();
        }

        return true;
    }

    return false;
}

void ImplServerFontEntry::HandleFontOptions( void )
{
	bool GetFCFontOptions( const ImplFontAttributes&, int nSize, ImplFontOptions& );

	if( !mpServerFont )
		return;
	if( !mbGotFontOptions )
	{
		// get and cache the font options
		mbGotFontOptions = true;
		mbValidFontOptions = GetFCFontOptions( *maFontSelData.mpFontData,
			maFontSelData.mnHeight, maFontOptions );
	}
	// apply the font options
	if( mbValidFontOptions )
		mpServerFont->SetFontOptions( maFontOptions );
}

//--------------------------------------------------------------------------

namespace {

class CairoWrapper
{
private:
    oslModule mpCairoLib;
    
    cairo_surface_t* (*mp_xlib_surface_create_with_xrender_format)(Display *, Drawable , Screen *, XRenderPictFormat *, int , int );
    void (*mp_surface_destroy)(cairo_surface_t *);
    cairo_t* (*mp_create)(cairo_surface_t *);
    void (*mp_destroy)(cairo_t*);
    void (*mp_clip)(cairo_t*);
    void (*mp_rectangle)(cairo_t*, double, double, double, double);
    cairo_font_face_t * (*mp_ft_font_face_create_for_ft_face)(FT_Face, int);
    void (*mp_set_font_face)(cairo_t *, cairo_font_face_t *);
    void (*mp_font_face_destroy)(cairo_font_face_t *);
    void (*mp_matrix_init_identity)(cairo_matrix_t *);
    void (*mp_matrix_scale)(cairo_matrix_t *, double, double);
    void (*mp_matrix_rotate)(cairo_matrix_t *, double);
    void (*mp_set_font_matrix)(cairo_t *, const cairo_matrix_t *);
    void (*mp_show_glyphs)(cairo_t *, const cairo_glyph_t *, int );
    void (*mp_set_source_rgb)(cairo_t *, double , double , double );
    void (*mp_set_font_options)(cairo_t *, const void *);
    void (*mp_ft_font_options_substitute)(const void*, void*);

    bool canEmbolden() const { return false; }

    CairoWrapper();
public:
    static CairoWrapper& get();
    bool isValid() const { return (mpCairoLib != NULL); }
    bool isCairoRenderable(const ServerFont& rFont);

    cairo_surface_t* xlib_surface_create_with_xrender_format(Display *pDisplay, Drawable drawable, Screen *pScreen, XRenderPictFormat *pFormat, int width, int height)
        { return (*mp_xlib_surface_create_with_xrender_format)(pDisplay, drawable, pScreen, pFormat, width, height); }
    void surface_destroy(cairo_surface_t *surface) { (*mp_surface_destroy)(surface); }
    cairo_t* create(cairo_surface_t *surface) { return (*mp_create)(surface); }
    void destroy(cairo_t *cr) { (*mp_destroy)(cr); }
    void clip(cairo_t *cr) { (*mp_clip)(cr); }
    void rectangle(cairo_t *cr, double x, double y, double width, double height) 
        { (*mp_rectangle)(cr, x, y, width, height); }
    cairo_font_face_t* ft_font_face_create_for_ft_face(FT_Face face, int load_flags)
        { return (*mp_ft_font_face_create_for_ft_face)(face, load_flags); }
    void set_font_face(cairo_t *cr, cairo_font_face_t *font_face)
        { (*mp_set_font_face)(cr, font_face); }
    void font_face_destroy(cairo_font_face_t *font_face)
        { (*mp_font_face_destroy)(font_face); }
    void matrix_init_identity(cairo_matrix_t *matrix)
        { (*mp_matrix_init_identity)(matrix); }
    void matrix_scale(cairo_matrix_t *matrix, double sx, double sy)
        { (*mp_matrix_scale)(matrix, sx, sy); }
    void matrix_rotate(cairo_matrix_t *matrix, double radians)
        { (*mp_matrix_rotate)(matrix, radians); }
    void set_font_matrix(cairo_t *cr, const cairo_matrix_t *matrix)
        { (*mp_set_font_matrix)(cr, matrix); }
    void show_glyphs(cairo_t *cr, const cairo_glyph_t *glyphs, int no_glyphs)
        { (*mp_show_glyphs)(cr, glyphs, no_glyphs); }
    void set_source_rgb(cairo_t *cr, double red, double green, double blue)
        { (*mp_set_source_rgb)(cr, red, green, blue); }
    void set_font_options(cairo_t *cr, const void *options)
        { (*mp_set_font_options)(cr, options); }
    void ft_font_options_substitute(const void *options, void *pattern)
        { (*mp_ft_font_options_substitute)(options, pattern); }
};

static CairoWrapper* pCairoInstance = NULL;

CairoWrapper& CairoWrapper::get()
{
    if( ! pCairoInstance )
        pCairoInstance = new CairoWrapper();
    return *pCairoInstance;
}

CairoWrapper::CairoWrapper()
:   mpCairoLib( NULL )
{
    static const char* pDisableCairoText = getenv( "SAL_DISABLE_CAIROTEXT" );
    if( pDisableCairoText && (pDisableCairoText[0] != '0') )
        return;
    
    int nDummy;
    if( !XQueryExtension( GetX11SalData()->GetDisplay()->GetDisplay(), "RENDER", &nDummy, &nDummy, &nDummy ) )
        return;

    mpCairoLib = osl_loadAsciiModule( "libcairo.so.2", SAL_LOADMODULE_DEFAULT );
    if( !mpCairoLib )
        return;

#ifdef DEBUG
    // check cairo version
    int (*p_version)();
    p_version = (int(*)()) osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_version" );
	const int nVersion = p_version ? (*p_version)() : 0;
    fprintf( stderr, "CAIRO version=%d\n", nVersion );
#endif

    mp_xlib_surface_create_with_xrender_format = (cairo_surface_t* (*)(Display *, Drawable , Screen *, XRenderPictFormat *, int , int )) 
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_xlib_surface_create_with_xrender_format" );
    mp_surface_destroy = (void(*)(cairo_surface_t*)) 
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_surface_destroy" );
    mp_create = (cairo_t*(*)(cairo_surface_t*)) 
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_create" );
    mp_destroy = (void(*)(cairo_t*))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_destroy" );
    mp_clip = (void(*)(cairo_t*))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_clip" );
    mp_rectangle = (void(*)(cairo_t*, double, double, double, double))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_rectangle" );
    mp_ft_font_face_create_for_ft_face = (cairo_font_face_t * (*)(FT_Face, int))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_ft_font_face_create_for_ft_face" );
    mp_set_font_face = (void (*)(cairo_t *, cairo_font_face_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_font_face" );
    mp_font_face_destroy = (void (*)(cairo_font_face_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_font_face_destroy" );
    mp_matrix_init_identity = (void (*)(cairo_matrix_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_init_identity" );
    mp_matrix_scale = (void (*)(cairo_matrix_t *, double, double))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_scale" );
    mp_matrix_rotate = (void (*)(cairo_matrix_t *, double))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_matrix_rotate" );
    mp_set_font_matrix = (void (*)(cairo_t *, const cairo_matrix_t *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_font_matrix" );
    mp_show_glyphs = (void (*)(cairo_t *, const cairo_glyph_t *, int ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_show_glyphs" );
    mp_set_source_rgb = (void (*)(cairo_t *, double , double , double ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_source_rgb" );
    mp_set_font_options = (void (*)(cairo_t *, const void *options ))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_set_font_options" );
    mp_ft_font_options_substitute = (void (*)(const void *, void *))
        osl_getAsciiFunctionSymbol( mpCairoLib, "cairo_ft_font_options_substitute" );

    if( !( 
            mp_xlib_surface_create_with_xrender_format &&
            mp_surface_destroy &&
            mp_create &&
            mp_destroy &&
            mp_clip &&
            mp_rectangle &&
            mp_ft_font_face_create_for_ft_face &&
            mp_set_font_face &&
            mp_font_face_destroy &&
            mp_matrix_init_identity &&
            mp_matrix_scale &&
            mp_matrix_rotate &&
            mp_set_font_matrix &&
            mp_show_glyphs &&
            mp_set_source_rgb &&
            mp_set_font_options &&
            mp_ft_font_options_substitute
        ) )
    {
        osl_unloadModule( mpCairoLib );
	mpCairoLib = NULL;
#if OSL_DEBUG_LEVEL > 1
        fprintf( stderr, "not all needed symbols were found\n" );
#endif
    }
}

bool CairoWrapper::isCairoRenderable(const ServerFont& rFont)
{
    return rFont.GetFtFace() && isValid() && rFont.GetAntialiasAdvice() &&
        (rFont.NeedsArtificialBold() ? canEmbolden() : true);
}

} //namespace

CairoFontsCache::LRUFonts CairoFontsCache::maLRUFonts;
int CairoFontsCache::mnRefCount = 0;

CairoFontsCache::CairoFontsCache()
{
    ++mnRefCount;
}

CairoFontsCache::~CairoFontsCache()
{
    --mnRefCount;
    if (!mnRefCount && !maLRUFonts.empty())
    {
        CairoWrapper &rCairo = CairoWrapper::get();
        LRUFonts::iterator aEnd = maLRUFonts.end();
        for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
            rCairo.font_face_destroy((cairo_font_face_t*)aI->first);
    }
}

void CairoFontsCache::CacheFont(void *pFont, void* pId)
{
    maLRUFonts.push_front( std::pair<void*, void *>(pFont, pId) );
    if (maLRUFonts.size() > 8)
    {
        CairoWrapper &rCairo = CairoWrapper::get();
        rCairo.font_face_destroy((cairo_font_face_t*)maLRUFonts.back().first);
        maLRUFonts.pop_back();
    }
}

void* CairoFontsCache::FindCachedFont(void *pId)
{
    LRUFonts::iterator aEnd = maLRUFonts.end();
    for (LRUFonts::iterator aI = maLRUFonts.begin(); aI != aEnd; ++aI)
        if (aI->second == pId)
            return aI->first;
    return NULL;
}

void X11SalGraphics::DrawCairoAAFontString( const ServerFontLayout& rLayout )
{
    std::vector<cairo_glyph_t> cairo_glyphs;
    cairo_glyphs.reserve( 256 );
    
    Point aPos;
    sal_GlyphId aGlyphId;
    for( int nStart = 0; rLayout.GetNextGlyphs( 1, &aGlyphId, aPos, nStart ); )
    {
        cairo_glyph_t aGlyph;
        aGlyph.index = aGlyphId & GF_IDXMASK;
        aGlyph.x = aPos.X();
        aGlyph.y = aPos.Y();
        cairo_glyphs.push_back(aGlyph);
    }

    if (cairo_glyphs.empty())
        return;

    // find a XRenderPictFormat compatible with the Drawable
    XRenderPictFormat* pVisualFormat = static_cast<XRenderPictFormat*>(GetXRenderFormat());
    if( !pVisualFormat )
    {
        Visual* pVisual = GetDisplay()->GetVisual( m_nScreen ).GetVisual();
        pVisualFormat = XRenderPeer::GetInstance().FindVisualFormat( pVisual );
        // cache the XRenderPictFormat
        SetXRenderFormat( static_cast<void*>(pVisualFormat) );
    }

    DBG_ASSERT( pVisualFormat!=NULL, "no matching XRenderPictFormat for text" );
    if( !pVisualFormat )
	    return;

    CairoWrapper &rCairo = CairoWrapper::get();

    Display* pDisplay = GetXDisplay();

    cairo_surface_t *surface = rCairo.xlib_surface_create_with_xrender_format (pDisplay,
        hDrawable_, ScreenOfDisplay(pDisplay, m_nScreen), pVisualFormat, SAL_MAX_INT16, SAL_MAX_INT16);

    /*
     * It might be ideal to cache surface and cairo context between calls and
     * only destroy it when the drawable changes, but to do that we need to at
     * least change the SalFrame etc impls to dtor the SalGraphics *before* the
     * destruction of the windows they reference
    */
    cairo_t *cr = rCairo.create(surface);
    rCairo.surface_destroy(surface);

    if (const void *pOptions = Application::GetSettings().GetStyleSettings().GetCairoFontOptions())
        rCairo.set_font_options( cr, pOptions);

    if( mpClipRegion && !XEmptyRegion( mpClipRegion ) )
    {
        for (long i = 0; i < mpClipRegion->numRects; ++i)
        {
            rCairo.rectangle(cr, 
            mpClipRegion->rects[i].x1, 
            mpClipRegion->rects[i].y1,
            mpClipRegion->rects[i].x2 - mpClipRegion->rects[i].x1,
            mpClipRegion->rects[i].y2 - mpClipRegion->rects[i].y1);
        }
        rCairo.clip(cr);
    }

    rCairo.set_source_rgb(cr, 
        SALCOLOR_RED(nTextColor_)/255.0, 
        SALCOLOR_GREEN(nTextColor_)/255.0,
        SALCOLOR_BLUE(nTextColor_)/255.0);

    ServerFont& rFont = rLayout.GetServerFont();

    cairo_font_face_t* font_face = NULL;

    void *pId = rFont.GetFtFace();
    font_face = (cairo_font_face_t*)m_aCairoFontsCache.FindCachedFont(pId);
    if (!font_face)
    {
        font_face = rCairo.ft_font_face_create_for_ft_face(pId, rFont.GetLoadFlags());
        m_aCairoFontsCache.CacheFont(font_face, pId);
    }

    rCairo.set_font_face(cr, font_face);

    cairo_matrix_t m;
    const ImplFontSelectData& rFSD = rFont.GetFontSelData();
    int nWidth = rFSD.mnWidth ? rFSD.mnWidth : rFSD.mnHeight;

    rCairo.matrix_init_identity(&m);

    if (rLayout.GetOrientation())
        rCairo.matrix_rotate(&m, (3600 - rLayout.GetOrientation()) * M_PI / 1800.0);

    rCairo.matrix_scale(&m, nWidth, rFSD.mnHeight);
    if (rFont.NeedsArtificialItalic())
        m.xy = -m.xx * 0x6000L / 0x10000L;

    rCairo.set_font_matrix(cr, &m);
    rCairo.show_glyphs(cr, &cairo_glyphs[0], cairo_glyphs.size());
    rCairo.destroy(cr);
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerAAFontString( const ServerFontLayout& rLayout )
{
	// get xrender target for this drawable
    Picture aDstPic = GetXRenderPicture();
	if( !aDstPic )
		return;

    // get a XRenderPicture for the font foreground
    // TODO: move into own method
    XRenderPeer& rRenderPeer = XRenderPeer::GetInstance();
	XRenderPictFormat* pVisualFormat = (XRenderPictFormat*)GetXRenderFormat();
	DBG_ASSERT( pVisualFormat, "we already have a render picture, but XRenderPictFormat==NULL???");
    const int nVisualDepth = pVisualFormat->depth;
    SalDisplay::RenderEntry& rEntry = GetDisplay()->GetRenderEntries( m_nScreen )[ nVisualDepth ];
    if( !rEntry.m_aPicture )
    {
        // create and cache XRenderPicture for the font foreground
        Display* pDisplay = GetXDisplay();
#ifdef DEBUG
        int iDummy;
        unsigned uDummy;
        XLIB_Window wDummy;
        unsigned int nDrawDepth;
        ::XGetGeometry( pDisplay, hDrawable_, &wDummy, &iDummy, &iDummy,
                      &uDummy, &uDummy, &uDummy, &nDrawDepth );
        DBG_ASSERT( static_cast<unsigned>(nVisualDepth) == nDrawDepth, "depth messed up for XRender" );
#endif

        rEntry.m_aPixmap = ::XCreatePixmap( pDisplay, hDrawable_, 1, 1, nVisualDepth );

        XRenderPictureAttributes aAttr;
        aAttr.repeat = true;
        rEntry.m_aPicture = rRenderPeer.CreatePicture ( rEntry.m_aPixmap, pVisualFormat, CPRepeat, &aAttr );
    }

    // set font foreground color and opacity
    XRenderColor aRenderColor = GetXRenderColor( nTextColor_ );
    rRenderPeer.FillRectangle( PictOpSrc, rEntry.m_aPicture, &aRenderColor, 0, 0, 1, 1 );
	
    // set clipping
    // TODO: move into GetXRenderPicture()?
    if( mpClipRegion && !XEmptyRegion( mpClipRegion ) )
        rRenderPeer.SetPictureClipRegion( aDstPic, mpClipRegion );

    ServerFont& rFont = rLayout.GetServerFont();
    X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();
    GlyphSet aGlyphSet = rGlyphPeer.GetGlyphSet( rFont, m_nScreen );

    Point aPos;
    static const int MAXGLYPHS = 160;
    sal_GlyphId aGlyphAry[ MAXGLYPHS ];
    int nMaxGlyphs = rLayout.GetOrientation() ? 1 : MAXGLYPHS;
    for( int nStart = 0;;)
    {
        int nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, aGlyphAry, aPos, nStart );
        if( !nGlyphs )
            break;

        // #i51924# avoid 32->16bit coordinate truncation problem in X11
        // TODO: reevaluate once displays with >30000 pixels are available
        if( aPos.X() >= 30000 || aPos.Y() >= 30000 )
            continue;

        unsigned int aRenderAry[ MAXGLYPHS ];
        for( int i = 0; i < nGlyphs; ++i )
             aRenderAry[ i ] = rGlyphPeer.GetXRGlyph( rFont, aGlyphAry[i] );
        rRenderPeer.CompositeString32( rEntry.m_aPicture, aDstPic,
           aGlyphSet, aPos.X(), aPos.Y(), aRenderAry, nGlyphs );
    }
}

//--------------------------------------------------------------------------

bool X11SalGraphics::DrawServerAAForcedString( const ServerFontLayout& rLayout )
{
    ServerFont& rFont = rLayout.GetServerFont();

    // prepare glyphs and get extent of operation
    X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();
    int nXmin = 0;
    int nXmax = 0;
    int nYmin = 0;
    int nYmax = 0;
    int nStart = 0;
    Point aPos;
    sal_GlyphId nGlyph;
    for( bool bFirst=true; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = rGlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;
        const int nX2 = nX1 + pRawBitmap->mnWidth;
        const int nY2 = nY1 + pRawBitmap->mnHeight;

        if( bFirst )
        {
            bFirst = false;
            nXmin = nX1;
            nXmax = nX2;
            nYmin = nY1;
            nYmax = nY2;
        }
        else
        {
            if( nXmin > nX1 ) nXmin = nX1;
            if( nXmax < nX2 ) nXmax = nX2;
            if( nYmin > nY1 ) nYmin = nY1;
            if( nYmax < nY2 ) nYmax = nY2;
        }
    }

    // get XImage
    GetDisplay()->GetXLib()->PushXErrorLevel( true );
    Display* pDisplay = GetXDisplay();

    XRectangle aXRect;
    long nWidth = 1, nHeight = 1;
    if( m_pFrame )
        nWidth = m_pFrame->maGeometry.nWidth, nHeight = m_pFrame->maGeometry.nHeight;
    else if( m_pVDev )
        nWidth = m_pVDev->GetWidth(), nHeight = m_pVDev->GetHeight();

    if( mpClipRegion && !XEmptyRegion( mpClipRegion ) )
    {
        // get bounding box
        XClipBox( mpClipRegion, &aXRect );
        // clip with window
        if( aXRect.x < 0 ) aXRect.x = 0;

        if( aXRect.y < 0 ) aXRect.y = 0;
        if( aXRect.width+aXRect.x > nWidth ) aXRect.width = nWidth-aXRect.x;
        if( aXRect.height+aXRect.y > nHeight ) aXRect.height = nHeight-aXRect.y;
    }
    else
    {
        aXRect.x = 0;
        aXRect.y = 0;
        aXRect.width = nWidth;
        aXRect.height = nHeight;
    }
    if( m_pFrame )
    {
        // clip with screen
        int nScreenX = m_pFrame->maGeometry.nX+aXRect.x;
        int nScreenY = m_pFrame->maGeometry.nY+aXRect.y;
        const Size& rScreenSize = GetDisplay()->getDataForScreen( m_nScreen ).m_aSize;
        int nScreenW = rScreenSize.Width();
        int nScreenH = rScreenSize.Height();
        if( nScreenX < 0 )
            aXRect.x -= nScreenX, aXRect.width += nScreenX;
        if( nScreenX+aXRect.width > nScreenW )
            aXRect.width = nScreenW-nScreenX;
        if( nScreenY < 0 )
            aXRect.y -= nScreenY, aXRect.height += nScreenY;
        if( nScreenY+aXRect.height > nScreenH )
            aXRect.height = nScreenH-nScreenY;
    }


    if( nXmin < aXRect.x )  nXmin = aXRect.x;
    if( nYmin < aXRect.y )  nYmin = aXRect.y;
    if( nXmax >= aXRect.x+aXRect.width )    nXmax = aXRect.x + aXRect.width - 1;
    if( nYmax >= aXRect.y+aXRect.height )   nYmax = aXRect.y + aXRect.height - 1;

    if( nXmin > nXmax )
        return false;
    if( nYmin > nYmax )
        return false;

    XImage* pImg = XGetImage( pDisplay, hDrawable_,
                              nXmin, nYmin,
                              (nXmax-nXmin+1), (nYmax-nYmin+1),
                              ~0, ZPixmap );
    if( pImg == NULL )
    {
        if( m_pFrame )
        {
            // the reason we did not get an image could be that the frame
            // geometry changed in the meantime; lets get the current geometry
            // and clip against the current window size as well as the screen
            // with the current frame position
            const Size& rScreenSize = GetDisplay()->getDataForScreen(m_nScreen).m_aSize;
            int nScreenW = rScreenSize.Width();
            int nScreenH = rScreenSize.Height();
            XLIB_Window aRoot = None;
            int x = 0, y = 0;
            unsigned int w = 0, h = 0, bw = 0, d;
            XGetGeometry( pDisplay, hDrawable_, &aRoot, &x, &y, &w, &h, &bw, &d );
            XTranslateCoordinates( pDisplay, hDrawable_, aRoot, 0, 0, &x, &y, &aRoot );
            if( nXmin + x < 0 ) // clip on left screen edge
                nXmin += x-nXmin;
            if( nYmin + y < 0 ) // clip on top screen edge
                nYmin += y-nYmin;
            if( nXmax >= int(w) ) // clip on right window egde
                nXmax = w-1;
            if( nYmax >= int(h) ) // clip on bottom window edge
                nYmax = h-1;
            if( nXmax + x >= nScreenW ) // clip on right screen edge
                nXmax -= (nXmax + x - nScreenW)+1;
            if( nYmax + y >= nScreenH ) // clip on bottom screen edge
                nYmax -= (nYmax + y - nScreenH)+1;
            if( nXmax >= nXmin && nYmax >= nYmin )
            {
                // try again to get the image
                pImg = XGetImage( pDisplay, hDrawable_,
                                  nXmin, nYmin,
                                  (nXmax-nXmin+1), (nYmax-nYmin+1),
                                  ~0, ZPixmap );
            }
        }
        if( pImg == NULL )
        {
            GetDisplay()->GetXLib()->PopXErrorLevel();
            return false;
        }
    }

    // prepare context
    GC nGC = GetFontGC();
    XGCValues aGCVal;
    XGetGCValues( pDisplay, nGC, GCForeground, &aGCVal );

    unsigned long nOrigColor = XGetPixel( pImg, 0, 0 );
    XPutPixel( pImg, 0, 0, aGCVal.foreground );
    unsigned char aColor[4];
    aColor[0] = pImg->data[0];
    aColor[1] = pImg->data[1];
    aColor[2] = pImg->data[2];
    aColor[3] = pImg->data[3];
    XPutPixel( pImg, 0, 0, nOrigColor );

    // work on XImage
    const int bpp = pImg->bits_per_pixel >> 3;
    for( nStart = 0; rLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        const RawBitmap* const pRawBitmap = rGlyphPeer.GetRawBitmap( rFont, nGlyph );
        if( !pRawBitmap )
            continue;

        const int nX1 = aPos.X() + pRawBitmap->mnXOffset;
        const int nY1 = aPos.Y() + pRawBitmap->mnYOffset;

        if( (nX1 <= nXmax) && (int(nX1 + pRawBitmap->mnWidth) > nXmin)
        &&  (nY1 <= nYmax) && (int(nY1 + pRawBitmap->mnHeight) > nYmin) )
        {
            const unsigned char* p10 = pRawBitmap->mpBits;
            unsigned char* p20 = (unsigned char*)pImg->data;                // dest left limit
            p20 += (nY1 - nYmin) * pImg->bytes_per_line;
            unsigned char* p21 = p20 + (nX1 - nXmin + pImg->xoffset) * bpp;
            int y = pRawBitmap->mnHeight;
            if( y > nYmax - nY1 )
                y = nYmax - nY1 + 1;
            while( --y >= 0 )
            {
                if( p20 >= (unsigned char*)pImg->data )
                {
                    unsigned char* const p22 = p20 + pImg->width * bpp; // dest right limit
                    unsigned char* pDst = p21;
                    const unsigned char* pSrc = p10;
                    for( int x = pRawBitmap->mnWidth; (--x >= 0) && (p22 > pDst); ++pSrc )
                    {
                        if( (*pSrc == 0) || (p20 > pDst) )          // keep background
                            pDst += bpp;
                        else if( *pSrc == 0xFF )                    // paint foreground
                        {
                            const unsigned char* pColor = aColor;
                            for( int z = bpp; --z >= 0; ++pColor, ++pDst )
                                *pDst = *pColor;
                        }
                        else                                        // blend fg into bg
                        {
                            const unsigned char* pColor = aColor;
                            for( int z = bpp; --z >= 0; ++pColor, ++pDst )
                                // theoretically it should be *257) >> 16
                                // but the error is <0.4% worst case and we are in
                                // the innermost loop of very perf-sensitive code

                                *pDst += (*pSrc * ((int)*pColor - *pDst)) >> 8;
                        }
                    }
                }
                p10 += pRawBitmap->mnScanlineSize;
                p20 += pImg->bytes_per_line;
                p21 += pImg->bytes_per_line;
            }
        }
    }

    // put XImage
    XPutImage( pDisplay, hDrawable_, nGC, pImg,
        0, 0, nXmin, nYmin, (nXmax - nXmin + 1), (nYmax - nYmin + 1) );
    XDestroyImage( pImg );

    GetDisplay()->GetXLib()->PopXErrorLevel();
    return true;
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerSimpleFontString( const ServerFontLayout& rSalLayout )
{
    ServerFont& rFont = rSalLayout.GetServerFont();
    X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();

    Display* pDisplay = GetXDisplay();
    GC nGC = GetFontGC();

    XGCValues aGCVal;
    aGCVal.fill_style = FillStippled;
    aGCVal.line_width = 0;
    GC tmpGC = XCreateGC( pDisplay, hDrawable_, GCFillStyle|GCLineWidth, &aGCVal );
    XCopyGC( pDisplay, nGC, (1<<GCLastBit)-(1+GCFillStyle+GCLineWidth), tmpGC );

    Point aPos;
    sal_GlyphId nGlyph;
    for( int nStart = 0; rSalLayout.GetNextGlyphs( 1, &nGlyph, aPos, nStart ); )
    {
        // #i51924# avoid 32->16bit coordinate truncation problem in X11
        // TODO: reevaluate once displays with >30000 pixels are available
        if( aPos.X() >= 30000 || aPos.Y() >= 30000 )
            continue;

        Pixmap aStipple = rGlyphPeer.GetPixmap( rFont, nGlyph, m_nScreen );
        const GlyphMetric& rGM = rFont.GetGlyphMetric( nGlyph );

        if( aStipple != None )
        {
            const int nDestX    = aPos.X() + rGM.GetOffset().X();
            const int nDestY    = aPos.Y() + rGM.GetOffset().Y();

            aGCVal.stipple      = aStipple;
            aGCVal.ts_x_origin  = nDestX;
            aGCVal.ts_y_origin  = nDestY;
            XChangeGC( pDisplay, tmpGC, GCStipple|GCTileStipXOrigin|GCTileStipYOrigin, &aGCVal );

            const int nWidth    = rGM.GetSize().Width();
            const int nHeight   = rGM.GetSize().Height();
            XFillRectangle( pDisplay, hDrawable_, tmpGC, nDestX, nDestY, nWidth, nHeight );
        }
    }

    XFreeGC( pDisplay, tmpGC );
}

//--------------------------------------------------------------------------

void X11SalGraphics::DrawServerFontLayout( const ServerFontLayout& rLayout )
{
    // draw complex text
    ServerFont& rFont = rLayout.GetServerFont();
	const bool bVertical = rFont.GetFontSelData().mbVertical;

    if( !bVertical && CairoWrapper::get().isCairoRenderable(rFont) )
        DrawCairoAAFontString( rLayout );
    else
    {
        X11GlyphPeer& rGlyphPeer = X11GlyphCache::GetInstance().GetPeer();
        if( rGlyphPeer.GetGlyphSet( rFont, m_nScreen ) )
            DrawServerAAFontString( rLayout );
        else if( !rGlyphPeer.ForcedAntialiasing( rFont, m_nScreen ) )
            DrawServerSimpleFontString( rLayout );
        else
            DrawServerAAForcedString( rLayout );
    }
}

//--------------------------------------------------------------------------

const ImplFontCharMap* X11SalGraphics::GetImplFontCharMap() const
{
    if( !mpServerFont[0] )
        return NULL;

    const ImplFontCharMap* pIFCMap = mpServerFont[0]->GetImplFontCharMap();
    return pIFCMap;
}

// ----------------------------------------------------------------------------
//
// SalGraphics
//
// ----------------------------------------------------------------------------

sal_uInt16 X11SalGraphics::SetFont( ImplFontSelectData *pEntry, int nFallbackLevel )
{
    sal_uInt16 nRetVal = 0;
    if( !setFont( pEntry, nFallbackLevel ) )
        nRetVal |= SAL_SETFONT_BADFONT;
    if( bPrinter_ || (mpServerFont[ nFallbackLevel ] != NULL) )
        nRetVal |= SAL_SETFONT_USEDRAWTEXTARRAY;
    return nRetVal;
}

// ----------------------------------------------------------------------------

void
X11SalGraphics::SetTextColor( SalColor nSalColor )
{
    if( nTextColor_	!= nSalColor )
    {
        nTextColor_     = nSalColor;
        nTextPixel_     = GetPixel( nSalColor );
        bFontGC_        = sal_False;
    }
}

// ----------------------------------------------------------------------------

bool X11SalGraphics::AddTempDevFont( ImplDevFontList* pFontList,
    const String& rFileURL, const String& rFontName )
{
    // inform PSP font manager
    rtl::OUString aUSystemPath;
    OSL_VERIFY( !osl::FileBase::getSystemPathFromFileURL( rFileURL, aUSystemPath ) );
    rtl_TextEncoding aEncoding = osl_getThreadTextEncoding();
    OString aOFileName( OUStringToOString( aUSystemPath, aEncoding ) );
    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    int nFontId = rMgr.addFontFile( aOFileName, 0 );
    if( !nFontId )
        return false;

    // prepare font data
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontFastInfo( nFontId, aInfo );
    aInfo.m_aFamilyName = rFontName;

    // inform glyph cache of new font
    ImplDevFontAttributes aDFA = PspGraphics::Info2DevFontAttributes( aInfo );
    aDFA.mnQuality += 5800;

    int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
    if( nFaceNum < 0 )
        nFaceNum = 0;

    GlyphCache& rGC = X11GlyphCache::GetInstance();
    const rtl::OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
    rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA );

    // announce new font to device's font list
    rGC.AnnounceFonts( pFontList );
    return true;
}

// ----------------------------------------------------------------------------

void RegisterFontSubstitutors( ImplDevFontList* );

void X11SalGraphics::GetDevFontList( ImplDevFontList *pList )
{
    // prepare the GlyphCache using psprint's font infos
    X11GlyphCache& rGC = X11GlyphCache::GetInstance();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    ::std::list< psp::fontID > aList;
    ::std::list< psp::fontID >::iterator it;
    psp::FastPrintFontInfo aInfo;
    rMgr.getFontList( aList );
    for( it = aList.begin(); it != aList.end(); ++it )
    {
        if( !rMgr.getFontFastInfo( *it, aInfo ) )
            continue;

        // the GlyphCache must not bother with builtin fonts because
        // it cannot access or use them anyway
        if( aInfo.m_eType == psp::fonttype::Builtin )
            continue;

        // normalize face number to the GlyphCache
        int nFaceNum = rMgr.getFontFaceNumber( aInfo.m_nID );
        if( nFaceNum < 0 )
            nFaceNum = 0;

        // for fonts where extra kerning info can be provided on demand
        // an ExtraKernInfo object is supplied
        const ExtraKernInfo* pExtraKernInfo = NULL;
        if( aInfo.m_eType == psp::fonttype::Type1 )
            pExtraKernInfo = new PspKernInfo( *it );

        // inform GlyphCache about this font provided by the PsPrint subsystem
        ImplDevFontAttributes aDFA = PspGraphics::Info2DevFontAttributes( aInfo );
        aDFA.mnQuality += 4096;
        const rtl::OString& rFileName = rMgr.getFontFileSysPath( aInfo.m_nID );
        rGC.AddFontFile( rFileName, nFaceNum, aInfo.m_nID, aDFA, pExtraKernInfo );
   }

    // announce glyphcache fonts
    rGC.AnnounceFonts( pList );

    // register platform specific font substitutions if available
    if( rMgr.hasFontconfig() )
        RegisterFontSubstitutors( pList );
    
    ImplGetSVData()->maGDIData.mbNativeFontConfig = rMgr.hasFontconfig();
}

// ----------------------------------------------------------------------------

void X11SalGraphics::GetDevFontSubstList( OutputDevice* )
{
    // no device specific font substitutions on X11 needed
}

// ----------------------------------------------------------------------------

void cairosubcallback( void* pPattern )
{
    CairoWrapper& rCairo = CairoWrapper::get();
    if( !rCairo.isValid() )
        return;
    const StyleSettings& rStyleSettings = Application::GetSettings().GetStyleSettings();
    const void* pFontOptions = rStyleSettings.GetCairoFontOptions();
    if( !pFontOptions )
        return;
    rCairo.ft_font_options_substitute( pFontOptions, pPattern );
}

bool GetFCFontOptions( const ImplFontAttributes& rFontAttributes, int nSize,
	ImplFontOptions& rFontOptions)
{
    // TODO: get rid of these insane enum-conversions
    // e.g. by using the classic vclenum values inside VCL

    psp::FastPrintFontInfo aInfo;
    // set family name
    aInfo.m_aFamilyName = rFontAttributes.GetFamilyName();
    // set italic
    switch( rFontAttributes.GetSlant() )
    {
        case ITALIC_NONE:
            aInfo.m_eItalic = psp::italic::Upright;
            break;
        case ITALIC_NORMAL:
            aInfo.m_eItalic = psp::italic::Italic;
            break;
        case ITALIC_OBLIQUE:
            aInfo.m_eItalic = psp::italic::Oblique;
            break;
        default:
            aInfo.m_eItalic = psp::italic::Unknown;
            break;
    }
    // set weight
    switch( rFontAttributes.GetWeight() )
    {
        case WEIGHT_THIN:
            aInfo.m_eWeight = psp::weight::Thin;
            break;
        case WEIGHT_ULTRALIGHT:
            aInfo.m_eWeight = psp::weight::UltraLight;
            break;
        case WEIGHT_LIGHT:
            aInfo.m_eWeight = psp::weight::Light;
            break;
        case WEIGHT_SEMILIGHT:
            aInfo.m_eWeight = psp::weight::SemiLight;
            break;
        case WEIGHT_NORMAL:
            aInfo.m_eWeight = psp::weight::Normal;
            break;
        case WEIGHT_MEDIUM:
            aInfo.m_eWeight = psp::weight::Medium;
            break;
        case WEIGHT_SEMIBOLD:
            aInfo.m_eWeight = psp::weight::SemiBold;
            break;
        case WEIGHT_BOLD:
            aInfo.m_eWeight = psp::weight::Bold;
            break;
        case WEIGHT_ULTRABOLD:
            aInfo.m_eWeight = psp::weight::UltraBold;
            break;
        case WEIGHT_BLACK:
            aInfo.m_eWeight = psp::weight::Black;
            break;
        default:
            aInfo.m_eWeight = psp::weight::Unknown;
            break;
    }
    // set width
    switch( rFontAttributes.GetWidthType() )
    {
        case WIDTH_ULTRA_CONDENSED:
            aInfo.m_eWidth = psp::width::UltraCondensed;
            break;
        case WIDTH_EXTRA_CONDENSED:
            aInfo.m_eWidth = psp::width::ExtraCondensed;
            break;
        case WIDTH_CONDENSED:
            aInfo.m_eWidth = psp::width::Condensed;
            break;
        case WIDTH_SEMI_CONDENSED:
            aInfo.m_eWidth = psp::width::SemiCondensed;
            break;
        case WIDTH_NORMAL:
            aInfo.m_eWidth = psp::width::Normal;
            break;
        case WIDTH_SEMI_EXPANDED:
            aInfo.m_eWidth = psp::width::SemiExpanded;
            break;
        case WIDTH_EXPANDED:
            aInfo.m_eWidth = psp::width::Expanded;
            break;
        case WIDTH_EXTRA_EXPANDED:
            aInfo.m_eWidth = psp::width::ExtraExpanded;
            break;
        case WIDTH_ULTRA_EXPANDED:
            aInfo.m_eWidth = psp::width::UltraExpanded;
            break;
        default:
            aInfo.m_eWidth = psp::width::Unknown;
            break;
    }

    const psp::PrintFontManager& rPFM = psp::PrintFontManager::get();
    bool bOK = rPFM.getFontOptions( aInfo, nSize, cairosubcallback, rFontOptions);
    return bOK;
}

// ----------------------------------------------------------------------------

void
X11SalGraphics::GetFontMetric( ImplFontMetricData *pMetric, int nFallbackLevel )
{
    if( nFallbackLevel >= MAX_FALLBACK )
        return;

    if( mpServerFont[nFallbackLevel] != NULL )
    {
        long rDummyFactor;
        mpServerFont[nFallbackLevel]->FetchFontMetric( *pMetric, rDummyFactor );
    }
}

// ---------------------------------------------------------------------------

sal_uLong
X11SalGraphics::GetKernPairs( sal_uLong nPairs, ImplKernPairData *pKernPairs )
{
    if( ! bPrinter_ )
    {
        if( mpServerFont[0] != NULL )
        {
            ImplKernPairData* pTmpKernPairs;
            sal_uLong nGotPairs = mpServerFont[0]->GetKernPairs( &pTmpKernPairs );
            for( unsigned int i = 0; i < nPairs && i < nGotPairs; ++i )
                pKernPairs[ i ] = pTmpKernPairs[ i ];
            delete[] pTmpKernPairs;
            return nGotPairs;
        }
    }
	return 0;
}

// ---------------------------------------------------------------------------

bool X11SalGraphics::GetGlyphBoundRect( sal_GlyphId aGlyphId, Rectangle& rRect )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    ServerFont* pSF = mpServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= ~GF_FONTMASK;
    const GlyphMetric& rGM = pSF->GetGlyphMetric( aGlyphId );
    rRect = Rectangle( rGM.GetOffset(), rGM.GetSize() );
    return true;
}

// ---------------------------------------------------------------------------

bool X11SalGraphics::GetGlyphOutline( sal_GlyphId aGlyphId,
    ::basegfx::B2DPolyPolygon& rPolyPoly )
{
    const int nLevel = aGlyphId >> GF_FONTSHIFT;
    if( nLevel >= MAX_FALLBACK )
        return false;

    ServerFont* pSF = mpServerFont[ nLevel ];
    if( !pSF )
        return false;

    aGlyphId &= ~GF_FONTMASK;
    bool bOK = pSF->GetGlyphOutline( aGlyphId, rPolyPoly );
    return bOK;
}

//--------------------------------------------------------------------------

SalLayout* X11SalGraphics::GetTextLayout( ImplLayoutArgs& rArgs, int nFallbackLevel )
{
    SalLayout* pLayout = NULL;

    if( mpServerFont[ nFallbackLevel ]
    && !(rArgs.mnFlags & SAL_LAYOUT_DISABLE_GLYPH_PROCESSING) )
    {
#ifdef ENABLE_GRAPHITE
        // Is this a Graphite font?
        if (!bDisableGraphite_ &&
            GraphiteFontAdaptor::IsGraphiteEnabledFont(*mpServerFont[nFallbackLevel]))
        {
            sal_Int32 xdpi = GetDisplay()->GetResolution().A();
            sal_Int32 ydpi = GetDisplay()->GetResolution().B();

            GraphiteFontAdaptor * pGrfont = new GraphiteFontAdaptor( *mpServerFont[nFallbackLevel], xdpi, ydpi);
            if (!pGrfont) return NULL;
            pLayout = new GraphiteServerFontLayout(pGrfont);
        }
        else
#endif
            pLayout = new ServerFontLayout( *mpServerFont[ nFallbackLevel ] );
    }

    return pLayout;
}

//--------------------------------------------------------------------------

SystemFontData X11SalGraphics::GetSysFontData( int nFallbacklevel ) const
{
    SystemFontData aSysFontData;
    aSysFontData.nSize = sizeof( SystemFontData );
    aSysFontData.nFontId = 0;

    if (nFallbacklevel >= MAX_FALLBACK) nFallbacklevel = MAX_FALLBACK - 1;
    if (nFallbacklevel < 0 ) nFallbacklevel = 0;
    
    if (mpServerFont[nFallbacklevel] != NULL)
    {
        ServerFont* rFont = mpServerFont[nFallbacklevel];
        aSysFontData.nFontId = rFont->GetFtFace();
        aSysFontData.nFontFlags = rFont->GetLoadFlags();
        aSysFontData.bFakeBold = rFont->NeedsArtificialBold();
        aSysFontData.bFakeItalic = rFont->NeedsArtificialItalic();
        aSysFontData.bAntialias = rFont->GetAntialiasAdvice();
        aSysFontData.bVerticalCharacterType = rFont->GetFontSelData().mbVertical;
    }
            
    return aSysFontData;
}

//--------------------------------------------------------------------------

sal_Bool X11SalGraphics::CreateFontSubset(
                                   const rtl::OUString& rToFile,
                                   const ImplFontData* pFont,
                                   sal_GlyphId* pGlyphIds,
                                   sal_uInt8* pEncoding,
                                   sal_Int32* pWidths,
                                   int nGlyphCount,
                                   FontSubsetInfo& rInfo
                                   )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();

    psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    bool bSuccess = rMgr.createFontSubset( rInfo,
                                 aFont,
                                 rToFile,
                                 pGlyphIds,
                                 pEncoding,
                                 pWidths,
                                 nGlyphCount );
    return bSuccess;
}

//--------------------------------------------------------------------------

const void* X11SalGraphics::GetEmbedFontData( const ImplFontData* pFont, const sal_Ucs* pUnicodes, sal_Int32* pWidths, FontSubsetInfo& rInfo, long* pDataLen )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetEmbedFontData( aFont, pUnicodes, pWidths, rInfo, pDataLen );
}

//--------------------------------------------------------------------------

void X11SalGraphics::FreeEmbedFontData( const void* pData, long nLen )
{
    PspGraphics::DoFreeEmbedFontData( pData, nLen );
}

//--------------------------------------------------------------------------

const Ucs2SIntMap* X11SalGraphics::GetFontEncodingVector( const ImplFontData* pFont, const Ucs2OStrMap** pNonEncoded )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    return PspGraphics::DoGetFontEncodingVector( aFont, pNonEncoded );
}

//--------------------------------------------------------------------------

void X11SalGraphics::GetGlyphWidths( const ImplFontData* pFont,
                                   bool bVertical,
                                   Int32Vector& rWidths,
                                   Ucs2UIntMap& rUnicodeEnc )
{
    // in this context the pFont->GetFontId() is a valid PSP
    // font since they are the only ones left after the PDF
    // export has filtered its list of subsettable fonts (for
    // which this method was created). The correct way would
    // be to have the GlyphCache search for the ImplFontData pFont
    psp::fontID aFont = pFont->GetFontId();
    PspGraphics::DoGetGlyphWidths( aFont, bVertical, rWidths, rUnicodeEnc );
}

// ===========================================================================
// platform specific font substitution hooks

class FcPreMatchSubstititution
:   public ImplPreMatchFontSubstitution
{
public:
    bool FindFontSubstitute( ImplFontSelectData& ) const;
};

class FcGlyphFallbackSubstititution
:    public ImplGlyphFallbackFontSubstitution
{
    // TODO: add a cache
public:
    bool FindFontSubstitute( ImplFontSelectData&, OUString& rMissingCodes ) const;
};

void RegisterFontSubstitutors( ImplDevFontList* pList )
{
    // init font substitution defaults
    int nDisableBits = 0;
#ifdef SOLARIS
    nDisableBits = 1; // disable "font fallback" here on default
#endif
    // apply the environment variable if any
    const char* pEnvStr = ::getenv( "SAL_DISABLE_FC_SUBST" );
    if( pEnvStr )
    {
        if( (*pEnvStr >= '0') && (*pEnvStr <= '9') )
            nDisableBits = (*pEnvStr - '0'); 
        else
            nDisableBits = ~0U; // no specific bits set: disable all
    }

    // register font fallback substitutions (unless disabled by bit0)
    if( (nDisableBits & 1) == 0 )
    {
        static FcPreMatchSubstititution aSubstPreMatch;
        pList->SetPreMatchHook( &aSubstPreMatch );
    }

    // register glyph fallback substitutions (unless disabled by bit1)
    if( (nDisableBits & 2) == 0 )
    {
        static FcGlyphFallbackSubstititution aSubstFallback;
        pList->SetFallbackHook( &aSubstFallback );
    }
}

// -----------------------------------------------------------------------

static ImplFontSelectData GetFcSubstitute(const ImplFontSelectData &rFontSelData, OUString& rMissingCodes )
{
    ImplFontSelectData aRet(rFontSelData);

    const rtl::OString aLangAttrib = MsLangId::convertLanguageToIsoByteString( rFontSelData.meLanguage );

    psp::italic::type eItalic = psp::italic::Unknown;
    if( rFontSelData.GetSlant() != ITALIC_DONTKNOW )
    {
        switch( rFontSelData.GetSlant() )
        {
            case ITALIC_NONE:    eItalic = psp::italic::Upright; break;
            case ITALIC_NORMAL:  eItalic = psp::italic::Italic; break;
            case ITALIC_OBLIQUE: eItalic = psp::italic::Oblique; break;
            default:
                break;
        }
    }

    psp::weight::type eWeight = psp::weight::Unknown;
    if( rFontSelData.GetWeight() != WEIGHT_DONTKNOW )
    {
        switch( rFontSelData.GetWeight() )
        {
            case WEIGHT_THIN:		eWeight = psp::weight::Thin; break;
            case WEIGHT_ULTRALIGHT:	eWeight = psp::weight::UltraLight; break;
            case WEIGHT_LIGHT:		eWeight = psp::weight::Light; break;
            case WEIGHT_SEMILIGHT:	eWeight = psp::weight::SemiLight; break;
            case WEIGHT_NORMAL:		eWeight = psp::weight::Normal; break;
            case WEIGHT_MEDIUM:		eWeight = psp::weight::Medium; break;
            case WEIGHT_SEMIBOLD:	eWeight = psp::weight::SemiBold; break;
            case WEIGHT_BOLD:		eWeight = psp::weight::Bold; break;
            case WEIGHT_ULTRABOLD:	eWeight = psp::weight::UltraBold; break;
            case WEIGHT_BLACK:		eWeight = psp::weight::Black; break;
            default:
                break;
        }
    }

    psp::width::type eWidth = psp::width::Unknown;
    if( rFontSelData.GetWidthType() != WIDTH_DONTKNOW )
    {
        switch( rFontSelData.GetWidthType() )
        {
            case WIDTH_ULTRA_CONDENSED:	eWidth = psp::width::UltraCondensed; break;
            case WIDTH_EXTRA_CONDENSED: eWidth = psp::width::ExtraCondensed; break;
            case WIDTH_CONDENSED:	eWidth = psp::width::Condensed; break;
            case WIDTH_SEMI_CONDENSED:	eWidth = psp::width::SemiCondensed; break;
            case WIDTH_NORMAL:		eWidth = psp::width::Normal; break;
            case WIDTH_SEMI_EXPANDED:	eWidth = psp::width::SemiExpanded; break;
            case WIDTH_EXPANDED:	eWidth = psp::width::Expanded; break;
            case WIDTH_EXTRA_EXPANDED:	eWidth = psp::width::ExtraExpanded; break;
            case WIDTH_ULTRA_EXPANDED:	eWidth = psp::width::UltraExpanded; break;
            default:
                break;
        }
    }

    psp::pitch::type ePitch = psp::pitch::Unknown;
    if( rFontSelData.GetPitch() != PITCH_DONTKNOW )
    {
        switch( rFontSelData.GetPitch() )
        {
            case PITCH_FIXED:    ePitch=psp::pitch::Fixed; break;
            case PITCH_VARIABLE: ePitch=psp::pitch::Variable; break;
            default:
                break;
        }
    }

    const psp::PrintFontManager& rMgr = psp::PrintFontManager::get();
    aRet.maSearchName = rMgr.Substitute( rFontSelData.maTargetName, rMissingCodes, aLangAttrib, eItalic, eWeight, eWidth, ePitch);

    switch (eItalic)
    {
        case psp::italic::Upright: aRet.meItalic = ITALIC_NONE; break;
        case psp::italic::Italic: aRet.meItalic = ITALIC_NORMAL; break;
        case psp::italic::Oblique: aRet.meItalic = ITALIC_OBLIQUE; break;
        default:
            break;
    }

    switch (eWeight)
    {
        case psp::weight::Thin: aRet.meWeight = WEIGHT_THIN; break;
        case psp::weight::UltraLight: aRet.meWeight = WEIGHT_ULTRALIGHT; break;
        case psp::weight::Light: aRet.meWeight = WEIGHT_LIGHT; break;
        case psp::weight::SemiLight: aRet.meWeight = WEIGHT_SEMILIGHT; break;
        case psp::weight::Normal: aRet.meWeight = WEIGHT_NORMAL; break;
        case psp::weight::Medium: aRet.meWeight = WEIGHT_MEDIUM; break;
        case psp::weight::SemiBold: aRet.meWeight = WEIGHT_SEMIBOLD; break;
        case psp::weight::Bold: aRet.meWeight = WEIGHT_BOLD; break;
        case psp::weight::UltraBold: aRet.meWeight = WEIGHT_ULTRABOLD; break;
        case psp::weight::Black: aRet.meWeight = WEIGHT_BLACK; break;
        default:
                break;
    }

    switch (eWidth)
    {
        case psp::width::UltraCondensed: aRet.meWidthType = WIDTH_ULTRA_CONDENSED; break;
        case psp::width::ExtraCondensed: aRet.meWidthType = WIDTH_EXTRA_CONDENSED; break;
        case psp::width::Condensed: aRet.meWidthType = WIDTH_CONDENSED; break;
        case psp::width::SemiCondensed: aRet.meWidthType = WIDTH_SEMI_CONDENSED; break;
        case psp::width::Normal: aRet.meWidthType = WIDTH_NORMAL; break;
        case psp::width::SemiExpanded: aRet.meWidthType = WIDTH_SEMI_EXPANDED; break;
        case psp::width::Expanded: aRet.meWidthType = WIDTH_EXPANDED; break;
        case psp::width::ExtraExpanded: aRet.meWidthType = WIDTH_EXTRA_EXPANDED; break;
        case psp::width::UltraExpanded: aRet.meWidthType = WIDTH_ULTRA_EXPANDED; break;
        default:
            break;
    }

    switch (ePitch)
    {
        case psp::pitch::Fixed: aRet.mePitch = PITCH_FIXED; break;
        case psp::pitch::Variable: aRet.mePitch = PITCH_VARIABLE; break;
        default:
            break;
    }

    return aRet;
}

namespace
{
    bool uselessmatch(const ImplFontSelectData &rOrig, const ImplFontSelectData &rNew)
    {
        return
          (
            rOrig.maTargetName == rNew.maSearchName &&
            rOrig.meWeight == rNew.meWeight &&
            rOrig.meItalic == rNew.meItalic &&
            rOrig.mePitch == rNew.mePitch &&
            rOrig.meWidthType == rNew.meWidthType
          );
    }
}

//--------------------------------------------------------------------------

bool FcPreMatchSubstititution::FindFontSubstitute( ImplFontSelectData &rFontSelData ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
        return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    rtl::OUString aDummy;
    const ImplFontSelectData aOut = GetFcSubstitute( rFontSelData, aDummy );
    // TODO: cache the font substitution suggestion
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const ByteString aOrigName( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    const ByteString aSubstName( aOut.maSearchName, RTL_TEXTENCODING_UTF8 );
    printf( "FcPreMatchSubstititution \"%s\" bipw=%d%d%d%d -> ",
        aOrigName.GetBuffer(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.GetBuffer(),
	    aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// -----------------------------------------------------------------------

bool FcGlyphFallbackSubstititution::FindFontSubstitute( ImplFontSelectData& rFontSelData,
    rtl::OUString& rMissingCodes ) const
{
    // We dont' actually want to talk to Fontconfig at all for symbol fonts
    if( rFontSelData.IsSymbolFont() )
	return false;
    // StarSymbol is a unicode font, but it still deserves the symbol flag
    if( 0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "starsymbol", 10)
    ||  0 == rFontSelData.maSearchName.CompareIgnoreCaseToAscii( "opensymbol", 10) )
        return false;

    const ImplFontSelectData aOut = GetFcSubstitute( rFontSelData, rMissingCodes );
    // TODO: cache the unicode + srcfont specific result
    // FC doing it would be preferable because it knows the invariables
    // e.g. FC knows the FC rule that all Arial gets replaced by LiberationSans
    // whereas we would have to check for every size or attribute
    if( !aOut.maSearchName.Len() )
        return false;

    const bool bHaveSubstitute = !uselessmatch( rFontSelData, aOut );

#ifdef DEBUG
    const ByteString aOrigName( rFontSelData.maTargetName, RTL_TEXTENCODING_UTF8 );
    const ByteString aSubstName( aOut.maSearchName, RTL_TEXTENCODING_UTF8 );
    printf( "FcGFSubstititution \"%s\" bipw=%d%d%d%d ->",
        aOrigName.GetBuffer(), rFontSelData.meWeight, rFontSelData.meItalic,
        rFontSelData.mePitch, rFontSelData.meWidthType );
    if( !bHaveSubstitute )
        printf( "no substitute available\n" );
    else
        printf( "\"%s\" bipw=%d%d%d%d\n", aSubstName.GetBuffer(),
	    aOut.meWeight, aOut.meItalic, aOut.mePitch, aOut.meWidthType );
#endif

    if( bHaveSubstitute )
        rFontSelData = aOut;

    return bHaveSubstitute;
}

// ===========================================================================

