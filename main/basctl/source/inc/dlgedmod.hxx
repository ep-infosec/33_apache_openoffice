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




#ifndef _BASCTL_DLGEDMOD_HXX
#define _BASCTL_DLGEDMOD_HXX

#include <svx/svdmodel.hxx>


//============================================================================
// DlgEdModel
//============================================================================

class DlgEdPage;
class Window;
class SfxObjectShell;

class DlgEdModel : public SdrModel
{
	friend class DlgEdPage;

private:
	DlgEdModel( const DlgEdModel& );                // not implemented
	void operator=(const DlgEdModel& rSrcModel);    // not implemented

public:
	TYPEINFO();

    DlgEdModel();
	virtual ~DlgEdModel();

	virtual void DlgEdModelChanged( FASTBOOL bChanged = sal_True );

	virtual SdrPage*  AllocPage(FASTBOOL bMasterPage);

	virtual Window* GetCurDocViewWin();
};

#endif

