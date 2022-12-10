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


#ifndef _SWWDOCSH_HXX
#define _SWWDOCSH_HXX
#include "swdllapi.h"
#include "docsh.hxx"
#include <swdll.hxx>

class SW_DLLPUBLIC SwWebDocShell: public SwDocShell
{
	sal_uInt16		nSourcePara;	// aktive Zeile in der SourceView

public:

    using SotObject::GetInterface;

	// aber selbst implementieren
	SFX_DECL_INTERFACE(SW_WEBDOCSHELL)
    SFX_DECL_OBJECTFACTORY();
	TYPEINFO();

	SwWebDocShell(SfxObjectCreateMode eMode = SFX_CREATE_MODE_EMBEDDED);
	~SwWebDocShell();

	virtual void		FillClass( SvGlobalName * pClassName,
								   sal_uInt32 * pClipFormat,
								   String * pAppName,
                                   String * pLongUserName,
                                   String * pUserName,
                                   sal_Int32 nFileFormat,
                                   sal_Bool bTemplate = sal_False ) const;
	sal_uInt16 		GetSourcePara()const
							{return nSourcePara;}
    void        SetSourcePara(sal_uInt16 nSet) {nSourcePara = nSet;}
};

#endif


