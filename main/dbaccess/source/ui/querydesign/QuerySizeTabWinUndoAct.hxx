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


#ifndef DBAUI_QUERYSIZETABWINUNDOACT_HXX
#define DBAUI_QUERYSIZETABWINUNDOACT_HXX

#ifndef DBAUI_QUERYDESIGNUNDOACTION_HXX
#include "QueryDesignUndoAction.hxx"
#endif
#ifndef _DBU_QRY_HRC_
#include "dbu_qry.hrc"
#endif

namespace dbaui
{

	// ================================================================================================
	// OQuerySizeTabWinUndoAct - Undo-Klasse fuer Groessenveraenderung eines TabWins
	class OTableWindow;
	class OJoinSizeTabWinUndoAct : public OQueryDesignUndoAction
	{
		Point			m_ptNextPosition;
		Size			m_szNextSize;
		OTableWindow*	m_pTabWin;

	protected:
		inline void ToggleSizePosition();

	public:
		OJoinSizeTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPos, const Size& szOriginalSize, OTableWindow* pTabWin);
		// Nebenbedingung : es darf nicht gescrollt worden sein, waehrend die neue Groesse/Position ermittelt wurde, das heisst, die Position
		// hier sind physische, nicht logische Koordinaten
		// (im Gegensatz zur QueryMoveTabWinUndoAct)

		virtual void	Undo() { ToggleSizePosition(); }
		virtual void	Redo() { ToggleSizePosition(); }
	};

	//------------------------------------------------------------------------------
	inline OJoinSizeTabWinUndoAct::OJoinSizeTabWinUndoAct(OJoinTableView* pOwner, const Point& ptOriginalPos, const Size& szOriginalSize, OTableWindow* pTabWin)
		:OQueryDesignUndoAction(pOwner, STR_QUERY_UNDO_SIZETABWIN)
		,m_ptNextPosition(ptOriginalPos)
		,m_szNextSize(szOriginalSize)
		,m_pTabWin(pTabWin)
	{
	}

	//------------------------------------------------------------------------------
	inline void OJoinSizeTabWinUndoAct::ToggleSizePosition()
	{
		Point ptNext = m_pTabWin->GetPosPixel();
		Size szNext = m_pTabWin->GetSizePixel();

		m_pOwner->Invalidate(INVALIDATE_NOCHILDREN);
		m_pTabWin->SetPosSizePixel(m_ptNextPosition, m_szNextSize);
		m_pOwner->Invalidate(INVALIDATE_NOCHILDREN);

		m_ptNextPosition = ptNext;
		m_szNextSize = szNext;
	}
}

#endif //DBAUI_QUERYSIZETABWINUNDOACT_HXX


