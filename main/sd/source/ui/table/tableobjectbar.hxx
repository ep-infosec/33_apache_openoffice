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



#ifndef SD_TABLE_OBJECT_BAR_HXX
#define SD_TABLE_OBJECT_BAR_HXX

#include <sfx2/module.hxx>
#include <sfx2/shell.hxx>
#include "glob.hxx"

namespace sd {

class View;
class ViewShell;

}

namespace sd { namespace ui { namespace table {

class TableObjectBar  : public SfxShell
{
public:
    TYPEINFO();
    SFX_DECL_INTERFACE( SD_IF_SDDRAWTABLEOBJECTBAR )

	TableObjectBar( ::sd::ViewShell* pSdViewShell, ::sd::View* pSdView);
    virtual ~TableObjectBar (void);

	void			GetState( SfxItemSet& rSet );
	void			GetAttrState( SfxItemSet& rSet );
	void			Execute( SfxRequest& rReq );

protected:
    ::sd::View* mpView;
	::sd::ViewShell* mpViewSh;
};

} } }

#endif
