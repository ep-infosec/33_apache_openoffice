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


#ifndef _SWWVIEW_HXX
#define _SWWVIEW_HXX

#include "swdllapi.h"
#include "view.hxx"

/*--------------------------------------------------------------------
	Beschreibung:
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwWebView: public SwView
{
protected:
	virtual void	SelectShell();
public:

	SFX_DECL_VIEWFACTORY(SwWebView);
	SFX_DECL_INTERFACE(SW_WEBVIEWSHELL)
	TYPEINFO();

	SwWebView(SfxViewFrame* pFrame, SfxViewShell*);
	~SwWebView();
};

#endif
