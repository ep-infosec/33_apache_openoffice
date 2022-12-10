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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/sdrundomanager.hxx>

//////////////////////////////////////////////////////////////////////////////

SdrUndoManager::SdrUndoManager(sal_uInt16 nMaxUndoActionCount)
:   EditUndoManager(nMaxUndoActionCount),
    maEndTextEditHdl(),
    mpLastUndoActionBeforeTextEdit(0),
    mbEndTextEditTriggeredFromUndo(false)
{
}
    
SdrUndoManager::~SdrUndoManager()
{
}

sal_Bool SdrUndoManager::Undo()
{
    if(isTextEditActive())
    {
        sal_Bool bRetval(sal_False);

        // we are in text edit mode
        if(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction(0))
        {
            // there is an undo action for text edit, trigger it
            bRetval = EditUndoManager::Undo();
        }
        else
        {
            // no more text edit undo, end text edit
            mbEndTextEditTriggeredFromUndo = true;
            maEndTextEditHdl.Call(this);
            mbEndTextEditTriggeredFromUndo = false;
        }

        return bRetval;
    }
    else
    {
        // no undo triggered up to now, trigger local one
        return SfxUndoManager::Undo();
    }
}

sal_Bool SdrUndoManager::Redo()
{
    sal_Bool bRetval(sal_False);

    if(isTextEditActive())
    {
        // we are in text edit mode
        bRetval = EditUndoManager::Redo();
    }

    if(!bRetval)
    {
        // no redo triggered up to now, trigger local one
        bRetval = SfxUndoManager::Redo();
    }

    return bRetval;
}

void SdrUndoManager::Clear()
{
    if(isTextEditActive())
    {
        while(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction(0))
        {
            RemoveLastUndoAction();
        }

        // urgently needed: RemoveLastUndoAction does NOT correct the Redo stack by itself (!)
        ClearRedo();
    }
    else
    {
        // call parent
        EditUndoManager::Clear();
    }
}

void SdrUndoManager::SetEndTextEditHdl(const Link& rLink)
{
    maEndTextEditHdl = rLink; 

    if(isTextEditActive())
    {
        // text edit start, remember last non-textedit action for later cleanup
        mpLastUndoActionBeforeTextEdit = GetUndoActionCount() ? GetUndoAction(0) : 0;
    }
    else
    {
        // text edit ends, pop all textedit actions up to the remembered non-textedit action from the start
        // to set back the UndoManager to the state before text edit started. If that action is already gone 
        // (due to being removed from the undo stack in the meantime), all need to be removed anyways
        while(GetUndoActionCount() && mpLastUndoActionBeforeTextEdit != GetUndoAction(0))
        {
            RemoveLastUndoAction();
        }

        // urgently needed: RemoveLastUndoAction does NOT correct the Redo stack by itself (!)
        ClearRedo();

        // forget marker again
        mpLastUndoActionBeforeTextEdit = 0;
    }
}

bool SdrUndoManager::isTextEditActive() const
{
    return maEndTextEditHdl.IsSet();
}

//////////////////////////////////////////////////////////////////////////////
// eof
