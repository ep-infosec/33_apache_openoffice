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



#include "svpvd.hxx"
#include "svpgdi.hxx"

#include <basegfx/vector/b2ivector.hxx>
#include <basebmp/scanlineformats.hxx>

#include "stdio.h"

using namespace basegfx;
using namespace basebmp;

SvpSalVirtualDevice::~SvpSalVirtualDevice()
{
}

SalGraphics* SvpSalVirtualDevice::GetGraphics()
{
    SvpSalGraphics* pGraphics = new SvpSalGraphics();
    pGraphics->setDevice( m_aDevice );
    m_aGraphics.push_back( pGraphics );
    return pGraphics;
}

void SvpSalVirtualDevice::ReleaseGraphics( SalGraphics* pGraphics )
{
    m_aGraphics.remove( dynamic_cast<SvpSalGraphics*>(pGraphics) );
    delete pGraphics;
}

sal_Bool SvpSalVirtualDevice::SetSize( long nNewDX, long nNewDY )
{
    B2IVector aDevSize( nNewDX, nNewDY );
    if( aDevSize.getX() == 0 )
        aDevSize.setX( 1 );
    if( aDevSize.getY() == 0 )
        aDevSize.setY( 1 );
    if( ! m_aDevice.get() || m_aDevice->getSize() != aDevSize )
    {
        sal_uInt32 nFormat = SVP_DEFAULT_BITMAP_FORMAT;
        std::vector< basebmp::Color > aDevPal;
        switch( m_nBitCount )
        {
            case 1: nFormat = Format::ONE_BIT_MSB_PAL;
                aDevPal.reserve(2);
                aDevPal.push_back( basebmp::Color( 0, 0, 0 ) );
                aDevPal.push_back( basebmp::Color( 0xff, 0xff, 0xff ) );
                break;
            case 4: nFormat = Format::FOUR_BIT_MSB_PAL; break;
            case 8: nFormat = Format::EIGHT_BIT_PAL; break;
#ifdef OSL_BIGENDIAN
            case 16: nFormat = Format::SIXTEEN_BIT_MSB_TC_MASK; break;
#else
            case 16: nFormat = Format::SIXTEEN_BIT_LSB_TC_MASK; break;
#endif
            case 0:
            case 24: nFormat = Format::TWENTYFOUR_BIT_TC_MASK; break;
            case 32: nFormat = Format::THIRTYTWO_BIT_TC_MASK; break;
        }
        m_aDevice = aDevPal.empty()
                    ? createBitmapDevice( aDevSize, false, nFormat )
                    : createBitmapDevice( aDevSize, false, nFormat, PaletteMemorySharedVector( new std::vector< basebmp::Color >(aDevPal) ) );
        
        // update device in existing graphics
        for( std::list< SvpSalGraphics* >::iterator it = m_aGraphics.begin();
             it != m_aGraphics.end(); ++it )
             (*it)->setDevice( m_aDevice );
             
    }
    return true;
}

void SvpSalVirtualDevice::GetSize( long& rWidth, long& rHeight )
{
    if( m_aDevice.get() )
    {
        B2IVector aDevSize( m_aDevice->getSize() );
        rWidth = aDevSize.getX();
        rHeight = aDevSize.getY();
    }
    else
        rWidth = rHeight = 0;
}

