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



#ifndef _SFX2_LAYOUT_PRE_HXX
#define _SFX2_LAYOUT_PRE_HXX

#include <layout/layout-pre.hxx>

#if ENABLE_LAYOUT

///* Allow re-inclusion for next hxx file. */
#undef _SFX2_LAYOUT_POST_HXX

#undef SfxDialog
#undef SfxModalDialog
#undef SfxModelessDialog

#define SfxDialog layout::SfxDialog

#define SfxModalDialog SfxDialog
#define SfxModelessDialog SfxDialog

#endif /* ENABLE_LAYOUT */

#endif /* _SFX2_LAYOUT_PRE_HXX */
