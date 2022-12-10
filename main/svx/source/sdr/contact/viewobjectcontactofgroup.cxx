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
#include "precompiled_svx.hxx"

#include <svx/sdr/contact/viewobjectcontactofgroup.hxx>
#include <svx/sdr/contact/displayinfo.hxx>
#include <svx/sdr/contact/objectcontact.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <drawinglayer/primitive2d/groupprimitive2d.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <svx/sdr/contact/viewcontact.hxx>

//////////////////////////////////////////////////////////////////////////////

using namespace com::sun::star;

//////////////////////////////////////////////////////////////////////////////

namespace sdr
{
	namespace contact
	{
		ViewObjectContactOfGroup::ViewObjectContactOfGroup(ObjectContact& rObjectContact, ViewContact& rViewContact)
		:	ViewObjectContactOfSdrObj(rObjectContact, rViewContact)
		{
		}

		ViewObjectContactOfGroup::~ViewObjectContactOfGroup()
		{
		}

		drawinglayer::primitive2d::Primitive2DSequence ViewObjectContactOfGroup::getPrimitive2DSequenceHierarchy(DisplayInfo& rDisplayInfo) const
		{
			drawinglayer::primitive2d::Primitive2DSequence xRetval;

			// check model-view visibility
			if(isPrimitiveVisible(rDisplayInfo))
			{
				const sal_uInt32 nSubHierarchyCount(GetViewContact().GetObjectCount());

				if(nSubHierarchyCount)
				{
					const sal_Bool bDoGhostedDisplaying(
						GetObjectContact().DoVisualizeEnteredGroup()
						&& !GetObjectContact().isOutputToPrinter()
						&& GetObjectContact().getActiveViewContact() == &GetViewContact());

					if(bDoGhostedDisplaying)
					{
						rDisplayInfo.ClearGhostedDrawMode();
					}

					// create object hierarchy
					xRetval = getPrimitive2DSequenceSubHierarchy(rDisplayInfo);

					if(xRetval.hasElements())
					{
						// get ranges
						const drawinglayer::geometry::ViewInformation2D& rViewInformation2D(GetObjectContact().getViewInformation2D());
						const ::basegfx::B2DRange aObjectRange(drawinglayer::primitive2d::getB2DRangeFromPrimitive2DSequence(xRetval, rViewInformation2D));
						const basegfx::B2DRange aViewRange(rViewInformation2D.getViewport());

						// check geometrical visibility
						if(!aViewRange.isEmpty() && !aViewRange.overlaps(aObjectRange))
						{
							// not visible, release
							xRetval.realloc(0);
						}
					}

					if(bDoGhostedDisplaying)
					{
						rDisplayInfo.SetGhostedDrawMode();
					}
				}
				else
				{
					// draw replacement object for group. This will use ViewContactOfGroup::createViewIndependentPrimitive2DSequence
					// which creates the replacement primitives for an empty group
					xRetval = ViewObjectContactOfSdrObj::getPrimitive2DSequenceHierarchy(rDisplayInfo);
				}
			}
			return xRetval;
		}
	} // end of namespace contact
} // end of namespace sdr

//////////////////////////////////////////////////////////////////////////////
// eof
