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



#ifndef _PROPIMP0_HXX
#define _PROPIMP0_HXX

#include <xmloff/xmlprhdl.hxx>

//////////////////////////////////////////////////////////////////////////////
// graphic property Stroke

class XMLDurationPropertyHdl : public XMLPropertyHandler
{
public:
	virtual ~XMLDurationPropertyHdl();
	virtual sal_Bool importXML( const rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
	virtual sal_Bool exportXML( rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class SvXMLImport;
class XMLOpacityPropertyHdl : public XMLPropertyHandler
{
private:
	SvXMLImport*	mpImport;
public:
	XMLOpacityPropertyHdl( SvXMLImport* pImport );
	virtual ~XMLOpacityPropertyHdl();
	virtual sal_Bool importXML( const rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
	virtual sal_Bool exportXML( rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class XMLTextAnimationStepPropertyHdl : public XMLPropertyHandler
{
public:
	virtual ~XMLTextAnimationStepPropertyHdl();
	virtual sal_Bool importXML( const rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
	virtual sal_Bool exportXML( rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};

class SvXMLExport;
class XMLDateTimeFormatHdl : public XMLPropertyHandler
{
private:
	SvXMLExport* mpExport;

public:
	XMLDateTimeFormatHdl( SvXMLExport* pExport );
	virtual ~XMLDateTimeFormatHdl();
	virtual sal_Bool importXML( const rtl::OUString& rStrImpValue, ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
	virtual sal_Bool exportXML( rtl::OUString& rStrExpValue, const ::com::sun::star::uno::Any& rValue, const SvXMLUnitConverter& rUnitConverter ) const;
};


#endif	//  _PROPIMP0_HXX
