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


#ifndef _SFX_FRMDESCRHXX
#define _SFX_FRMDESCRHXX

#include "sal/config.h"
#include "sfx2/dllapi.h"

#include <sfx2/sfxsids.hrc>
#include <svl/poolitem.hxx>
#include <tools/urlobj.hxx>
#include <tools/string.hxx>
#include <svl/svarray.hxx>
#include <tools/gen.hxx>

class SvStream;
struct SfxFrameDescriptor_Impl;
class SvStrings;
class SfxFrameDescriptor;
class Wallpaper;
typedef SfxFrameDescriptor* SfxFrameDescriptorPtr;
SV_DECL_PTRARR(SfxFramesArr, SfxFrameDescriptorPtr, 4, 2)

//===========================================================================
// Die SfxFrame...descriptoren bauen eine rekursive Struktur auf, die
// s"amtliche zur Anzeige des Frame-Dokuments erforderlichen Daten umfa\st.
// "Uber einen SfxFrameSetDescriptor hat man Zugriff auf die gesamte darunter
// liegende Struktur.
// Aufgrund der besonderen Eigenschaften des SfxFrames-Dokuments ist ein
// SfxFramesSetDescriptor nicht nur der Inhalt dieses Dokuments, sondern
// beschreibt auch die View darauf.
// Das FrameSet wird aus Zeilen aufgebaut, die wiederum die eigentlichen
// Fenster enthalten. Eine Zeile kann horizontal oder vertikal ausgerichtet
// sein, woraus sich auch das Alignment des FrameSets ergibt.
//===========================================================================

enum ScrollingMode
{
	ScrollingYes,
	ScrollingNo,
	ScrollingAuto
};

enum SizeSelector
{
	SIZE_ABS,
	SIZE_PERCENT,
	SIZE_REL
};

#define BORDER_SET			2
#define BORDER_YES			1
#define BORDER_NO			0
#define SPACING_NOT_SET		-1L
#define SIZE_NOT_SET		-1L

class SfxItemSet;
struct SfxFrameProperties;

class SFX2_DLLPUBLIC SfxFrameDescriptor
{
	INetURLObject			aURL;
	INetURLObject			aActualURL;
	String					aName;
	Size					aMargin;
	long					nWidth;
	ScrollingMode			eScroll;
	SizeSelector			eSizeSelector;
	sal_uInt16					nHasBorder;
	sal_uInt16					nItemId;
	sal_Bool					bResizeHorizontal;
	sal_Bool					bResizeVertical;
	sal_Bool					bHasUI;
	sal_Bool                    bReadOnly;
	SfxFrameDescriptor_Impl* pImp;
	SvStrings*				pScripts;
	SvStrings*				pComments;

public:
                            SfxFrameDescriptor();
							~SfxFrameDescriptor();

							// Eigenschaften
	void					TakeProperties( const SfxFrameProperties& rProp );

							// FileName/URL
	SfxItemSet*             GetArgs();
	const INetURLObject&	GetURL() const
							{ return aURL; }
	void					SetURL( const INetURLObject& rURL );
	void					SetURL( const String& rURL );
	const INetURLObject&	GetActualURL() const
							{ return aActualURL; }
	void					SetActualURL( const INetURLObject& rURL );
	void					SetActualURL( const String& rURL );
	sal_Bool					CheckContent() const;
	sal_Bool                    CompareOriginal( SfxFrameDescriptor& rSet ) const;
	void					UnifyContent( sal_Bool );
	void                    SetReadOnly( sal_Bool bSet ) { bReadOnly = bSet;}
	sal_Bool                    IsReadOnly(  ) const { return bReadOnly;}
	void                    SetEditable( sal_Bool bSet );
	sal_Bool                    IsEditable() const;

							// Size
	void					SetWidth( long n )
							{ nWidth = n; }
	void					SetWidthPercent( long n )
							{ nWidth = n; eSizeSelector = SIZE_PERCENT; }
	void					SetWidthRel( long n )
							{ nWidth = n; eSizeSelector = SIZE_REL; }
	void					SetWidthAbs( long n )
							{ nWidth = n; eSizeSelector = SIZE_ABS; }
	long					GetWidth() const
							{ return nWidth; }
	SizeSelector			GetSizeSelector() const
							{ return eSizeSelector; }
	sal_Bool					IsResizable() const
							{ return bResizeHorizontal && bResizeVertical; }
	void					SetResizable( sal_Bool bRes )
							{ bResizeHorizontal = bResizeVertical = bRes; }

							// FrameName
	const String&			GetName() const
							{ return aName; }
	void					SetName( const String& rName )
							{ aName = rName; }

							// Margin, Scrolling
	const Size&				GetMargin() const
							{ return aMargin; }
	void					SetMargin( const Size& rMargin )
							{ aMargin = rMargin; }
	ScrollingMode			GetScrollingMode() const
							{ return eScroll; }
	void					SetScrollingMode( ScrollingMode eMode )
							{ eScroll = eMode; }

							// FrameBorder
	void					SetWallpaper( const Wallpaper& rWallpaper );
	const Wallpaper*		GetWallpaper() const;
	sal_Bool					HasFrameBorder() const;

	sal_Bool					IsFrameBorderOn() const
							{ return ( nHasBorder & BORDER_YES ) != 0; }

	void					SetFrameBorder( sal_Bool bBorder )
							{
								nHasBorder = bBorder ?
											BORDER_YES | BORDER_SET :
											BORDER_NO | BORDER_SET;
							}
	sal_Bool					IsFrameBorderSet() const
							{ return (nHasBorder & BORDER_SET) != 0; }
	void					ResetBorder()
							{ nHasBorder = 0; }

	sal_Bool					HasUI() const
							{ return bHasUI; }
	void					SetHasUI( sal_Bool bOn )
							{ bHasUI = bOn; }

							// Attribute f"ur das Splitwindow
	sal_uInt16					GetItemId() const
							{ return nItemId; }
	void					SetItemId( sal_uInt16 nId )
							{ nItemId = nId; }
	sal_uInt16 					GetWinBits() const;
	long					GetSize() const;
	sal_uInt16					GetItemPos() const;

							// Kopie z.B. f"ur die Views
    SfxFrameDescriptor*     Clone( sal_Bool bWithIds = sal_True ) const;
};

// Kein Bock, einen operator= zu implementieren...
struct SfxFrameProperties
{
	String								aURL;
	String								aName;
	long								lMarginWidth;
	long								lMarginHeight;
	long								lSize;
	long								lSetSize;
	long								lFrameSpacing;
	long								lInheritedFrameSpacing;
	ScrollingMode						eScroll;
	SizeSelector						eSizeSelector;
	SizeSelector						eSetSizeSelector;
	sal_Bool								bHasBorder;
	sal_Bool								bBorderSet;
	sal_Bool								bResizable;
	sal_Bool								bSetResizable;
	sal_Bool								bIsRootSet;
	sal_Bool								bIsInColSet;
	sal_Bool								bHasBorderInherited;
	SfxFrameDescriptor*              	pFrame;

private:
	SfxFrameProperties( SfxFrameProperties& ) {}
public:
										SfxFrameProperties()
											: lMarginWidth( SIZE_NOT_SET ),
											  lMarginHeight( SIZE_NOT_SET ),
											  lSize( 1L ),
											  lSetSize( 1L ),
											  lFrameSpacing( SPACING_NOT_SET ),
											  lInheritedFrameSpacing( SPACING_NOT_SET ),
											  eScroll( ScrollingAuto ),
											  eSizeSelector( SIZE_REL ),
											  eSetSizeSelector( SIZE_REL ),
											  bHasBorder( sal_True ),
											  bBorderSet( sal_True ),
											  bResizable( sal_True ),
											  bSetResizable( sal_True ),
											  bIsRootSet( sal_False ),
											  bIsInColSet( sal_False ),
											  bHasBorderInherited( sal_True ),
											  pFrame( 0 ) {}

										SfxFrameProperties( const SfxFrameDescriptor *pD );
										~SfxFrameProperties() { delete pFrame; }

	int             					operator ==( const SfxFrameProperties& ) const;
	SfxFrameProperties&				    operator =( const SfxFrameProperties &rProp );
};

class SfxFrameDescriptorItem : public SfxPoolItem
{
	SfxFrameProperties					aProperties;
public:
										TYPEINFO();

										SfxFrameDescriptorItem ( const SfxFrameDescriptor *pD, const sal_uInt16 nId = SID_FRAMEDESCRIPTOR )
											: SfxPoolItem( nId )
											, aProperties( pD )
										{}

										SfxFrameDescriptorItem ( const sal_uInt16 nId = SID_FRAMEDESCRIPTOR )
											: SfxPoolItem( nId )
										{}

										SfxFrameDescriptorItem( const SfxFrameDescriptorItem& rCpy )
											: SfxPoolItem( rCpy )
										{
											aProperties = rCpy.aProperties;
										}

	virtual								~SfxFrameDescriptorItem();

	virtual int             			operator ==( const SfxPoolItem& ) const;
	SfxFrameDescriptorItem&				operator =( const SfxFrameDescriptorItem & );

	virtual SfxItemPresentation 		GetPresentation( SfxItemPresentation ePres,
											SfxMapUnit eCoreMetric,
											SfxMapUnit ePresMetric,
                                            UniString &rText, const IntlWrapper * = 0 ) const;

	virtual SfxPoolItem*    			Clone( SfxItemPool *pPool = 0 ) const;
	//virtual SfxPoolItem*    			Create(SvStream &, sal_uInt16) const;
	//virtual SvStream&					Store(SvStream &, sal_uInt16 nItemVersion ) const;
	//virtual sal_uInt16						GetVersion( sal_uInt16 nFileFormatVersion ) const;

	const SfxFrameProperties&			GetProperties() const
										{ return aProperties; }
	void 								SetProperties( const SfxFrameProperties& rProp )
										{ aProperties = rProp; }
};

#endif // #ifndef _SFX_FRMDESCRHXX

