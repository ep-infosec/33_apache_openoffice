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
#include "precompiled_sd.hxx"

#include "sdpage.hxx"
#include "View.hxx"
#include "pres.hxx"

namespace sd {

static bool implIsMultiPresObj( PresObjKind eKind )
{
	switch( eKind )
	{						
	case PRESOBJ_OUTLINE:
	case PRESOBJ_GRAPHIC:
	case PRESOBJ_OBJECT:
	case PRESOBJ_CHART:
	case PRESOBJ_ORGCHART:
	case PRESOBJ_TABLE:
	case PRESOBJ_IMAGE:
	case PRESOBJ_MEDIA:
		return true;
	default:
		return false;
	}
}

SdrObject* View::GetEmptyPresentationObject( PresObjKind eKind )
{
	SdrObject* pEmptyObj = 0;

	SdrPageView*	pPV = GetSdrPageView();
	if( pPV )
	{
		SdPage* pPage = static_cast< SdPage* >( pPV->GetPage() );
		if( pPage && !pPage->IsMasterPage() )
		{
			// first try selected shape
			if ( AreObjectsMarked() )
			{
				/**********************************************************
				* Is an empty graphic object available?
				**********************************************************/
				const SdrMarkList& rMarkList = GetMarkedObjectList();

				if (rMarkList.GetMarkCount() == 1)
				{
					SdrMark* pMark = rMarkList.GetMark(0);
					SdrObject* pObj = pMark->GetMarkedSdrObj();

					if( pObj->IsEmptyPresObj() && implIsMultiPresObj( pPage->GetPresObjKind(pObj) ) )
						pEmptyObj = pObj;
				}
			}

			// try to find empty pres obj of same type
			if( !pEmptyObj )
			{
				int nIndex = 1;
				do
				{
					pEmptyObj = pPage->GetPresObj(eKind, nIndex++ );
				}
				while( (pEmptyObj != 0) && (!pEmptyObj->IsEmptyPresObj()) );
			}

			// last try to find empty pres obj of multiple type
			if( !pEmptyObj )
			{
				const std::list< SdrObject* >& rShapes = pPage->GetPresentationShapeList().getList();
		    	
    			for( std::list< SdrObject* >::const_iterator iter( rShapes.begin() ); iter != rShapes.end(); iter++ )
    			{
    				if( (*iter)->IsEmptyPresObj() && implIsMultiPresObj(pPage->GetPresObjKind(*iter)) )
    				{
						pEmptyObj = (*iter);
						break;
					}
				}
			}
		}
	}

	return pEmptyObj;
}

}
