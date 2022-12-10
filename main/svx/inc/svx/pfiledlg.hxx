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


#ifndef _SVX_PFILEDLG_HXX
#define _SVX_PFILEDLG_HXX

// include ---------------------------------------------------------------

#include <sfx2/filedlghelper.hxx>
#include "svx/svxdllapi.h"

/*************************************************************************
|*
|* Filedialog to insert Plugin-Fileformats
|*
\************************************************************************/

class SVX_DLLPUBLIC SvxPluginFileDlg
{
private:
	sfx2::FileDialogHelper			maFileDlg;

public:
	// with nKind = SID_INSERT_SOUND    or
	//      nKind = SID_INSERT_VIDEO
	SvxPluginFileDlg (Window *pParent, sal_uInt16 nKind );
	~SvxPluginFileDlg ();

	ErrCode					 Execute();
	String					 GetPath() const;

	static bool IsAvailable (sal_uInt16 nKind);

	void					SetContext( sfx2::FileDialogHelper::Context eNewContext );
};

#endif // _SVX_PFILEDLG_HXX


