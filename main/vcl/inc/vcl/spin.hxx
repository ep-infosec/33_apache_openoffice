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



#ifndef _SV_SPIN_HXX
#define _SV_SPIN_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/timer.hxx>

// --------------
// - SpinButton -
// --------------

class VCL_DLLPUBLIC SpinButton : public Control
{
private:
    AutoTimer       maRepeatTimer;
    Rectangle       maUpperRect;
    Rectangle       maLowerRect;
    Rectangle       maFocusRect;
    sal_Bool            mbRepeat         : 1;
    sal_Bool            mbUpperIn        : 1;
    sal_Bool            mbLowerIn        : 1;
    sal_Bool            mbInitialUp      : 1;
    sal_Bool            mbInitialDown    : 1;
    sal_Bool            mbHorz           : 1;
    sal_Bool            mbUpperIsFocused : 1;
    Link            maUpHdlLink;
    Link            maDownHdlLink;
    long            mnMinRange;
    long            mnMaxRange;
    long            mnValue;
    long            mnValueStep;

    SAL_DLLPRIVATE Rectangle* ImplFindPartRect( const Point& rPt );
    using Window::ImplInit;
    SAL_DLLPRIVATE void       ImplInit( Window* pParent, WinBits nStyle );
    DECL_DLLPRIVATE_LINK(     ImplTimeout, Timer* );

public:
    explicit        SpinButton( Window* pParent, WinBits nStyle = 0 );
    explicit        SpinButton( Window* pParent, const ResId& );
    virtual         ~SpinButton();

    virtual void    Up();
    virtual void    Down();

    virtual void    Resize();
    virtual void    Paint( const Rectangle& rRect );
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, const Size& rSize, sal_uLong nFlags );
    virtual void    MouseButtonDown( const MouseEvent& rMEvt );
    virtual void    MouseButtonUp( const MouseEvent& rMEvt );
    virtual void    MouseMove( const MouseEvent& rMEvt );
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    StateChanged( StateChangedType nStateChange );
    virtual void    GetFocus();
    virtual void    LoseFocus();

    void            SetRangeMin( long nNewRange );
    long            GetRangeMin() const { return mnMinRange; }
    void            SetRangeMax( long nNewRange );
    long            GetRangeMax() const { return mnMaxRange; }
    void            SetRange( const Range& rRange );
    Range           GetRange() const { return Range( GetRangeMin(), GetRangeMax() ); }
    void            SetValue( long nValue );
    long            GetValue() const { return mnValue; }
    void            SetValueStep( long nNewStep ) { mnValueStep = nNewStep; }
    long            GetValueStep() const { return mnValueStep; }
    virtual long    PreNotify( NotifyEvent& rNEvt );

    void            SetUpHdl( const Link& rLink ) { maUpHdlLink = rLink; }
    const Link&     GetUpHdl() const   { return maUpHdlLink;   }
    void            SetDownHdl( const Link& rLink ) { maDownHdlLink = rLink; }
    const Link&     GetDownHdl() const { return maDownHdlLink; }

private:
    // moves the focus to the upper or lower rect. Return sal_True if the focus rect actually changed.
    SAL_DLLPRIVATE sal_Bool        ImplMoveFocus( sal_Bool _bUpper );
    SAL_DLLPRIVATE void        ImplCalcFocusRect( sal_Bool _bUpper );

    SAL_DLLPRIVATE inline sal_Bool ImplIsUpperEnabled( ) const
    {
        return mnValue + mnValueStep <= mnMaxRange;
    }
    SAL_DLLPRIVATE inline sal_Bool ImplIsLowerEnabled( ) const
    {
        return mnValue >= mnMinRange + mnValueStep;
    }
};

#endif  // _SV_SPIN_HXX

