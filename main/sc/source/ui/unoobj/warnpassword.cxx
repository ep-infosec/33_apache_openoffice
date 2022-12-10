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
#include "precompiled_sc.hxx"

// ============================================================================
#include "warnpassword.hxx"
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XInteractionRequest.hpp>
#include <svl/itemset.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/sfxsids.hrc>
#include <ucbhelper/simpleinteractionrequest.hxx>
#include <com/sun/star/task/InteractionClassification.hpp>
#include <com/sun/star/ucb/InteractiveAppException.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <svx/svxerr.hxx>


using ::rtl::OUString;
using ::com::sun::star::uno::makeAny;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Exception;
using ::com::sun::star::uno::XInterface;
using ::com::sun::star::task::InteractionClassification_QUERY;
using ::com::sun::star::task::XInteractionHandler;
using ::com::sun::star::task::XInteractionRequest;
using ::com::sun::star::ucb::InteractiveAppException;

bool ScWarnPassword::WarningOnPassword( SfxMedium& rMedium )
{
    bool bReturn = true;
    Reference< XInteractionHandler > xHandler( rMedium.GetInteractionHandler());
    if( xHandler.is() )
    {

        OUString empty;
        Any xException( makeAny(InteractiveAppException(empty, 
				Reference <XInterface> (), 
				InteractionClassification_QUERY, 
			     ERRCODE_SVX_EXPORT_FILTER_CRYPT)));      

        Reference< ucbhelper::SimpleInteractionRequest > xRequest
                    = new ucbhelper::SimpleInteractionRequest(
                        xException,
			            ucbhelper::CONTINUATION_APPROVE
                            | ucbhelper::CONTINUATION_DISAPPROVE );

        xHandler->handle( xRequest.get() );

        const sal_Int32 nResp = xRequest->getResponse();

        switch ( nResp )
        {
		case ucbhelper::CONTINUATION_UNKNOWN:
                break;

		case ucbhelper::CONTINUATION_APPROVE:
                // Continue 
                break;

		case ucbhelper::CONTINUATION_DISAPPROVE:
                bReturn = false;
                break;
        }
    }
    return bReturn;
}

