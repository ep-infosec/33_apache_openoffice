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

#include "svx/sidebar/PopupControl.hxx"

#include <svtools/valueset.hxx>
#include <boost/function.hpp>
#include <svx/SvxColorValueSet.hxx>

class Window;
class SfxBindings;
class RedId;
class FloatingWindow;


namespace svx { namespace sidebar {

/** The ColorControl uses a ValueSet control for displaying all named
    colors in a matrix.
*/
class SVX_DLLPUBLIC ColorControl
    : public PopupControl
{
public:
    /** Create a new ColorControl object.
        @param rControlResId
            The resource id for the whole color control.
        @param rNoColorGetter
            A functor for getting the color which will be returned when the
            WB_NONEFIELD is used and got selected
        @param rColorSetter
            A functor for setting the color that is selected by the
            user.
        @param pNoColorStringResId
            Resource id of an optional string for the "no color"
            string.  When a value is given then a
            field/button is created above the color matrix for
            selecting "no color" ie. transparent.
            When zero is given then no such field is created.            
    */
	ColorControl (
        Window* pParent,
        SfxBindings* pBindings,
        const ResId& rControlResId,
        const ResId& rValueSetResId,
        const ::boost::function<Color(void)>& rNoColorGetter,
        const ::boost::function<void(String&,Color)>& rColorSetter,
        FloatingWindow* pFloatingWindow,
        const ResId* pNoColorStringResId);
    virtual ~ColorControl (void);
    
	void GetFocus (void);
	void SetCurColorSelect (
        const Color aCol,
        const bool bAvl);

private:	
	SfxBindings* mpBindings;
	SvxColorValueSet maVSColor;
    FloatingWindow* mpFloatingWindow;
    const String msNoColorString;
    ::boost::function<Color(void)> maNoColorGetter;
    ::boost::function<void(String&,Color)> maColorSetter;

	void FillColors (void);
	DECL_LINK(VSSelectHdl, void *);
};

} } // end of namespace svx::sidebar
