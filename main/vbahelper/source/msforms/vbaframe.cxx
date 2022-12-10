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



#include "vbaframe.hxx"
#include "vbanewfont.hxx"
#include "vbacontrols.hxx"
#include <ooo/vba/msforms/fmBorderStyle.hpp>
#include <ooo/vba/msforms/fmSpecialEffect.hpp>

using namespace com::sun::star;
using namespace ooo::vba;


const static rtl::OUString LABEL( RTL_CONSTASCII_USTRINGPARAM("Label") );

ScVbaFrame::ScVbaFrame(
        const uno::Reference< XHelperInterface >& xParent,
        const uno::Reference< uno::XComponentContext >& xContext,
        const uno::Reference< uno::XInterface >& xControl,
        const uno::Reference< frame::XModel >& xModel,
        ov::AbstractGeometryAttributes* pGeomHelper,
        const css::uno::Reference< css::awt::XControl >& xDialog ) :
    FrameImpl_BASE( xParent, xContext, xControl, xModel, pGeomHelper ),
    mxDialog( xDialog )
{
}

// XFrame attributes

rtl::OUString SAL_CALL ScVbaFrame::getCaption() throw (css::uno::RuntimeException)
{
    rtl::OUString Label;
    m_xProps->getPropertyValue( LABEL ) >>= Label;
    return Label;
}

void SAL_CALL ScVbaFrame::setCaption( const rtl::OUString& _caption ) throw (::com::sun::star::uno::RuntimeException)
{
    m_xProps->setPropertyValue( LABEL, uno::makeAny( _caption ) );
}

sal_Int32 SAL_CALL ScVbaFrame::getSpecialEffect() throw (uno::RuntimeException)
{
    return msforms::fmSpecialEffect::fmSpecialEffectEtched;
}

void SAL_CALL ScVbaFrame::setSpecialEffect( sal_Int32 /*nSpecialEffect*/ ) throw (uno::RuntimeException)
{
}

sal_Int32 SAL_CALL ScVbaFrame::getBorderStyle() throw (uno::RuntimeException)
{
    return msforms::fmBorderStyle::fmBorderStyleNone;
}

void SAL_CALL ScVbaFrame::setBorderStyle( sal_Int32 /*nBorderStyle*/ ) throw (uno::RuntimeException)
{
}

uno::Reference< msforms::XNewFont > SAL_CALL ScVbaFrame::getFont() throw (uno::RuntimeException)
{
    return new VbaNewFont( this, mxContext, m_xProps );
}

// XFrame methods

uno::Any SAL_CALL ScVbaFrame::Controls( const uno::Any& rIndex ) throw (uno::RuntimeException)
{
    // horizontal anchor of frame children is inside border line (add one unit to compensate border line width)
    double fOffsetX = mpGeometryHelper->getOffsetX() + getLeft() + 1.0;
    // vertical anchor of frame children is inside border line (add half of text height and one unit to compensate border line width)
    double fOffsetY = mpGeometryHelper->getOffsetY() + getTop() + (getFont()->getSize() / 2.0) + 1.0;

	uno::Reference< XCollection > xControls( new ScVbaControls( this, mxContext, mxDialog, m_xModel, fOffsetX, fOffsetY ) );
	if( rIndex.hasValue() )
		return uno::Any( xControls->Item( rIndex, uno::Any() ) );
	return uno::Any( xControls );
}

// XHelperInterface

VBAHELPER_IMPL_XHELPERINTERFACE( ScVbaFrame, "ooo.vba.msforms.Frame" )
