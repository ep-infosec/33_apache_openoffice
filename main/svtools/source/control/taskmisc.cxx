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
#include "precompiled_svtools.hxx"

#define _TASKBAR_CXX

#include <tools/list.hxx>
#include <tools/debug.hxx>
#include <vcl/help.hxx>
#include <svtools/taskbar.hxx>

// =======================================================================

TaskButtonBar::TaskButtonBar( Window* pParent, WinBits nWinStyle ) :
	ToolBox( pParent, nWinStyle | WB_3DLOOK )
{
	SetAlign( WINDOWALIGN_BOTTOM );
	SetButtonType( BUTTON_SYMBOLTEXT );
}

// -----------------------------------------------------------------------

TaskButtonBar::~TaskButtonBar()
{
}

// -----------------------------------------------------------------------

void TaskButtonBar::RequestHelp( const HelpEvent& rHEvt )
{
	ToolBox::RequestHelp( rHEvt );
}

// =======================================================================

WindowArrange::WindowArrange()
{
	mpWinList = new List;
}

// -----------------------------------------------------------------------

WindowArrange::~WindowArrange()
{
	delete mpWinList;
}

// -----------------------------------------------------------------------

static sal_uInt16 ImplCeilSqareRoot( sal_uInt16 nVal )
{
	sal_uInt16 i;

	// Ueberlauf verhindern
	if ( nVal > 0xFE * 0xFE )
		return 0xFE;

	for ( i=0; i*i < nVal; i++ )
		{}

	return i;
}

// -----------------------------------------------------------------------

static void ImplPosSizeWindow( Window* pWindow,
							   long nX, long nY, long nWidth, long nHeight )
{
	if ( nWidth < 32 )
		nWidth = 32;
	if ( nHeight < 24 )
		nHeight = 24;
	pWindow->SetPosSizePixel( nX, nY, nWidth, nHeight );
}

// -----------------------------------------------------------------------

void WindowArrange::ImplTile( const Rectangle& rRect )
{
	sal_uInt16 nCount = (sal_uInt16)mpWinList->Count();
	if ( nCount < 3 )
	{
		ImplVert( rRect );
		return;
	}

	sal_uInt16		i;
	sal_uInt16		j;
	sal_uInt16		nCols;
	sal_uInt16		nRows;
	sal_uInt16		nActRows;
	sal_uInt16		nOffset;
	long		nOverWidth;
	long		nOverHeight;
	Window* 	pWindow;
	long		nX = rRect.Left();
	long		nY = rRect.Top();
	long		nWidth = rRect.GetWidth();
	long		nHeight = rRect.GetHeight();
	long		nRectY = nY;
	long		nRectWidth = nWidth;
	long		nRectHeight = nHeight;
	long		nTempWidth;
	long		nTempHeight;

	nCols	= ImplCeilSqareRoot( nCount );
	nOffset = (nCols*nCols) - nCount;
	if ( nOffset >= nCols )
	{
		nRows	 = nCols -1;
		nOffset = nOffset - nCols;
	}
	else
		nRows = nCols;

	nWidth /= nCols;
	if ( nWidth < 1 )
		nWidth = 1;
	nOverWidth = nRectWidth-(nWidth*nCols);

	pWindow = (Window*)mpWinList->First();
	for ( i = 0; i < nCols; i++ )
	{
		if ( i < nOffset )
			nActRows = nRows - 1;
		else
			nActRows = nRows;

		nTempWidth = nWidth;
		if ( nOverWidth > 0 )
		{
			nTempWidth++;
			nOverWidth--;
		}

		nHeight = nRectHeight / nActRows;
		if ( nHeight < 1 )
			nHeight = 1;
		nOverHeight = nRectHeight-(nHeight*nActRows);
		for ( j = 0; j < nActRows; j++ )
		{
			// Ueberhang verteilen
			nTempHeight = nHeight;
			if ( nOverHeight > 0 )
			{
				nTempHeight++;
				nOverHeight--;
			}
			ImplPosSizeWindow( pWindow, nX, nY, nTempWidth, nTempHeight );
			nY += nTempHeight;

			pWindow = (Window*)mpWinList->Next();
			if ( !pWindow )
				break;
		}

		nX += nWidth;
		nY = nRectY;

		if ( !pWindow )
			break;
	}
}

// -----------------------------------------------------------------------

void WindowArrange::ImplHorz( const Rectangle& rRect )
{
	long		nCount = (long)mpWinList->Count();
	long		nX = rRect.Left();
	long		nY = rRect.Top();
	long		nWidth = rRect.GetWidth();
	long		nHeight = rRect.GetHeight();
	long		nRectHeight = nHeight;
	long		nOver;
	long		nTempHeight;
	Window* 	pWindow;

	nHeight /= nCount;
	if ( nHeight < 1 )
		nHeight = 1;
	nOver = nRectHeight - (nCount*nHeight);
	pWindow = (Window*)mpWinList->First();
	while ( pWindow )
	{
		nTempHeight = nHeight;
		if ( nOver > 0 )
		{
			nTempHeight++;
			nOver--;
		}
		ImplPosSizeWindow( pWindow, nX, nY, nWidth, nTempHeight );
		nY += nTempHeight;

		pWindow = (Window*)mpWinList->Next();
	}
}

// -----------------------------------------------------------------------

void WindowArrange::ImplVert( const Rectangle& rRect )
{
	long		nCount = (long)mpWinList->Count();
	long		nX = rRect.Left();
	long		nY = rRect.Top();
	long		nWidth = rRect.GetWidth();
	long		nHeight = rRect.GetHeight();
	long		nRectWidth = nWidth;
	long		nOver;
	long		nTempWidth;
	Window* 	pWindow;

	nWidth /= nCount;
	if ( nWidth < 1 )
		nWidth = 1;
	nOver = nRectWidth - (nCount*nWidth);
	pWindow = (Window*)mpWinList->First();
	while ( pWindow )
	{
		nTempWidth = nWidth;
		if ( nOver > 0 )
		{
			nTempWidth++;
			nOver--;
		}
		ImplPosSizeWindow( pWindow, nX, nY, nTempWidth, nHeight );
		nX += nTempWidth;

		pWindow = (Window*)mpWinList->Next();
	}
}

// -----------------------------------------------------------------------

void WindowArrange::ImplCascade( const Rectangle& rRect )
{
	long		nX = rRect.Left();
	long		nY = rRect.Top();
	long		nWidth = rRect.GetWidth();
	long		nHeight = rRect.GetHeight();
	long		nRectWidth = nWidth;
	long		nRectHeight = nHeight;
	long		nOff;
	long		nCascadeWins;
	sal_Int32	nLeftBorder;
	sal_Int32	nTopBorder;
	sal_Int32	nRightBorder;
	sal_Int32	nBottomBorder;
	long		nStartOverWidth;
	long		nStartOverHeight;
	long		nOverWidth = 0;
	long		nOverHeight = 0;
	long		nTempX;
	long		nTempY;
	long		nTempWidth;
	long		nTempHeight;
	long		i;
	Window* 	pWindow;
	Window* 	pTempWindow;

	// Border-Fenster suchen um den Versatz zu ermitteln
	pTempWindow = (Window*)mpWinList->First();
	pTempWindow->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
	while ( !nTopBorder )
	{
		Window* pBrdWin = pTempWindow->GetWindow( WINDOW_REALPARENT );
		if ( !pBrdWin || (pBrdWin->GetWindow( WINDOW_CLIENT ) != pTempWindow) )
			break;
		pTempWindow = pBrdWin;
		pTempWindow->GetBorder( nLeftBorder, nTopBorder, nRightBorder, nBottomBorder );
	}
	if ( !nTopBorder )
		nTopBorder = 22;
	nOff = nTopBorder;

	nCascadeWins = nRectHeight / 3 / nOff;
	if ( !nCascadeWins )
		nCascadeWins = 1;
	nWidth	 -= nCascadeWins*nOff;
	nHeight  -= nCascadeWins*nOff;
	if ( nWidth < 1 )
		nWidth = 1;
	if ( nHeight < 1 )
		nHeight = 1;

	nStartOverWidth = nRectWidth-(nWidth+(nCascadeWins*nOff));
	nStartOverHeight = nRectHeight-(nHeight+(nCascadeWins*nOff));

	i = 0;
	pWindow = (Window*)mpWinList->First();
	while ( pWindow )
	{
		if ( !i )
		{
			nOverWidth = nStartOverWidth;
			nOverHeight = nStartOverHeight;
		}

		// Position
		nTempX = nX + (i*nOff);
		nTempY = nY + (i*nOff);

		// Ueberhang verteilen
		nTempWidth = nWidth;
		if ( nOverWidth > 0 )
		{
			nTempWidth++;
			nOverWidth--;
		}
		nTempHeight = nHeight;
		if ( nOverHeight > 0 )
		{
			nTempHeight++;
			nOverHeight--;
		}

		ImplPosSizeWindow( pWindow, nTempX, nTempY, nTempWidth, nTempHeight );

		if ( i < nCascadeWins )
			i++;
		else
			i = 0;

		pWindow = (Window*)mpWinList->Next();
	}
}

// -----------------------------------------------------------------------

void WindowArrange::Arrange( sal_uInt16 nType, const Rectangle& rRect )
{
	if ( !mpWinList->Count() )
		return;

	switch ( nType )
	{
		case WINDOWARRANGE_TILE:
			ImplTile( rRect );
			break;
		case WINDOWARRANGE_HORZ:
			ImplHorz( rRect );
			break;
		case WINDOWARRANGE_VERT:
			ImplVert( rRect );
			break;
		case WINDOWARRANGE_CASCADE:
			ImplCascade( rRect );
			break;
	}
}

