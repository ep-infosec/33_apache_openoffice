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



#ifndef SC_OPTUTIL_HXX
#define SC_OPTUTIL_HXX

#include <unotools/configitem.hxx>
#include <tools/link.hxx>
#include "scdllapi.h"


class ScOptionsUtil
{
public:
	static sal_Bool		IsMetricSystem();
};


//	ConfigItem for classes that use items from several sub trees

class SC_DLLPUBLIC ScLinkConfigItem : public utl::ConfigItem
{
	Link	aCommitLink;

public:
            ScLinkConfigItem( const rtl::OUString& rSubTree );
            ScLinkConfigItem( const rtl::OUString& rSubTree, sal_Int16 nMode );
	void	SetCommitLink( const Link& rLink );

	virtual void	Notify( const com::sun::star::uno::Sequence<rtl::OUString>& aPropertyNames );
	virtual void	Commit();

	void 	SetModified()	{ ConfigItem::SetModified(); }
	com::sun::star::uno::Sequence< com::sun::star::uno::Any>
			GetProperties(const com::sun::star::uno::Sequence< rtl::OUString >& rNames)
							{ return ConfigItem::GetProperties( rNames ); }
	sal_Bool PutProperties( const com::sun::star::uno::Sequence< rtl::OUString >& rNames,
							const com::sun::star::uno::Sequence< com::sun::star::uno::Any>& rValues)
							{ return ConfigItem::PutProperties( rNames, rValues ); }

    using ConfigItem::EnableNotification;
    using ConfigItem::GetNodeNames;

//	sal_Bool EnableNotification(com::sun::star::uno::Sequence< rtl::OUString >& rNames)
//							{ return ConfigItem::EnableNotification( rNames ); }

//	com::sun::star::uno::Sequence< rtl::OUString > GetNodeNames(rtl::OUString& rNode)
//							{ return ConfigItem::GetNodeNames( rNode ); }
};

#endif


