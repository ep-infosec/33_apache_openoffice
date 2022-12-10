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



#ifndef _SD_UNMODPG_HXX
#define _SD_UNMODPG_HXX

#include "sdundo.hxx"
#include "pres.hxx"

class SdDrawDocument;
class SdPage;

class ModifyPageUndoAction : public SdUndoAction
{
	SdPage*         mpPage;
	String          maOldName;
	String          maNewName;
	AutoLayout      meOldAutoLayout;
	AutoLayout      meNewAutoLayout;
	sal_Bool            mbOldBckgrndVisible;
	sal_Bool            mbNewBckgrndVisible;
	sal_Bool            mbOldBckgrndObjsVisible;
	sal_Bool            mbNewBckgrndObjsVisible;

	String          maComment;

public:
	TYPEINFO();
	ModifyPageUndoAction(
		SdDrawDocument*			pTheDoc,
		SdPage*					pThePage,
		String					aTheNewName,
		AutoLayout				eTheNewAutoLayout,
		sal_Bool					bTheNewBckgrndVisible,
		sal_Bool					bTheNewBckgrndObjsVisible);

	virtual ~ModifyPageUndoAction();
	virtual void Undo();
	virtual void Redo();

	virtual String GetComment() const;
};

// --------------------------------------------------------------------

class RenameLayoutTemplateUndoAction : public SdUndoAction
{
public:
	RenameLayoutTemplateUndoAction( SdDrawDocument* pDocument, const String& rOldLayoutName, const String& rNewLayoutName );

	virtual void Undo();
	virtual void Redo();

	virtual String GetComment() const;

private:
	String maOldName;
	String maNewName;
	const String maComment;
};

#endif      // _SD_UNMODPG_HXX

