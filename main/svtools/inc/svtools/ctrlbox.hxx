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



#ifndef _CTRLBOX_HXX
#define _CTRLBOX_HXX

#include "svtools/svtdllapi.h"

#ifndef _LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif
#ifndef _IMAGE_HXX
#include <vcl/image.hxx>
#endif
#ifndef _VIRDEV_HXX
#include <vcl/virdev.hxx>
#endif
#ifndef _METRIC_HXX
#include <vcl/metric.hxx>
#endif
#ifndef _FIELD_HXX
#include <vcl/field.hxx>
#endif

class ImplFontList;
class ImpColorList;
class ImpLineList;
class FontList;

/*************************************************************************

Beschreibung
============

class ColorListBox

Beschreibung

Erlaubt die Auswahl von Farben.

--------------------------------------------------------------------------

class LineListBox

Beschreibung

Erlaubt die Auswahl von Linien-Styles und Groessen. Es ist darauf zu achten,
das vor dem ersten Insert die Units und die Fesntergroesse gesetzt sein
muessen. An Unit wird Point und mm unterstuetzt und als SourceUnit Point,
mm und Twips. Alle Angaben muessen in 100teln der jeweiligen Einheit
vorliegen.

Line1 ist aeussere, Line2 die innere und Distance die Distanz zwischen
den beiden Linien. Wenn Line2 = 0 ist, wird nur Line1 angezeigt. Als
Default sind sowohl Source als auch Ziel-Unit FUNIT_POINT.

Mit SetColor() kann die Linienfarbe eingestellt werden.

Anmerkungen und Ausnahmen

Gegenueber einer normalen ListBox, koennen keine User-Daten gesetzt
werden. Ausserdem sollte wenn der UpdateMode ausgeschaltet ist, keine
Daten abgefragt oder die Selektion gesetzt werden, da in diesem Zustand
die Daten nicht definiert sind. Wenn der UpdateMode ausgeschaltet ist,
sollte der Rueckgabewert bei Insert nicht ausgewertet werden, da er keine
Bedeutung hat. Ausserdem darf nicht das WinBit WB_SORT gesetzt sein.

--------------------------------------------------------------------------

class FontNameBox

Beschreibung

Erlaubt die Auswahl von Fonts. Die ListBox wird mit Fill gefuellt, wo
ein Pointer auf eine FontList uebergeben werden muss.

Mit EnableWYSIWYG() kann man einstellen, das die Fontnamen in Ihrer Schrift
angezeigt werden und mit EnableSymbols() kann eingestellt werden, das
vor dem Namen ueber ein Symbol angezeigt wird, ob es sich um eine
Drucker oder Bildschirmschrift handelt.

Querverweise

FontList; FontStyleBox; FontSizeBox; FontNameMenu

--------------------------------------------------------------------------

class FontStyleBox

Beschreibung

Erlaubt die Auswahl eines FontStyles. Mit Fill wird die ListBox mit
den Styles zum uebergebenen Font gefuellt. Nachgebildete Styles werden
immer mit eingefuegt (kann sich aber noch aendern, da vielleicht
nicht alle Applikationen [StarDraw,Formel,FontWork] mit Syntetic-Fonts
umgehen koennen). Bei Fill bleibt vorherige Name soweit wie moeglich
erhalten.

Fuer DontKnow sollte die FontStyleBox mit String() gefuellt werden.
Dann enthaellt die Liste die Standardattribute. Der Style, der gerade
angezeigt wird, muss gegebenenfalls noch vom Programm zurueckgesetzt werden.

Querverweise

FontList; FontNameBox; FontSizeBox; FontStyleMenu

--------------------------------------------------------------------------

class FontSizeBox

Beschreibung

Erlaubt die Auswahl von Fontgroessen. Werte werden ueber GetValue()
abgefragt und ueber SetValue() gesetzt. Fill fuellt die ListBox mit den
Groessen zum uebergebenen Font. Alle Groessen werden in 10tel Point
angegeben. Die FontListe, die bei Fill uebergeben wird, muss bis zum
naechsten Fill-Aufruf erhalten bleiben.

Zusaetzlich erlaubt die FontSizeBox noch einen Relative-Mode. Dieser
dient dazu, Prozent-Werte eingeben zu koennen. Dies kann zum Beispiel
nuetzlich sein, wenn man die Box in einem Vorlagen-Dialog anbietet.
Dieser Modus ist nur anschaltbar, jedoch nicht wieder abschaltbar.

Fuer DontKnow sollte die FontSizeBox mit FontInfo() gefuellt werden.
Dann enthaellt die Liste die Standardgroessen. Die Groesse, die gerade
angezeigt wird, muss gegebenenfalls noch vom Programm zurueckgesetzt werden.

Querverweise

FontList; FontNameBox; FontStyleBox; FontSizeMenu

*************************************************************************/

// ----------------
// - ColorListBox -
// ----------------

class SVT_DLLPUBLIC ColorListBox : public ListBox
{
	ImpColorList*	pColorList; // Separate Liste, falls UserDaten von aussen verwendet werden.
	Size			aImageSize;

#ifdef _CTRLBOX_CXX
    using Window::ImplInit;
	SVT_DLLPRIVATE void			ImplInit();
	SVT_DLLPRIVATE void			ImplDestroyColorEntries();
#endif
public:
					ColorListBox( Window* pParent,
								  WinBits nWinStyle = WB_BORDER );
					ColorListBox( Window* pParent, const ResId& rResId );
	virtual 		~ColorListBox();

	virtual void	UserDraw( const UserDrawEvent& rUDEvt );

    using ListBox::InsertEntry;
	virtual sal_uInt16	InsertEntry( const XubString& rStr,
								 sal_uInt16 nPos = LISTBOX_APPEND );
	virtual sal_uInt16	InsertEntry( const Color& rColor, const XubString& rStr,
								 sal_uInt16 nPos = LISTBOX_APPEND );
	void			InsertAutomaticEntry();
    using ListBox::RemoveEntry;
	virtual void	RemoveEntry( sal_uInt16 nPos );
	virtual void	Clear();
	void			CopyEntries( const ColorListBox& rBox );

    using ListBox::GetEntryPos;
	virtual sal_uInt16	GetEntryPos( const Color& rColor ) const;
	virtual Color	GetEntryColor( sal_uInt16 nPos ) const;
	Size			GetImageSize() const { return aImageSize; }

	void			SelectEntry( const XubString& rStr, sal_Bool bSelect = sal_True )
						{ ListBox::SelectEntry( rStr, bSelect ); }
	void			SelectEntry( const Color& rColor, sal_Bool bSelect = sal_True );
	XubString		GetSelectEntry( sal_uInt16 nSelIndex = 0 ) const
						{ return ListBox::GetSelectEntry( nSelIndex ); }
	Color			GetSelectEntryColor( sal_uInt16 nSelIndex = 0 ) const;
	sal_Bool			IsEntrySelected( const XubString& rStr ) const
						{ return ListBox::IsEntrySelected( rStr ); }

	sal_Bool			IsEntrySelected( const Color& rColor ) const;

private:
	// declared as private because some compilers would generate the default functions
					ColorListBox( const ColorListBox& );
	ColorListBox&	operator =( const ColorListBox& );

	void			SetEntryData( sal_uInt16 nPos, void* pNewData );
	void*			GetEntryData( sal_uInt16 nPos ) const;
};

inline void ColorListBox::SelectEntry( const Color& rColor, sal_Bool bSelect )
{
	sal_uInt16 nPos = GetEntryPos( rColor );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		ListBox::SelectEntryPos( nPos, bSelect );
}

inline sal_Bool ColorListBox::IsEntrySelected( const Color& rColor ) const
{
	sal_uInt16 nPos = GetEntryPos( rColor );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		return IsEntryPosSelected( nPos );
	else
		return sal_False;
}

inline Color ColorListBox::GetSelectEntryColor( sal_uInt16 nSelIndex ) const
{
	sal_uInt16 nPos = GetSelectEntryPos( nSelIndex );
	Color aColor;
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		aColor = GetEntryColor( nPos );
	return aColor;
}

// ---------------
// - LineListBox -
// ---------------

class SVT_DLLPUBLIC LineListBox : public ListBox
{
	ImpLineList*	pLineList;
	VirtualDevice	aVirDev;
	Size			aTxtSize;
	Color			aColor;
	Color			maPaintCol;
	FieldUnit		eUnit;
	FieldUnit		eSourceUnit;

	SVT_DLLPRIVATE void			ImpGetLine( long nLine1, long nLine2, long nDistance, Bitmap& rBmp, XubString& rStr );
    using Window::ImplInit;
	SVT_DLLPRIVATE void			ImplInit();
	void			UpdateLineColors( void );
	sal_Bool			UpdatePaintLineColor( void );		// returns sal_True if maPaintCol has changed
	inline const Color&	GetPaintColor( void ) const;
	virtual void	DataChanged( const DataChangedEvent& rDCEvt );

public:
					LineListBox( Window* pParent, WinBits nWinStyle = WB_BORDER );
					LineListBox( Window* pParent, const ResId& rResId );
	virtual 		~LineListBox();

    using ListBox::InsertEntry;
	virtual sal_uInt16	InsertEntry( const XubString& rStr, sal_uInt16 nPos = LISTBOX_APPEND );
	virtual sal_uInt16	InsertEntry( long nLine1, long nLine2 = 0, long nDistance = 0, sal_uInt16 nPos = LISTBOX_APPEND );
    using ListBox::RemoveEntry;
	virtual void	RemoveEntry( sal_uInt16 nPos );
	virtual void	Clear();

    using ListBox::GetEntryPos;
	sal_uInt16			GetEntryPos( long nLine1, long nLine2 = 0, long nDistance = 0 ) const;
	long			GetEntryLine1( sal_uInt16 nPos ) const;
	long			GetEntryLine2( sal_uInt16 nPos ) const;
	long			GetEntryDistance( sal_uInt16 nPos ) const;

	inline void		SelectEntry( const XubString& rStr, sal_Bool bSelect = sal_True ) { ListBox::SelectEntry( rStr, bSelect ); }
	void			SelectEntry( long nLine1, long nLine2 = 0, long nDistance = 0, sal_Bool bSelect = sal_True );
	long			GetSelectEntryLine1( sal_uInt16 nSelIndex = 0 ) const;
	long			GetSelectEntryLine2( sal_uInt16 nSelIndex = 0 ) const;
	long			GetSelectEntryDistance( sal_uInt16 nSelIndex = 0 ) const;
	inline sal_Bool		IsEntrySelected( const XubString& rStr ) const { return ListBox::IsEntrySelected( rStr ); }
	sal_Bool			IsEntrySelected( long nLine1, long nLine2 = 0, long nDistance = 0 ) const;

	inline void		SetUnit( FieldUnit eNewUnit ) { eUnit = eNewUnit; }
	inline FieldUnit	GetUnit() const { return eUnit; }
	inline void		SetSourceUnit( FieldUnit eNewUnit ) { eSourceUnit = eNewUnit; }
	inline FieldUnit	GetSourceUnit() const { return eSourceUnit; }

	void			SetColor( const Color& rColor );
	inline Color	GetColor( void ) const;

private:
	// declared as private because some compilers would generate the default methods
					LineListBox( const LineListBox& );
	LineListBox&	operator =( const LineListBox& );
	void			SetEntryData( sal_uInt16 nPos, void* pNewData );
	void*			GetEntryData( sal_uInt16 nPos ) const;
};

inline void LineListBox::SelectEntry( long nLine1, long nLine2, long nDistance, sal_Bool bSelect )
{
	sal_uInt16 nPos = GetEntryPos( nLine1, nLine2, nDistance );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		ListBox::SelectEntryPos( nPos, bSelect );
}

inline long LineListBox::GetSelectEntryLine1( sal_uInt16 nSelIndex ) const
{
	sal_uInt16 nPos = GetSelectEntryPos( nSelIndex );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		return GetEntryLine1( nPos );
	else
		return 0;
}

inline long LineListBox::GetSelectEntryLine2( sal_uInt16 nSelIndex ) const
{
	sal_uInt16 nPos = GetSelectEntryPos( nSelIndex );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		return GetEntryLine2( nPos );
	else
		return 0;
}

inline long LineListBox::GetSelectEntryDistance( sal_uInt16 nSelIndex ) const
{
	sal_uInt16 nPos = GetSelectEntryPos( nSelIndex );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		return GetEntryDistance( nPos );
	else
		return 0;
}

inline sal_Bool LineListBox::IsEntrySelected( long nLine1, long nLine2, long nDistance ) const
{
	sal_uInt16 nPos = GetEntryPos( nLine1, nLine2, nDistance );
	if ( nPos != LISTBOX_ENTRY_NOTFOUND )
		return IsEntryPosSelected( nPos );
	else
		return sal_False;
}

inline void LineListBox::SetColor( const Color& rColor )
{
    aColor = rColor;

	UpdateLineColors();
}

inline Color LineListBox::GetColor( void ) const
{
	return aColor;
}


// ---------------
// - FontNameBox -
// ---------------

class SVT_DLLPUBLIC FontNameBox : public ComboBox
{
private:
	ImplFontList*	mpFontList;
	Image			maImagePrinterFont;
	Image			maImageBitmapFont;
	Image			maImageScalableFont;
	sal_Bool			mbWYSIWYG;
	sal_Bool			mbSymbols;

#ifdef _CTRLBOX_CXX
	SVT_DLLPRIVATE void			ImplCalcUserItemSize();
	SVT_DLLPRIVATE void			ImplDestroyFontList();
#endif

	void			InitBitmaps( void );
protected:
	virtual void	DataChanged( const DataChangedEvent& rDCEvt );
public:
					FontNameBox( Window* pParent,
								 WinBits nWinStyle = WB_SORT );
					FontNameBox( Window* pParent, const ResId& rResId );
	virtual 		~FontNameBox();

	virtual void	UserDraw( const UserDrawEvent& rUDEvt );

	void			Fill( const FontList* pList );

	void			EnableWYSIWYG( sal_Bool bEnable = sal_True );
	sal_Bool			IsWYSIWYGEnabled() const { return mbWYSIWYG; }

	void			EnableSymbols( sal_Bool bEnable = sal_True );
	sal_Bool			IsSymbolsEnabled() const { return mbSymbols; }

private:
	// declared as private because some compilers would generate the default functions
					FontNameBox( const FontNameBox& );
	FontNameBox&	operator =( const FontNameBox& );
};

// ----------------
// - FontStyleBox -
// ----------------

class SVT_DLLPUBLIC FontStyleBox : public ComboBox
{
	XubString		aLastStyle;

private:
    using ComboBox::SetText;
public:
					FontStyleBox( Window* pParent, WinBits nWinStyle = 0 );
					FontStyleBox( Window* pParent, const ResId& rResId );
	virtual 		~FontStyleBox();

	virtual void	Select();
	virtual void	LoseFocus();
	virtual void	Modify();

	void			SetText( const XubString& rText );
	void			Fill( const XubString& rName, const FontList* pList );

private:
	// declared as private because some compilers would generate the default functions
					FontStyleBox( const FontStyleBox& );
	FontStyleBox&	operator =( const FontStyleBox& );
};

inline void FontStyleBox::SetText( const XubString& rText )
{
	aLastStyle = rText;
	ComboBox::SetText( rText );
}

// ---------------
// - FontSizeBox -
// ---------------

class SVT_DLLPUBLIC FontSizeBox : public MetricBox
{
	FontInfo		aFontInfo;
	const FontList* pFontList;
	sal_uInt16			nRelMin;
	sal_uInt16			nRelMax;
	sal_uInt16			nRelStep;
	short			nPtRelMin;
	short			nPtRelMax;
	short			nPtRelStep;
	sal_Bool			bRelativeMode:1,
					bRelative:1,
					bPtRelative:1,
					bStdSize:1;

#ifdef _CTRLBOX_CXX
    using Window::ImplInit;
	SVT_DLLPRIVATE void			ImplInit();
#endif

protected:
	virtual XubString CreateFieldText( sal_Int64 nValue ) const;

public:
					FontSizeBox( Window* pParent, WinBits nWinStyle = 0 );
					FontSizeBox( Window* pParent, const ResId& rResId );
	virtual 		~FontSizeBox();

	void			Reformat();
	void			Modify();

	void			Fill( const FontInfo* pInfo, const FontList* pList );

	void			EnableRelativeMode( sal_uInt16 nMin = 50, sal_uInt16 nMax = 150,
										sal_uInt16 nStep = 5 );
	void			EnablePtRelativeMode( short nMin = -200, short nMax = 200,
										  short nStep = 10 );
	sal_Bool			IsRelativeMode() const { return bRelativeMode; }
	void			SetRelative( sal_Bool bRelative = sal_False );
	sal_Bool			IsRelative() const { return bRelative; }
	void			SetPtRelative( sal_Bool bPtRel = sal_True )
						{ bPtRelative = bPtRel; SetRelative( sal_True ); }
	sal_Bool			IsPtRelative() const { return bPtRelative; }

	virtual void	SetValue( sal_Int64 nNewValue, FieldUnit eInUnit );
	virtual void	SetValue( sal_Int64 nNewValue  );
	virtual sal_Int64	GetValue( FieldUnit eOutUnit ) const;
	virtual sal_Int64	GetValue() const;
	sal_Int64			GetValue( sal_uInt16 nPos, FieldUnit eOutUnit ) const;
	void			SetUserValue( sal_Int64 nNewValue, FieldUnit eInUnit );
	void			SetUserValue( sal_Int64 nNewValue ) { SetUserValue( nNewValue, FUNIT_NONE ); }

private:
	// declared as private because some compilers would generate the default functions
					FontSizeBox( const FontSizeBox& );
	FontSizeBox&	operator =( const FontSizeBox& );
};

#endif	// _CTRLBOX_HXX
