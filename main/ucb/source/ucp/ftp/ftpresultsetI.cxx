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
#include "precompiled_ftp.hxx"

#include "ucbhelper/propertyvalueset.hxx"
#include "rtl/ref.hxx"
#include "com/sun/star/ucb/Command.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/ucb/XCommandProcessor.hpp"
#include "com/sun/star/sdbc/XRow.hpp"
#include "ftpresultsetI.hxx"
#include "ftpcontent.hxx"


using namespace std;
using namespace ftp;
using namespace com::sun::star::ucb;
using namespace com::sun::star::lang;
using namespace com::sun::star::uno;
using namespace com::sun::star::beans;
using namespace com::sun::star::sdbc;


ResultSetI::ResultSetI(const Reference<XMultiServiceFactory>&  xMSF,
                       const Reference<XContentProvider>&  xProvider,
                       sal_Int32 nOpenMode,
                       const Sequence<Property>& seqProp,
                       const Sequence< NumberedSortingInfo >& seqSort,
                       const std::vector<FTPDirentry>&  dirvec)
    : ResultSetBase(xMSF,xProvider,nOpenMode,seqProp,seqSort)
{
    for( unsigned int i = 0; i < dirvec.size(); ++i)
        m_aPath.push_back(dirvec[i].m_aURL);

    // m_aIdents holds the contentidentifiers

    m_aItems.resize( m_aPath.size() );
    m_aIdents.resize( m_aPath.size() );

    for(unsigned n = 0; n < m_aItems.size(); ++n) {
        rtl::Reference<ucbhelper::PropertyValueSet> xRow =
            new ucbhelper::PropertyValueSet(xMSF);

        for( int i = 0; i < seqProp.getLength(); ++i) {
            const rtl::OUString& Name = seqProp[i].Name;
            if(Name.compareToAscii("ContentType") == 0 )
                xRow->appendString(seqProp[i],
                                   rtl::OUString::createFromAscii(
                                       "application/ftp" ));
            else if(Name.compareToAscii("Title") == 0)
                xRow->appendString(seqProp[i],dirvec[n].m_aName);
            else if(Name.compareToAscii("IsReadOnly") == 0)
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_WRITE));
            else if(Name.compareToAscii("IsDocument") == 0)
                xRow->appendBoolean(seqProp[i],
                                    ! sal_Bool(dirvec[n].m_nMode &
                                               INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.compareToAscii("IsFolder") == 0)
                xRow->appendBoolean(seqProp[i],
                                    sal_Bool(dirvec[n].m_nMode &
                                             INETCOREFTP_FILEMODE_ISDIR));
            else if(Name.compareToAscii("Size") == 0)
                xRow->appendLong(seqProp[i],
                                 dirvec[n].m_nSize);
            else if(Name.compareToAscii("DateCreated") == 0)
                xRow->appendTimestamp(seqProp[i],
                                      dirvec[n].m_aDate);
            else if(Name.compareToAscii("CreatableContentsInfo") == 0)
                xRow->appendObject(
                    seqProp[i],
                    makeAny(FTPContent::queryCreatableContentsInfo_Static()));
            else
                xRow->appendVoid(seqProp[i]);
        }
        m_aItems[n] = Reference<XRow>(xRow.get());
    }
}
