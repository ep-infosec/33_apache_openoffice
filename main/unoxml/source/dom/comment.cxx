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



#include <comment.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>


namespace DOM
{
    CComment::CComment(CDocument const& rDocument, ::osl::Mutex const& rMutex,
            xmlNodePtr const pNode)
        : CComment_Base(rDocument, rMutex, NodeType_COMMENT_NODE, pNode)
    {
    }

    void CComment::saxify(
            const Reference< XDocumentHandler >& i_xHandler) {
        if (!i_xHandler.is()) throw RuntimeException();
        Reference< XExtendedDocumentHandler > xExtended(i_xHandler, UNO_QUERY);
        if (xExtended.is()) {
            xExtended->comment(getData());
        }
    }

    OUString SAL_CALL CComment::getNodeName()throw (RuntimeException)
    {
        return OUString::createFromAscii("#comment");
    }

    OUString SAL_CALL CComment::getNodeValue() throw (RuntimeException)
    {
        return CCharacterData::getData();
    }
}
