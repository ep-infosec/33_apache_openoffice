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



#ifndef _OUTLUNDO_HXX
#define _OUTLUNDO_HXX

#include <editeng/outliner.hxx>
#include <editeng/editdata.hxx>
#include <editeng/editund2.hxx>

class OutlinerUndoBase : public EditUndo
{
private:
	Outliner*	mpOutliner;

public:
				OutlinerUndoBase( sal_uInt16 nId, Outliner* pOutliner );

	Outliner*	GetOutliner() const { return mpOutliner; }
};

class OutlinerUndoChangeParaFlags : public OutlinerUndoBase
{
private:
	sal_uInt16		mnPara;
	sal_uInt16		mnOldFlags;
	sal_uInt16		mnNewFlags;
	
	void ImplChangeFlags( sal_uInt16 nFlags );

public:
	OutlinerUndoChangeParaFlags( Outliner* pOutliner, sal_uInt16 nPara, sal_uInt16 nOldDepth, sal_uInt16 nNewDepth );

	virtual void	Undo();
	virtual void	Redo();
};

class OutlinerUndoChangeParaNumberingRestart : public OutlinerUndoBase
{
private:
    sal_uInt16		mnPara;
    
    struct ParaRestartData
    {
        sal_Int16       mnNumberingStartValue;
        sal_Bool        mbParaIsNumberingRestart;
    };

    ParaRestartData maUndoData;
    ParaRestartData maRedoData;

    void ImplApplyData( const ParaRestartData& rData );
public:
    OutlinerUndoChangeParaNumberingRestart( Outliner* pOutliner, sal_uInt16 nPara,
        sal_Int16 nOldNumberingStartValue, sal_Int16 mnNewNumberingStartValue,
        sal_Bool  nOldbParaIsNumberingRestart, sal_Bool nbNewParaIsNumberingRestart );

    virtual void	Undo();
	virtual void	Redo();
};
    
class OutlinerUndoChangeDepth : public OutlinerUndoBase
{
	using SfxUndoAction::Repeat;
private:
	sal_uInt16 			mnPara;
	sal_Int16		mnOldDepth;
	sal_Int16		mnNewDepth;

public:
					OutlinerUndoChangeDepth( Outliner* pOutliner, sal_uInt16 nPara, sal_Int16 nOldDepth, sal_Int16 nNewDepth );

	virtual void	Undo();
	virtual void	Redo();
	virtual void 	Repeat();
};

// Hilfs-Undo: Wenn es fuer eine Aktion keine OutlinerUndoAction gibst, weil
// die EditEngine das handelt, aber z.B. noch das Bullet neu berechnet werden muss.

class OutlinerUndoCheckPara : public OutlinerUndoBase
{
	using SfxUndoAction::Repeat;
private:
	sal_uInt16 			mnPara;

public:
					OutlinerUndoCheckPara( Outliner* pOutliner, sal_uInt16 nPara );

	virtual void	Undo();
	virtual void	Redo();
	virtual void 	Repeat();
};



// -------------------------------------


class OLUndoExpand : public EditUndo
{
	using SfxUndoAction::Repeat;
	void Restore( sal_Bool bUndo );
public:
	OLUndoExpand( Outliner* pOut, sal_uInt16 nId );
	~OLUndoExpand();
	virtual void Undo();
	virtual void Redo();
	virtual void Repeat();

	sal_uInt16* pParas;  // 0 == nCount enthaelt Absatznummer
	Outliner* pOutliner;
	sal_uInt16 nCount;
};

#endif
