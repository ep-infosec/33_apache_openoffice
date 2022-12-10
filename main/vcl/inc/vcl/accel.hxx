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



#ifndef _SV_ACCEL_HXX
#define _SV_ACCEL_HXX

#include <vcl/sv.h>
#include <vcl/dllapi.h>
#include <tools/resid.hxx>
#include <tools/rc.hxx>
#include <vcl/keycod.hxx>

class ImplAccelData;
class ImplAccelEntry;

// ---------------
// - Accelerator -
// ---------------

class VCL_DLLPUBLIC Accelerator : public Resource
{
	friend class ImplAccelManager;

private:
	ImplAccelData*	mpData;
	XubString		maHelpStr;
	Link			maActivateHdl;
	Link			maDeactivateHdl;
	Link			maSelectHdl;

	// Werden vom AcceleratorManager gesetzt
	KeyCode 		maCurKeyCode;
	sal_uInt16			mnCurId;
	sal_uInt16			mnCurRepeat;
	sal_Bool			mbIsCancel;
	sal_Bool*			mpDel;

//#if 0 // _SOLAR__PRIVATE
	SAL_DLLPRIVATE  void        ImplInit();
	SAL_DLLPRIVATE  void        ImplCopyData( ImplAccelData& rAccelData );
	SAL_DLLPRIVATE  void        ImplDeleteData();
	SAL_DLLPRIVATE  void        ImplInsertAccel( sal_uInt16 nItemId, const KeyCode& rKeyCode,
									 sal_Bool bEnable, Accelerator* pAutoAccel );

	SAL_DLLPRIVATE  ImplAccelEntry* ImplGetAccelData( const KeyCode& rKeyCode ) const;
//#endif

protected:
	SAL_DLLPRIVATE  void        ImplLoadRes( const ResId& rResId );

public:
					Accelerator();
					Accelerator( const Accelerator& rAccel );
					Accelerator( const ResId& rResId );
	virtual			~Accelerator();

	virtual void	Activate();
	virtual void	Deactivate();
	virtual void	Select();

	void			InsertItem( sal_uInt16 nItemId, const KeyCode& rKeyCode );
	void			InsertItem( const ResId& rResId );
	void			RemoveItem( sal_uInt16 nItemId );
	void			RemoveItem( const KeyCode rKeyCode );
	void			Clear();

	sal_uInt16			GetCurItemId() const { return mnCurId; }
	const KeyCode&	GetCurKeyCode() const { return maCurKeyCode; }
	sal_uInt16			GetCurRepeat() const { return mnCurRepeat; }
	sal_Bool			IsCancel() const { return mbIsCancel; }

	sal_uInt16			GetItemCount() const;
	sal_uInt16			GetItemId( sal_uInt16 nPos ) const;
	KeyCode 		GetItemKeyCode( sal_uInt16 nPos ) const;
	sal_uInt16			GetItemId( const KeyCode& rKeyCode ) const;
	KeyCode 		GetKeyCode( sal_uInt16 nItemId ) const;
	sal_Bool			IsIdValid( sal_uInt16 nItemId ) const;
	sal_Bool			IsKeyCodeValid( const KeyCode rKeyCode ) const;
	sal_Bool			Call( const KeyCode& rKeyCode, sal_uInt16 nRepeat = 0 );

	void			SetAccel( sal_uInt16 nItemId, Accelerator* pAccel );
	Accelerator*	GetAccel( sal_uInt16 nItemId ) const;
	void			SetAccel( const KeyCode rKeyCode, Accelerator* pAccel );
	Accelerator*	GetAccel( const KeyCode rKeyCode ) const;

	void			EnableItem( sal_uInt16 nItemId, sal_Bool bEnable = sal_True );
	sal_Bool			IsItemEnabled( sal_uInt16 nItemId ) const;
	void			EnableItem( const KeyCode rKeyCode, sal_Bool bEnable = sal_True );
	sal_Bool			IsItemEnabled( const KeyCode rKeyCode ) const;

	void			SetHelpText( const XubString& rHelpText ) { maHelpStr = rHelpText; }
	const XubString& GetHelpText() const { return maHelpStr; }

	void			SetActivateHdl( const Link& rLink ) { maActivateHdl = rLink; }
	const Link& 	GetActivateHdl() const { return maActivateHdl; }
	void			SetDeactivateHdl( const Link& rLink ) { maDeactivateHdl = rLink; }
	const Link& 	GetDeactivateHdl() const { return maDeactivateHdl; }
	void			SetSelectHdl( const Link& rLink ) { maSelectHdl = rLink; }
	const Link& 	GetSelectHdl() const { return maSelectHdl; }

	Accelerator&	operator=( const Accelerator& rAccel );
};

#endif	// _SV_ACCEL_HXX
