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

#include <svtools/svmedit2.hxx>
#include <svtools/xtextedt.hxx>

ExtMultiLineEdit::ExtMultiLineEdit( Window* pParent, WinBits nWinStyle ) :

	MultiLineEdit( pParent, nWinStyle )

{
}

ExtMultiLineEdit::ExtMultiLineEdit( Window* pParent, const ResId& rResId ) :

	MultiLineEdit( pParent, rResId )

{
}

ExtMultiLineEdit::~ExtMultiLineEdit()
{
}

void ExtMultiLineEdit::InsertText( const String& rNew, sal_Bool )
{
	GetTextView()->InsertText( rNew, sal_False );
}

void ExtMultiLineEdit::SetAutoScroll( sal_Bool bAutoScroll )
{
	GetTextView()->SetAutoScroll( bAutoScroll );
}

void ExtMultiLineEdit::EnableCursor( sal_Bool bEnable )
{
	GetTextView()->EnableCursor( bEnable );
}

void ExtMultiLineEdit::SetAttrib( const TextAttrib& rAttr, sal_uLong nPara, sal_uInt16 nStart, sal_uInt16 nEnd )
{
	GetTextEngine()->SetAttrib( rAttr, nPara, nStart, nEnd );
}

void ExtMultiLineEdit::SetLeftMargin( sal_uInt16 nLeftMargin )
{
	GetTextEngine()->SetLeftMargin( nLeftMargin );
}

sal_uLong ExtMultiLineEdit::GetParagraphCount() const
{
	return GetTextEngine()->GetParagraphCount();
}

