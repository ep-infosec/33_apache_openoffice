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



#ifndef ADC_CORFRAME_HXX
#define ADC_CORFRAME_HXX



// USED SERVICES
	// BASE CLASSES
	// COMPONENTS
	// PARAMETERS

class Html_Image;


namespace display
{


class CorporateFrame
{
  public:
	// LIFECYCLE
	virtual				~CorporateFrame() {}

    // INQUIRY
	virtual DYN Html_Image *
						LogoSrc() const = 0;
	virtual const char *
						LogoLink() const = 0;
	virtual const char *
						CopyrightText() const = 0;

	virtual const char *
	                    CssStyle() const = 0;
	virtual const char *
						CssStylesExplanation() const = 0;
	virtual const char *
						DevelopersGuideHtmlRoot() const = 0;
    virtual bool        SimpleLinks() const = 0;

    // ACCESS
	virtual void        Set_DevelopersGuideHtmlRoot(
	                        const String &      i_directory ) = 0;
	virtual void        Set_SimpleLinks() = 0;
};



// IMPLEMENTATION


}   // namespace display


#endif

