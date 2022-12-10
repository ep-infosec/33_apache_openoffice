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


#ifndef _ROMENU_HXX
#define _ROMENU_HXX


#ifndef _GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#include <tools/list.hxx>
#ifndef _MENU_HXX //autogen
#include <vcl/menu.hxx>
#endif

class SwView;
class SfxDispatcher;
class SvxBrushItem;
class ImageMap;
class INetImage;

class SwReadOnlyPopup : public PopupMenu
{
		  SwView &rView;
	const SvxBrushItem *pItem;
	const Point &rDocPos;
				Graphic aGraphic;
	String		sURL,
				sTargetFrameName,
				sDescription,
				sGrfName;
	List		aThemeList;
	sal_Bool 		bGrfToGalleryAsLnk;
	ImageMap*	pImageMap;
	INetImage*	pTargetURL;

	void Check( sal_uInt16 nMID, sal_uInt16 nSID, SfxDispatcher &rDis );
	String SaveGraphic( sal_uInt16 nId );

    using PopupMenu::Execute;

public:
	SwReadOnlyPopup( const Point &rDPos, SwView &rV );
	~SwReadOnlyPopup();
	
	void Execute( Window* pWin, const Point &rPPos );
    void Execute( Window* pWin, sal_uInt16 nId );
};


#endif

