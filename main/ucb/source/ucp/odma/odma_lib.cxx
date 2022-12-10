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
#include "precompiled_ucb.hxx"
//#ifndef _OSL_PROCESS_H_ 
//#include <osl/process.h>
//#endif
#include <osl/module.h>
#include <rtl/ustring.hxx>
#include "odma_lib.hxx"


namespace odma
{

	TODMRegisterApp			pODMRegisterApp;
	TODMUnRegisterApp		pODMUnRegisterApp;
	TODMSelectDoc			pODMSelectDoc;
	TODMOpenDoc				pODMOpenDoc;
	TODMSaveDoc				pODMSaveDoc;
	TODMCloseDoc			pODMCloseDoc;
	TODMNewDoc				pODMNewDoc;
	TODMSaveAs				pODMSaveAs;
	TODMActivate			pODMActivate;
	TODMGetDocInfo			pODMGetDocInfo;
	TODMSetDocInfo			pODMSetDocInfo;
	TODMGetDMSInfo			pODMGetDMSInfo;
	TODMGetDMSCount			pODMGetDMSCount;
	TODMGetDMSList			pODMGetDMSList;
	TODMGetDMS				pODMGetDMS;
	TODMSetDMS				pODMSetDMS;
	TODMQueryExecute		pODMQueryExecute;
	TODMQueryGetResults		pODMQueryGetResults;
	TODMQueryClose			pODMQueryClose;
	TODMCloseDocEx			pODMCloseDocEx;
	TODMSaveAsEx			pODMSaveAsEx;
	TODMSaveDocEx			pODMSaveDocEx;
	TODMSelectDocEx			pODMSelectDocEx;
	TODMQueryCapability		pODMQueryCapability;
	TODMSetDocEvent			pODMSetDocEvent;
	TODMGetAlternateContent pODMGetAlternateContent;
	TODMSetAlternateContent pODMSetAlternateContent;
	TODMGetDocRelation		pODMGetDocRelation;
	TODMSetDocRelation		pODMSetDocRelation;

	sal_Bool LoadFunctions(oslModule _pODMA);

	sal_Bool LoadLibrary()
	{
		static sal_Bool bLoaded = sal_False;
		static oslModule pODMA = NULL;
		
		if (bLoaded) 
			return sal_True;
		::rtl::OUString sPath;
	#ifdef WNT
		sPath = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMA32.DLL"));
	#endif
	#ifdef UNX
		sPath = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("libodma.so"));
	#endif

		pODMA = osl_loadModule( sPath.pData,SAL_LOADMODULE_NOW );
		if( !pODMA)
			return sal_False;

		return bLoaded = LoadFunctions(pODMA);
	}
	// -------------------------------------------------------------------------

	sal_Bool LoadFunctions(oslModule pODMA)
	{
		if ( ( pODMRegisterApp = (TODMRegisterApp)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMRegisterApp")).pData)) == NULL ) return sal_False;			
		if ( ( pODMUnRegisterApp = (TODMUnRegisterApp)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMUnRegisterApp")).pData)) == NULL ) return sal_False;		
		if ( ( pODMSelectDoc = (TODMSelectDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSelectDoc")).pData)) == NULL ) return sal_False;			
		if ( ( pODMOpenDoc = (TODMOpenDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMOpenDoc")).pData)) == NULL ) return sal_False;				
		if ( ( pODMSaveDoc = (TODMSaveDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveDoc")).pData)) == NULL ) return sal_False;				
		if ( ( pODMCloseDoc = (TODMCloseDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMCloseDoc")).pData)) == NULL ) return sal_False;			
		if ( ( pODMNewDoc = (TODMNewDoc)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMNewDoc")).pData)) == NULL ) return sal_False;				
		if ( ( pODMSaveAs = (TODMSaveAs)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveAs")).pData)) == NULL ) return sal_False;				
		if ( ( pODMActivate = (TODMActivate)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMActivate")).pData)) == NULL ) return sal_False;			
		if ( ( pODMGetDocInfo = (TODMGetDocInfo)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDocInfo")).pData)) == NULL ) return sal_False;			
		if ( ( pODMSetDocInfo = (TODMSetDocInfo)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDocInfo")).pData)) == NULL ) return sal_False;			
		if ( ( pODMGetDMSInfo = (TODMGetDMSInfo)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMSInfo")).pData)) == NULL ) return sal_False;			
		if ( ( pODMGetDMSCount = (TODMGetDMSCount)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMSCount")).pData)) == NULL ) return sal_False;			
		if ( ( pODMGetDMSList = (TODMGetDMSList)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMSList")).pData)) == NULL ) return sal_False;			
		if ( ( pODMGetDMS = (TODMGetDMS)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDMS")).pData)) == NULL ) return sal_False;				
		if ( ( pODMSetDMS = (TODMSetDMS)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDMS")).pData)) == NULL ) return sal_False;				
		if ( ( pODMQueryExecute = (TODMQueryExecute)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryExecute")).pData)) == NULL ) return sal_False;		
		if ( ( pODMQueryGetResults = (TODMQueryGetResults)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryGetResults")).pData)) == NULL ) return sal_False;		
		if ( ( pODMQueryClose = (TODMQueryClose)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryClose")).pData)) == NULL ) return sal_False;			
		if ( ( pODMCloseDocEx = (TODMCloseDocEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMCloseDocEx")).pData)) == NULL ) return sal_False;			
		if ( ( pODMSaveAsEx = (TODMSaveAsEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveAsEx")).pData)) == NULL ) return sal_False;			
		if ( ( pODMSaveDocEx = (TODMSaveDocEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSaveDocEx")).pData)) == NULL ) return sal_False;			
		if ( ( pODMSelectDocEx = (TODMSelectDocEx)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSelectDocEx")).pData)) == NULL ) return sal_False;			
		if ( ( pODMQueryCapability = (TODMQueryCapability)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMQueryCapability")).pData)) == NULL ) return sal_False;		
		if ( ( pODMSetDocEvent = (TODMSetDocEvent)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDocEvent")).pData)) == NULL ) return sal_False;			
		if ( ( pODMGetAlternateContent = (TODMGetAlternateContent)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetAlternateContent")).pData)) == NULL ) return sal_False; 
		if ( ( pODMSetAlternateContent = (TODMSetAlternateContent)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetAlternateContent")).pData)) == NULL ) return sal_False; 
		if ( ( pODMGetDocRelation = (TODMGetDocRelation)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMGetDocRelation")).pData)) == NULL ) return sal_False;		
		if ( ( pODMSetDocRelation = (TODMSetDocRelation)osl_getSymbol(pODMA,::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ODMSetDocRelation")).pData)) == NULL ) return sal_False;		
		return sal_True;
	}
}
