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


#ifndef _SWANNOTSH_HXX
#define _SWANNOTSH_HXX

#include <sfx2/shell.hxx>
#include "shellid.hxx"
#include "swmodule.hxx"

class SwView;
class SwAnnotationShell: public SfxShell
{
	SwView&		rView;

public:
	SFX_DECL_INTERFACE(SW_ANNOTATIONSHELL)
	TYPEINFO();

				SwAnnotationShell(SwView&);
	virtual		~SwAnnotationShell();

    void        StateDisableItems(SfxItemSet &);
	void		Exec(SfxRequest &);

	void		GetState(SfxItemSet &);
	void		StateInsert(SfxItemSet &rSet);

	void		NoteExec(SfxRequest &);
	void		GetNoteState(SfxItemSet &);

	void		ExecLingu(SfxRequest &rReq);
	void		GetLinguState(SfxItemSet &);

	void		ExecClpbrd(SfxRequest &rReq);
	void		StateClpbrd(SfxItemSet &rSet);

	void 		ExecTransliteration(SfxRequest &);

	void		ExecUndo(SfxRequest &rReq);
	void		StateUndo(SfxItemSet &rSet);

	void		StateStatusLine(SfxItemSet &rSet);

	void		InsertSymbol(SfxRequest& rReq);

	void		ExecSearch(SfxRequest&, sal_Bool bNoMessage = sal_False);
	void            StateSearch(SfxItemSet &);


	virtual ::svl::IUndoManager*
                GetUndoManager();
};

#endif
