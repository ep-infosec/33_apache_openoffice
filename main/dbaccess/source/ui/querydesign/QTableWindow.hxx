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


#ifndef DBAUI_QUERY_TABLEWINDOW_HXX
#define DBAUI_QUERY_TABLEWINDOW_HXX

#ifndef DBAUI_TABLEWINDOW_HXX
#include "TableWindow.hxx"
#endif
#ifndef DBAUI_QUERY_TABLEWINDOWDATA_HXX
#include "QTableWindowData.hxx"
#endif
#ifndef DBAUI_TABLEFIELDDESC_HXX
#include "TableFieldDescription.hxx"
#endif
#ifndef _RTTI_HXX
#include <tools/rtti.hxx>
#endif

namespace dbaui
{
	//==================================================================
	class OQueryTableWindow : public OTableWindow
	{
		sal_Int32			m_nAliasNum;
		::rtl::OUString		m_strInitialAlias;
	public:
		OQueryTableWindow( Window* pParent, const TTableWindowData::value_type& pTabWinData, sal_Unicode* pszInitialAlias = NULL );
		virtual ~OQueryTableWindow();

		::rtl::OUString	GetAliasName() const 
		{ 
			return static_cast<OQueryTableWindowData*>(GetData().get())->GetAliasName(); 
		}
		void			SetAliasName(const ::rtl::OUString& strNewAlias) 
		{ 
			static_cast<OQueryTableWindowData*>(GetData().get())->SetAliasName(strNewAlias); 
		}

		// spaeter Constructor, die Basisklasse ERZEUGT beim ersten Aufruf die Listbox
		virtual sal_Bool	Init();

		inline sal_Int32		GetAliasNum() const	{ return m_nAliasNum; }

		sal_Bool			ExistsField(const ::rtl::OUString& strFieldName, OTableFieldDescRef& rInfo);
		sal_Bool			ExistsAVisitedConn() const;

		virtual ::rtl::OUString		GetName() const { return GetWinName(); }

	protected:
		virtual void	KeyInput( const KeyEvent& rEvt );

		virtual void	OnEntryDoubleClicked(SvLBoxEntry* pEntry);
			// wird aus dem DoubleClickHdl der ListBox heraus aufgerufen
		/** delete the user data with the equal type as created within createUserData
			@param	_pUserData
				The user data store in the listbox entries. Created with a call to createUserData.
				_pUserData may be <NULL/>.
		*/
		virtual void deleteUserData(void*& _pUserData);

		/** creates user information that will be append at the ListBoxentry
			@param	_xColumn
				The corresponding column, can be <NULL/>.
			@param	_bPrimaryKey
				<TRUE/> when the column belongs to the primary key
			@return
				the user data which will be append at the listbox entry, may be <NULL/>
		*/
		virtual void* createUserData(const ::com::sun::star::uno::Reference< 
									::com::sun::star::beans::XPropertySet>& _xColumn,
									bool _bPrimaryKey);
	};
}
#endif // DBAUI_QUERY_TABLEWINDOW_HXX


