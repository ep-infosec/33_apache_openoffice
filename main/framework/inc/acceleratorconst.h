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



#ifndef _FRAMEWORK_ACCELERATORCONST_H_
#define _FRAMEWORK_ACCELERATORCONST_H_

//_______________________________________________
// own includes

#include <general.h>

//_______________________________________________
// interface includes

//_______________________________________________
// other includes

namespace framework{
    
#define DOCTYPE_ACCELERATORS            DECLARE_ASCII("<!DOCTYPE accel:acceleratorlist PUBLIC \"-//OpenOffice.org//DTD OfficeDocument 1.0//EN\" \"accelerator.dtd\">")
    
#define ATTRIBUTE_TYPE_CDATA		    DECLARE_ASCII("CDATA")

#define XMLNS_ACCEL                     DECLARE_ASCII("accel")
#define XMLNS_XLINK	                    DECLARE_ASCII("xlink")

#define ATTRIBUTE_URL                   DECLARE_ASCII("href" )
#define ATTRIBUTE_KEYCODE               DECLARE_ASCII("code" )
#define ATTRIBUTE_MOD_SHIFT             DECLARE_ASCII("shift")
#define ATTRIBUTE_MOD_MOD1              DECLARE_ASCII("mod1" )
#define ATTRIBUTE_MOD_MOD2              DECLARE_ASCII("mod2" )
#define ATTRIBUTE_MOD_MOD3              DECLARE_ASCII("mod3" )

// same items with a name space alias
#define AL_ELEMENT_ACCELERATORLIST      DECLARE_ASCII("accel:acceleratorlist")
#define AL_ELEMENT_ITEM				    DECLARE_ASCII("accel:item"           )

#define AL_XMLNS_ACCEL                  DECLARE_ASCII("xmlns:accel")
#define AL_XMLNS_XLINK	                DECLARE_ASCII("xmlns:xlink")

#define AL_ATTRIBUTE_URL                DECLARE_ASCII("xlink:href" )
#define AL_ATTRIBUTE_KEYCODE            DECLARE_ASCII("accel:code" )
#define AL_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("accel:shift")
#define AL_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("accel:mod1" )
#define AL_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("accel:mod2" )
#define AL_ATTRIBUTE_MOD_MOD3           DECLARE_ASCII("accel:mod3" )

// same items with full qualified name space
#define NS_ELEMENT_ACCELERATORLIST      DECLARE_ASCII("http://openoffice.org/2001/accel^acceleratorlist")
#define NS_ELEMENT_ITEM                 DECLARE_ASCII("http://openoffice.org/2001/accel^item"           )
                       
#define NS_XMLNS_ACCEL					DECLARE_ASCII("http://openoffice.org/2001/accel")
#define NS_XMLNS_XLINK					DECLARE_ASCII("http://www.w3.org/1999/xlink"    )

#define NS_ATTRIBUTE_URL                DECLARE_ASCII("http://www.w3.org/1999/xlink^href"     )
#define NS_ATTRIBUTE_KEYCODE            DECLARE_ASCII("http://openoffice.org/2001/accel^code" )
#define NS_ATTRIBUTE_MOD_SHIFT          DECLARE_ASCII("http://openoffice.org/2001/accel^shift")
#define NS_ATTRIBUTE_MOD_MOD1           DECLARE_ASCII("http://openoffice.org/2001/accel^mod1" )
#define NS_ATTRIBUTE_MOD_MOD2           DECLARE_ASCII("http://openoffice.org/2001/accel^mod2" )
#define NS_ATTRIBUTE_MOD_MOD3           DECLARE_ASCII("http://openoffice.org/2001/accel^mod3" )

} // namespace framework

#endif // _FRAMEWORK_ACCELERATORCONST_H_
