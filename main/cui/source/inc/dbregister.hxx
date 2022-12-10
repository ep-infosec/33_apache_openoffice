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



#ifndef SVX_DBREGISTER_HXX
#define SVX_DBREGISTER_HXX

#include "optpath.hxx"
#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>
#include <svl/poolitem.hxx>
#include "ControlFocusHelper.hxx"
#include <sfx2/basedlgs.hxx>

class SvLBoxEntry;
namespace svx
{
	class OptHeaderTabListBox;
}
//........................................................................
namespace svx
{
//........................................................................

    //====================================================================
	//= DbRegistrationOptionsPage
	//====================================================================
	class DbRegistrationOptionsPage : public SfxTabPage
	{
	private:
		FixedLine           aStdBox;
		FixedText			aTypeText;
		FixedText			aPathText;
		SvxControlFocusHelper	aPathCtrl;
		PushButton			m_aNew;
		PushButton			m_aEdit;
		PushButton      	m_aDelete;

		HeaderBar*			pHeaderBar;
		::svx::OptHeaderTabListBox*	pPathBox;
		SvLBoxEntry*		m_pCurEntry;
		sal_uLong				m_nOldCount;
		sal_Bool				m_bModified;

#ifdef SVX_DBREGISTER_HXX
		DECL_LINK( NewHdl, void * );
		DECL_LINK( EditHdl, void * );
		DECL_LINK( DeleteHdl, void * );

		DECL_LINK( PathSelect_Impl, SvTabListBox* );

		DECL_LINK( HeaderSelect_Impl, HeaderBar * );
		DECL_LINK( HeaderEndDrag_Impl, HeaderBar * );
		DECL_LINK( NameValidator, String*);


		/** inserts a new entry in the tablistbox
			@param	_sName
				The name of the entry.
			@param	_sLocation
				The location of the file.
		*/
		void insertNewEntry( const ::rtl::OUString& _sName,const ::rtl::OUString& _sLocation, const bool bReadOnly );

		/** opens the LinkDialog to create a register pair
			@param	_sOldName
				The old name of the entry may be empty.
			@param	_sOldLocation
				The old location of the entry may be empty.
			@param	_pEntry
				The entry to remove if the entry will be changed
		*/
		void openLinkDialog(const String& _sOldName,const String& _sOldLocation,SvLBoxEntry* _pEntry = NULL);

#endif

	public:
		DbRegistrationOptionsPage( Window* pParent, const SfxItemSet& rSet );
		virtual ~DbRegistrationOptionsPage();

		static SfxTabPage*	Create( Window* pParent, const SfxItemSet& rSet );
		static sal_uInt16*		GetRanges();

		virtual	sal_Bool 		FillItemSet( SfxItemSet& rSet );
		virtual	void 		Reset( const SfxItemSet& rSet );
		virtual void        FillUserData();
	};

    //====================================================================
	//= RegistrationItemSetHolder
	//====================================================================
    /** helper for DatabaseRegistrationDialog

        Necessary so that DatabaseRegistrationDialog is self-contained, i.e. always reflects
        the current registration state.
    */
    class RegistrationItemSetHolder
    {
    private:
        SfxItemSet  m_aRegistrationItems;

    protected:
        RegistrationItemSetHolder( const SfxItemSet& _rMasterSet );
        ~RegistrationItemSetHolder();

    protected:
        const SfxItemSet& getRegistrationItems() const { return m_aRegistrationItems; }
    };

    //====================================================================
	//= DatabaseRegistrationDialog
	//====================================================================
    class DatabaseRegistrationDialog    :public RegistrationItemSetHolder
                                        ,public SfxSingleTabDialog
    {
    public:
	    DatabaseRegistrationDialog( Window* pParent, const SfxItemSet& rAttr );
	    ~DatabaseRegistrationDialog();

        virtual short	Execute();
    };

//........................................................................
}	// namespace svx
//........................................................................

#endif // SVX_DBREGISTER_HXX


