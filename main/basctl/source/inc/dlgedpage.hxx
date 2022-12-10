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



#ifndef _BASCTL_DLGEDPAGE_HXX
#define _BASCTL_DLGEDPAGE_HXX

#include "svx/svdpage.hxx"

//============================================================================
// DlgEdPage
//============================================================================

class DlgEdModel;
class DlgEdForm;

class DlgEdPage : public SdrPage
{
private:
	DlgEdForm*		pDlgEdForm;

public:
	TYPEINFO();

	DlgEdPage( DlgEdModel& rModel, FASTBOOL bMasterPage=sal_False );
	DlgEdPage( const DlgEdPage& );
	virtual ~DlgEdPage();

	using SdrPage::Clone;
	virtual SdrPage* Clone() const;

	void			SetDlgEdForm( DlgEdForm* pForm ) { pDlgEdForm = pForm; }
	DlgEdForm*		GetDlgEdForm() const { return pDlgEdForm; }

	virtual SdrObject* SetObjectOrdNum(sal_uLong nOldObjNum, sal_uLong nNewObjNum);
};

#endif //_BASCTL_DLGEDPAGE_HXX
