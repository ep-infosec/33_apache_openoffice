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



#ifndef INCLUDED_SLIDESHOW_RGBCOLOR_HXX
#define INCLUDED_SLIDESHOW_RGBCOLOR_HXX

#include <cppcanvas/color.hxx>


/* Definition of RGBColor class */

namespace slideshow
{
    namespace internal    
    {
        class HSLColor;

        /** RGB color space class.        
         */
        class RGBColor
        {
        public:
            RGBColor();
            explicit RGBColor( ::cppcanvas::Color::IntSRGBA nRGBColor );
            RGBColor( double nRed, double nGreen, double nBlue );
            explicit RGBColor( const HSLColor& rColor );

            /** Hue of the color.
                
            	@return hue, is in the range [0,360]
             */
            double getHue() const;

            /** Saturation of the color.
                
            	@return saturation, is in the range [0,1]
             */
            double getSaturation() const;

            /** Luminance of the color.
                
            	@return luminance, is in the range [0,1]
             */
            double getLuminance() const;

            /** Get the RGB red value.
             */
            double getRed() const;

            /** Get the RGB green value.
             */
            double getGreen() const;

            /** Get the RGB blue value.
             */
            double getBlue() const;

            /** Create an HSL color object.
             */
            HSLColor getHSLColor() const;

            /** Create an integer sRGBA color.
             */
            ::cppcanvas::Color::IntSRGBA getIntegerColor() const;
 
            RGBColor(const RGBColor& rLHS);
            RGBColor& operator=( const RGBColor& rLHS); 

            struct RGBTriple
            {
                RGBTriple();
                RGBTriple( double nRed, double nGreen, double nBlue );

                double mnRed;
                double mnGreen;
                double mnBlue;
            };

        private:
            // default copy/assignment are okay
            // RGBColor(const RGBColor&);            
            // RGBColor& operator=( const RGBColor& );

            RGBTriple	maRGBTriple;
        };        

        RGBColor operator+( const RGBColor& rLHS, const RGBColor& rRHS );
        RGBColor operator*( const RGBColor& rLHS, const RGBColor& rRHS );
        RGBColor operator*( double nFactor, const RGBColor& rRHS );
        

        /** RGB color linear interpolator.
            
            @param t
            As usual, t must be in the [0,1] range
        */
        RGBColor interpolate( const RGBColor& rFrom, const RGBColor& rTo, double t );
    }
}

#endif /* INCLUDED_SLIDESHOW_RGBCOLOR_HXX */
