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



#include "ids.hrc"
#include "alreadyopen.hxx"

AlreadyOpenQueryBox::AlreadyOpenQueryBox( Window* pParent, ResMgr* pResMgr, const String& aMessage, sal_Bool bIsStoring ) :
    MessBox(pParent, 0,
            String( ResId( STR_ALREADYOPEN_TITLE, *pResMgr ) ),
            aMessage )
{
    SetImage( QueryBox::GetStandardImage() );

    if ( bIsStoring )
    {
        AddButton( String( ResId( STR_ALREADYOPEN_RETRY_SAVE_BTN, *pResMgr ) ), RET_YES,
                BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
        AddButton( String( ResId( STR_ALREADYOPEN_SAVE_BTN, *pResMgr ) ), RET_NO, 0 );
        AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );
    }
    else
    {
        AddButton( String( ResId( STR_ALREADYOPEN_READONLY_BTN, *pResMgr ) ), RET_YES,
                BUTTONDIALOG_DEFBUTTON | BUTTONDIALOG_OKBUTTON | BUTTONDIALOG_FOCUSBUTTON );
        AddButton( String( ResId( STR_ALREADYOPEN_OPEN_BTN, *pResMgr ) ), RET_NO, 0 );
        AddButton( BUTTON_CANCEL, RET_CANCEL, BUTTONDIALOG_CANCELBUTTON );
    }

    SetButtonHelpText( RET_YES, String::EmptyString() );
    SetButtonHelpText( RET_NO, String::EmptyString() );
}

AlreadyOpenQueryBox::~AlreadyOpenQueryBox()
{
}

