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



#ifndef SD_RULER_HXX
#define SD_RULER_HXX


#include <svx/ruler.hxx>


namespace sd {

class DrawViewShell;
class RulerCtrlItem;
class View;
class Window;

/*************************************************************************
|*
|* das Fenster der Diashow leitet Mouse- und Key-Events an die SlideViewShell
|*
\************************************************************************/

class Ruler 
    : public SvxRuler
{
public:
	Ruler (
        DrawViewShell& rViewSh, 
        ::Window* pParent, 
        ::sd::Window* pWin,
        sal_uInt16 nRulerFlags, 
        SfxBindings& rBindings,
        WinBits nWinStyle);
	virtual ~Ruler (void);

	void SetNullOffset(const Point& rOffset);

	sal_Bool IsHorizontal() const { return bHorz; }

	using ::Ruler::SetNullOffset;
protected:
	::sd::View* pSdView;
	::sd::Window* pSdWin;
	DrawViewShell* pDrViewShell;
	RulerCtrlItem* pCtrlItem;
	sal_Bool bHorz;

	virtual void	MouseButtonDown(const MouseEvent& rMEvt);
	virtual void	MouseButtonUp(const MouseEvent& rMEvt);
	virtual void	MouseMove(const MouseEvent& rMEvt);
	virtual void	Command(const CommandEvent& rCEvt);
	virtual void    ExtraDown();
};

} // end of namespace sd

#endif
