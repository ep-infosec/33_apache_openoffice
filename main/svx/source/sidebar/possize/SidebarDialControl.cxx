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

#include <svx/sidebar/SidebarDialControl.hxx>
#include "SidebarDialControlBmp.hxx"

#include <vcl/svapp.hxx>

namespace svx { namespace sidebar {

SidebarDialControl::SidebarDialControl (
    Window* pParent,
    const ResId& rResId)
    : svx::DialControl(pParent, rResId)
{
    mpImpl->mpBmpEnabled.reset(new SidebarDialControlBmp(*this));
    mpImpl->mpBmpDisabled.reset(new SidebarDialControlBmp(*this));
    mpImpl->mpBmpBuffered.reset(new SidebarDialControlBmp(*this));
    Init(GetOutputSizePixel());
}




SidebarDialControl::~SidebarDialControl (void)
{
}




void SidebarDialControl::MouseButtonDown( const MouseEvent& rMEvt )
{
    if( rMEvt.IsLeft() )
    {
        GrabFocus();
        CaptureMouse();
        mpImpl->mnOldAngle = mpImpl->mnAngle;
        HandleMouseEvent( rMEvt.GetPosPixel(), true );
    }
}




void SidebarDialControl::HandleMouseEvent( const Point& rPos, bool bInitial )
{
    long nX = rPos.X() - mpImpl->mnCenterX;
    long nY = mpImpl->mnCenterY - rPos.Y();
    double fH = sqrt( static_cast< double >( nX ) * nX + static_cast< double >( nY ) * nY );
    if( fH != 0.0 )
    {
        double fAngle = acos( nX / fH );
        sal_Int32 nAngle = static_cast< sal_Int32 >( fAngle / F_PI180 * 100.0 );
        if( nY < 0 )
            nAngle = 36000 - nAngle;
        if( bInitial )  // round to entire 15 degrees
            nAngle = ((nAngle + 750) / 1500) * 1500;

		if (Application::GetSettings().GetLayoutRTL())
			nAngle = 18000 - nAngle;
        SetRotation( nAngle, true );
    }
}

} } // end of namespace svx::sidebar

// eof
