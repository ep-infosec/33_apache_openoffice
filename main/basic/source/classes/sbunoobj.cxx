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
#include "precompiled_basic.hxx"
//#include <stl_queue.h>
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#ifndef _TOOLERR_HXX //autogen
#include <tools/errcode.hxx>
#endif
#include <svl/hint.hxx>

#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <cppuhelper/typeprovider.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <comphelper/extract.hxx>
#include <comphelper/processfactory.hxx>

#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>

#include <com/sun/star/script/ArrayWrapper.hpp>
#include <com/sun/star/script/NativeObjectWrapper.hpp>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/DeploymentException.hpp>
#include <com/sun/star/lang/XTypeProvider.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/PropertyConcept.hpp>
#include <com/sun/star/beans/MethodConcept.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/script/BasicErrorException.hpp>
#include <com/sun/star/script/XAllListener.hpp>
#include <com/sun/star/script/XInvocationAdapterFactory.hpp>
#include <com/sun/star/script/XTypeConverter.hpp>
#include <com/sun/star/script/XDefaultProperty.hpp>
#include <com/sun/star/script/XDirectInvocation.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/reflection/XIdlArray.hpp>
#include <com/sun/star/reflection/XIdlReflection.hpp>
#include <com/sun/star/reflection/XIdlClassProvider.hpp>
#include <com/sun/star/reflection/XServiceConstructorDescription.hpp>
#include <com/sun/star/bridge/oleautomation/NamedArgument.hpp>
#include <com/sun/star/bridge/oleautomation/Date.hpp>
#include <com/sun/star/bridge/oleautomation/Decimal.hpp>
#include <com/sun/star/bridge/oleautomation/Currency.hpp>
#include <com/sun/star/bridge/oleautomation/XAutomationObject.hpp>


using com::sun::star::uno::Reference;
using namespace com::sun::star::uno;
using namespace com::sun::star::lang;
using namespace com::sun::star::reflection;
using namespace com::sun::star::beans;
using namespace com::sun::star::script;
using namespace com::sun::star::container;
using namespace com::sun::star::bridge;
using namespace cppu;


#include<basic/sbstar.hxx>
#include<basic/sbuno.hxx>
#include<basic/sberrors.hxx>
#include<sbunoobj.hxx>
#include"sbjsmod.hxx"
#include<basic/basmgr.hxx>
#include<sbintern.hxx>
#include<runtime.hxx>

#include<math.h>
#include <hash_map>
#include <com/sun/star/reflection/XTypeDescriptionEnumerationAccess.hpp>
#include <com/sun/star/reflection/XConstantsTypeDescription.hpp>

TYPEINIT1(SbUnoMethod,SbxMethod)
TYPEINIT1(SbUnoProperty,SbxProperty)
TYPEINIT1(SbUnoObject,SbxObject)
TYPEINIT1(SbUnoClass,SbxObject)
TYPEINIT1(SbUnoService,SbxObject)
TYPEINIT1(SbUnoServiceCtor,SbxMethod)
TYPEINIT1(SbUnoSingleton,SbxObject)

typedef WeakImplHelper1< XAllListener > BasicAllListenerHelper;

// Flag, um immer ueber Invocation zu gehen
//#define INVOCATION_ONLY


// Identifier fuer die dbg_-Properies als Strings anlegen
static char const ID_DBG_SUPPORTEDINTERFACES[] = "Dbg_SupportedInterfaces";
static char const ID_DBG_PROPERTIES[] = "Dbg_Properties";
static char const ID_DBG_METHODS[] = "Dbg_Methods";

static ::rtl::OUString aSeqLevelStr( RTL_CONSTASCII_USTRINGPARAM("[]") );
static ::rtl::OUString defaultNameSpace( RTL_CONSTASCII_USTRINGPARAM("ooo.vba") );

// Gets the default property for an uno object. Note: There is some
// redirection built in. The property name specifies the name
// of the default property.

bool SbUnoObject::getDefaultPropName( SbUnoObject* pUnoObj, String& sDfltProp )
{
	bool result = false;
	Reference< XDefaultProperty> xDefaultProp( pUnoObj->maTmpUnoObj, UNO_QUERY );
	if ( xDefaultProp.is() )
	{
		sDfltProp = xDefaultProp->getDefaultPropertyName();
		if ( sDfltProp.Len() )
			result = true;
	}
	return result;
}

SbxVariable* getDefaultProp( SbxVariable* pRef )
{
	SbxVariable* pDefaultProp = NULL;
	if ( pRef->GetType() == SbxOBJECT )
	{
  		SbxObject* pObj = PTR_CAST(SbxObject,(SbxVariable*) pRef);
		if ( !pObj )
		{
			SbxBase* pObjVarObj = pRef->GetObject();
			pObj = PTR_CAST(SbxObject,pObjVarObj);
		}
		if ( pObj && pObj->ISA(SbUnoObject) )
		{
			SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)pObj);
			pDefaultProp = pUnoObj->GetDfltProperty();
		}
	}
	return pDefaultProp;
}

Reference< XComponentContext > getComponentContext_Impl( void )
{
    static Reference< XComponentContext > xContext;

	// Haben wir schon CoreReflection, sonst besorgen
	if( !xContext.is() )
	{
		Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
        Reference< XPropertySet > xProps( xFactory, UNO_QUERY );
        OSL_ASSERT( xProps.is() );
        if (xProps.is())
        {
            xProps->getPropertyValue(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) ) >>= xContext;
            OSL_ASSERT( xContext.is() );
        }
	}
	return xContext;
}

// CoreReflection statisch speichern
Reference< XIdlReflection > getCoreReflection_Impl( void )
{
	static Reference< XIdlReflection > xCoreReflection;

	// Haben wir schon CoreReflection, sonst besorgen
	if( !xCoreReflection.is() )
	{
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            xContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection") ) )
                    >>= xCoreReflection;
            OSL_ENSURE( xCoreReflection.is(), "### CoreReflection singleton not accessible!?" );
        }
        if( !xCoreReflection.is() )
        {
            throw DeploymentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theCoreReflection singleton not accessible") ),
                Reference< XInterface >() );
        }
	}
	return xCoreReflection;
}

// CoreReflection statisch speichern
Reference< XHierarchicalNameAccess > getCoreReflection_HierarchicalNameAccess_Impl( void )
{
	static Reference< XHierarchicalNameAccess > xCoreReflection_HierarchicalNameAccess;

	if( !xCoreReflection_HierarchicalNameAccess.is() )
	{
		Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
		if( xCoreReflection.is() )
		{
			xCoreReflection_HierarchicalNameAccess =
				Reference< XHierarchicalNameAccess >( xCoreReflection, UNO_QUERY );
		}
	}
	return xCoreReflection_HierarchicalNameAccess;
}

// Hold TypeProvider statically
Reference< XHierarchicalNameAccess > getTypeProvider_Impl( void )
{
	static Reference< XHierarchicalNameAccess > xAccess;

	// Haben wir schon CoreReflection, sonst besorgen
	if( !xAccess.is() )
	{
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            xContext->getValueByName(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("/singletons/com.sun.star.reflection.theTypeDescriptionManager") ) )
                    >>= xAccess;
            OSL_ENSURE( xAccess.is(), "### TypeDescriptionManager singleton not accessible!?" );
        }
        if( !xAccess.is() )
        {
            throw DeploymentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM
                    ("/singletons/com.sun.star.reflection.theTypeDescriptionManager singleton not accessible") ),
                Reference< XInterface >() );
        }
	}
	return xAccess;
}

// Hold TypeConverter statically
Reference< XTypeConverter > getTypeConverter_Impl( void )
{
	static Reference< XTypeConverter > xTypeConverter;

	// Haben wir schon CoreReflection, sonst besorgen
	if( !xTypeConverter.is() )
	{
        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
	        xTypeConverter = Reference<XTypeConverter>(
		        xSMgr->createInstanceWithContext(
			        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.script.Converter")),
			            xContext ), UNO_QUERY );
        }
        if( !xTypeConverter.is() )
        {
            throw DeploymentException(
                ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM
                    ("com.sun.star.script.Converter service not accessible") ),
                Reference< XInterface >() );
        }
	}
	return xTypeConverter;
}


// #111851 factory function to create an OLE object
SbUnoObject* createOLEObject_Impl( const String& aType )
{
	static Reference< XMultiServiceFactory > xOLEFactory;
	static bool bNeedsInit = true;

	if( bNeedsInit )
	{
		bNeedsInit = false;

        Reference< XComponentContext > xContext = getComponentContext_Impl();
        if( xContext.is() )
        {
            Reference<XMultiComponentFactory> xSMgr = xContext->getServiceManager();
	        xOLEFactory = Reference<XMultiServiceFactory>(
		        xSMgr->createInstanceWithContext(
			        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.OleObjectFactory")),
			            xContext ), UNO_QUERY );
        }
	}

	SbUnoObject* pUnoObj = NULL;
	if( xOLEFactory.is() )
	{
        // some type names available in VBA can not be directly used in COM
        ::rtl::OUString aOLEType = aType;
        if ( aOLEType.equals( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "SAXXMLReader30" ) ) ) )
            aOLEType = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Msxml2.SAXXMLReader.3.0" ) );

		Reference< XInterface > xOLEObject = xOLEFactory->createInstance( aOLEType );
		if( xOLEObject.is() )
		{
			Any aAny;
			aAny <<= xOLEObject;
			pUnoObj = new SbUnoObject( aType, aAny );
		}
	}
	return pUnoObj;
}


namespace
{
    void lcl_indent( ::rtl::OUStringBuffer& _inout_rBuffer, sal_Int32 _nLevel )
    {
        while ( _nLevel-- > 0 )
            _inout_rBuffer.appendAscii( "  " );
    }
}

void implAppendExceptionMsg( ::rtl::OUStringBuffer& _inout_rBuffer, const Exception& _e, const ::rtl::OUString& _rExceptionType, sal_Int32 _nLevel )
{
    _inout_rBuffer.appendAscii( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.appendAscii( "Type: " );

    if ( _rExceptionType.isEmpty() )
        _inout_rBuffer.appendAscii( "Unknown" );
    else
        _inout_rBuffer.append( _rExceptionType );

    _inout_rBuffer.appendAscii( "\n" );
    lcl_indent( _inout_rBuffer, _nLevel );
    _inout_rBuffer.appendAscii( "Message: " );
    _inout_rBuffer.append( _e.Message );

}

// Fehlermeldungs-Message bei Exception zusammenbauen
::rtl::OUString implGetExceptionMsg( const Exception& e, const ::rtl::OUString& aExceptionType_ )
{
    ::rtl::OUStringBuffer aMessageBuf;
    implAppendExceptionMsg( aMessageBuf, e, aExceptionType_, 0 );
    return aMessageBuf.makeStringAndClear();
}

String implGetExceptionMsg( const Any& _rCaughtException )
{
    OSL_PRECOND( _rCaughtException.getValueTypeClass() == TypeClass_EXCEPTION, "implGetExceptionMsg: illegal argument!" );
    if ( _rCaughtException.getValueTypeClass() != TypeClass_EXCEPTION )
        return String();

    return implGetExceptionMsg( *static_cast< const Exception* >( _rCaughtException.getValue() ), _rCaughtException.getValueTypeName() );
}

Any convertAny( const Any& rVal, const Type& aDestType )
{
    Any aConvertedVal;
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
	try
	{
        aConvertedVal = xConverter->convertTo( rVal, aDestType );
	}
	catch( const IllegalArgumentException& )
	{
		StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( ::cppu::getCaughtException() ) );
		return aConvertedVal;
	}
	catch( CannotConvertException& e2 )
	{
        String aCannotConvertExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
		StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
		return aConvertedVal;
	}
    return aConvertedVal;
}


// #105565 Special Object to wrap a strongly typed Uno Any
TYPEINIT1(SbUnoAnyObject,SbxObject)


// TODO: Spaeter auslagern
Reference<XIdlClass> TypeToIdlClass( const Type& rType )
{
	// void als Default-Klasse eintragen
	Reference<XIdlClass> xRetClass;
	typelib_TypeDescription * pTD = 0;
	rType.getDescription( &pTD );

	if( pTD )
	{
		::rtl::OUString sOWName( pTD->pTypeName );
		Reference< XIdlReflection > xRefl = getCoreReflection_Impl();
		xRetClass = xRefl->forName( sOWName );
	}
	return xRetClass;
}

// Exception type unknown
template< class EXCEPTION >
String implGetExceptionMsg( const EXCEPTION& e )
{
    return implGetExceptionMsg( e, ::getCppuType( &e ).getTypeName() );
}

// Error-Message fuer WrappedTargetExceptions
String implGetWrappedMsg( const WrappedTargetException& e )
{
    String aMsg;
	Any aWrappedAny = e.TargetException;
    Type aExceptionType = aWrappedAny.getValueType();

	// Really an Exception?
	if( aExceptionType.getTypeClass() == TypeClass_EXCEPTION )
	{
		Exception& e_ = *( (Exception*)aWrappedAny.getValue() );
		aMsg = implGetExceptionMsg( e_, String( aExceptionType.getTypeName() ) );
	}
	// Otherwise use WrappedTargetException itself
	else
	{
        aMsg = implGetExceptionMsg( e );
	}

	return aMsg;
}

void implHandleBasicErrorException( BasicErrorException& e )
{
    SbError nError = StarBASIC::GetSfxFromVBError( (sal_uInt16)e.ErrorCode );
    StarBASIC::Error( nError, e.ErrorMessageArgument );
}

void implHandleWrappedTargetException( const Any& _rWrappedTargetException )
{
    Any aExamine( _rWrappedTargetException );

    // completely strip the first InvocationTargetException, its error message isn't of any
    // interest to the user, it just says something like "invoking the UNO method went wrong.".
    InvocationTargetException aInvocationError;
    if ( aExamine >>= aInvocationError )
        aExamine = aInvocationError.TargetException;

    BasicErrorException aBasicError;

    SbError nError( ERRCODE_BASIC_EXCEPTION );
    ::rtl::OUStringBuffer aMessageBuf;

    // strip any other WrappedTargetException instances, but this time preserve the error messages.
    WrappedTargetException aWrapped;
    sal_Int32 nLevel = 0;
    while ( aExamine >>= aWrapped )
    {
        // special handling for BasicErrorException errors
        if ( aWrapped.TargetException >>= aBasicError )
        {
            nError = StarBASIC::GetSfxFromVBError( (sal_uInt16)aBasicError.ErrorCode );
            aMessageBuf.append( aBasicError.ErrorMessageArgument );
            aExamine.clear();
            break;
        }

        // append this round's message
        implAppendExceptionMsg( aMessageBuf, aWrapped, aExamine.getValueTypeName(), nLevel );
        if ( aWrapped.TargetException.getValueTypeClass() == TypeClass_EXCEPTION )
            // there is a next chain element
            aMessageBuf.appendAscii( "\nTargetException:" );

        // next round
        aExamine = aWrapped.TargetException;
        ++nLevel;
    }

    if ( aExamine.getValueTypeClass() == TypeClass_EXCEPTION )
    {
        // the last element in the chain is still an exception, but no WrappedTargetException
        implAppendExceptionMsg( aMessageBuf, *static_cast< const Exception* >( aExamine.getValue() ), aExamine.getValueTypeName(), nLevel );
    }

    StarBASIC::Error( nError, aMessageBuf.makeStringAndClear() );
}

static void implHandleAnyException( const Any& _rCaughtException )
{
    BasicErrorException aBasicError;
    WrappedTargetException aWrappedError;

    if ( _rCaughtException >>= aBasicError )
	{
		implHandleBasicErrorException( aBasicError );
	}
    else if ( _rCaughtException >>= aWrappedError )
	{
        implHandleWrappedTargetException( _rCaughtException );
	}
    else
	{
		StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( _rCaughtException ) );
	}
}


// NativeObjectWrapper handling
struct ObjectItem
{
	SbxObjectRef	m_xNativeObj;

	ObjectItem( void )
	{}
	ObjectItem( SbxObject* pNativeObj )
		: m_xNativeObj( pNativeObj )
	{}
};
static std::vector< ObjectItem >	GaNativeObjectWrapperVector;

void clearNativeObjectWrapperVector( void )
{
	GaNativeObjectWrapperVector.clear();
}

sal_uInt32 lcl_registerNativeObjectWrapper( SbxObject* pNativeObj )
{
	sal_uInt32 nIndex = GaNativeObjectWrapperVector.size();
	GaNativeObjectWrapperVector.push_back( ObjectItem( pNativeObj ) );
	return nIndex;
}

SbxObject* lcl_getNativeObject( sal_uInt32 nIndex )
{
	SbxObjectRef xRetObj;
	if( nIndex < GaNativeObjectWrapperVector.size() )
	{
		ObjectItem& rItem = GaNativeObjectWrapperVector[ nIndex ];
		xRetObj = rItem.m_xNativeObj;
	}
	return xRetObj;
}


// Von Uno nach Sbx wandeln
SbxDataType unoToSbxType( TypeClass eType )
{
	SbxDataType eRetType = SbxVOID;

	switch( eType )
	{
		case TypeClass_INTERFACE:
		case TypeClass_TYPE:
		case TypeClass_STRUCT:
		case TypeClass_EXCEPTION:		eRetType = SbxOBJECT;	break;

		/* folgende Typen lassen wir erstmal weg
		case TypeClass_SERVICE:			break;
		case TypeClass_CLASS:			break;
		case TypeClass_TYPEDEF:			break;
		case TypeClass_UNION:			break;
		case TypeClass_ARRAY:			break;
		*/
		case TypeClass_ENUM:			eRetType = SbxLONG;		break;
		case TypeClass_SEQUENCE:
			eRetType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
			break;

		/*
		case TypeClass_VOID:			break;
		case TypeClass_UNKNOWN:			break;
		*/

		case TypeClass_ANY:				eRetType = SbxVARIANT;	break;
		case TypeClass_BOOLEAN:			eRetType = SbxBOOL;		break;
		case TypeClass_CHAR:			eRetType = SbxCHAR;		break;
		case TypeClass_STRING:			eRetType = SbxSTRING;	break;
		case TypeClass_FLOAT:			eRetType = SbxSINGLE;	break;
		case TypeClass_DOUBLE:			eRetType = SbxDOUBLE;	break;
		//case TypeClass_OCTET:									break;
		case TypeClass_BYTE:			eRetType = SbxINTEGER;  break;
		//case TypeClass_INT:				eRetType = SbxINT;	break;
		case TypeClass_SHORT:			eRetType = SbxINTEGER;	break;
		case TypeClass_LONG:			eRetType = SbxLONG;		break;
		case TypeClass_HYPER:			eRetType = SbxSALINT64;	break;
		//case TypeClass_UNSIGNED_OCTET:						break;
		case TypeClass_UNSIGNED_SHORT:	eRetType = SbxUSHORT;	break;
		case TypeClass_UNSIGNED_LONG:	eRetType = SbxULONG;	break;
		case TypeClass_UNSIGNED_HYPER:  eRetType = SbxSALUINT64;break;
		//case TypeClass_UNSIGNED_INT:	eRetType = SbxUINT;		break;
		//case TypeClass_UNSIGNED_BYTE:	eRetType = SbxUSHORT;	break;
		default: break;
	}
	return eRetType;
}

SbxDataType unoToSbxType( const Reference< XIdlClass >& xIdlClass )
{
	SbxDataType eRetType = SbxVOID;
	if( xIdlClass.is() )
	{
		TypeClass eType = xIdlClass->getTypeClass();
        eRetType = unoToSbxType( eType );
    }
	return eRetType;
}

static void implSequenceToMultiDimArray( SbxDimArray*& pArray, Sequence< sal_Int32 >& indices, Sequence< sal_Int32 >& sizes, const Any& aValue, sal_Int32& dimension, sal_Bool bIsZeroIndex, Type* pType = NULL )
{
	Type aType = aValue.getValueType();
	TypeClass eTypeClass = aType.getTypeClass();

	sal_Int32 indicesIndex = indices.getLength() -1;
	sal_Int32 dimCopy = dimension;

	if ( eTypeClass == TypeClass_SEQUENCE )
	{
		Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
		Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
		typelib_TypeDescription * pTD = 0;
        aType.getDescription( &pTD );
		Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
		::typelib_typedescription_release( pTD );

		sal_Int32 nLen = xIdlArray->getLen( aValue );
		for ( sal_Int32 index = 0; index < nLen; ++index )
		{
			Any aElementAny = xIdlArray->get( aValue, (sal_uInt32)index );
			// This detects the dimension were currently processing
			if ( dimCopy == dimension )
			{
				++dimCopy;
				if ( sizes.getLength() < dimCopy )
				{
					sizes.realloc( sizes.getLength() + 1 );
					sizes[ sizes.getLength() - 1 ] = nLen;
					indices.realloc( indices.getLength() + 1 );
					indicesIndex = indices.getLength() - 1;
				}
			}

			if ( bIsZeroIndex )
				indices[ dimCopy - 1 ] = index;
			else
				indices[ dimCopy - 1] = index + 1;

			implSequenceToMultiDimArray( pArray, indices, sizes, aElementAny, dimCopy, bIsZeroIndex, &aElementType );
		}

	}
	else
	{
		if ( indices.getLength() < 1 )
		{
			// Should never ever get here ( indices.getLength()
			// should equal number of dimensions in the array )
			// And that should at least be 1 !
			// #QUESTION is there a better error?
			StarBASIC::Error( SbERR_INVALID_OBJECT );
			return;
		}

		SbxDataType eSbxElementType = unoToSbxType( pType ? pType->getTypeClass() : aValue.getValueTypeClass() );
		if ( !pArray )
		{
			pArray = new SbxDimArray( eSbxElementType );
			sal_Int32 nIndexLen = indices.getLength();

			// Dimension the array
			for ( sal_Int32 index = 0; index < nIndexLen; ++index )
			{
				if ( bIsZeroIndex )
					pArray->unoAddDim32( 0, sizes[ index ] - 1);
				else
					pArray->unoAddDim32( 1, sizes[ index ] );

			}
		}

		if ( pArray )
		{
			SbxVariableRef xVar = new SbxVariable( eSbxElementType );
			unoToSbxValue( (SbxVariable*)xVar, aValue );

			sal_Int32* pIndices = indices.getArray();
			pArray->Put32( 	(SbxVariable*)xVar, pIndices );

		}
	}
}

void unoToSbxValue( SbxVariable* pVar, const Any& aValue )
{
	Type aType = aValue.getValueType();
	TypeClass eTypeClass = aType.getTypeClass();
	switch( eTypeClass )
	{
		case TypeClass_TYPE:
		{
			// Map Type to IdlClass
			Type aType_;
			aValue >>= aType_;
			Reference<XIdlClass> xClass = TypeToIdlClass( aType_ );
			Any aClassAny;
			aClassAny <<= xClass;

			// SbUnoObject instanzieren
			String aName;
			SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aClassAny );
			SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

			// #51475 Wenn das Objekt ungueltig ist null liefern
			if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
			{
				pVar->PutObject( NULL );
			}
			else
			{
				pVar->PutObject( xWrapper );
			}
		}
		break;
		// Interfaces und Structs muessen in ein SbUnoObject gewrappt werden
		case TypeClass_INTERFACE:
		case TypeClass_STRUCT:
		case TypeClass_EXCEPTION:
		{
			if( eTypeClass == TypeClass_STRUCT )
			{
				ArrayWrapper aWrap;
				NativeObjectWrapper aNativeObjectWrapper;
				if ( (aValue >>= aWrap) )
				{
					SbxDimArray* pArray = NULL;
					Sequence< sal_Int32 > indices;
					Sequence< sal_Int32 > sizes;
					sal_Int32 dimension = 0;
					implSequenceToMultiDimArray( pArray, indices, sizes, aWrap.Array, dimension, aWrap.IsZeroIndex );
					if ( pArray )
					{
						SbxDimArrayRef xArray = pArray;
						sal_uInt16 nFlags = pVar->GetFlags();
						pVar->ResetFlag( SBX_FIXED );
						pVar->PutObject( (SbxDimArray*)xArray );
						pVar->SetFlags( nFlags );
					}
					else
						pVar->PutEmpty();
					break;
				}
				else if ( (aValue >>= aNativeObjectWrapper) )
				{
					sal_uInt32 nIndex = 0;
					if( (aNativeObjectWrapper.ObjectId >>= nIndex) )
					{
						SbxObject* pObj = lcl_getNativeObject( nIndex );
						pVar->PutObject( pObj );
					}
					else
						pVar->PutEmpty();
					break;
				}
				else
				{
					SbiInstance* pInst = pINST;
					if( pInst && pInst->IsCompatibility() )
					{
						oleautomation::Date aDate;
						if( (aValue >>= aDate) )
						{
							pVar->PutDate( aDate.Value );
							break;
						}
						else
						{
							oleautomation::Decimal aDecimal;
							if( (aValue >>= aDecimal) )
							{
								pVar->PutDecimal( aDecimal );
								break;
							}
							else
							{
								oleautomation::Currency aCurrency;
								if( (aValue >>= aCurrency) )
								{
									sal_Int64 nValue64 = aCurrency.Value;
									SbxINT64 aInt64;
									aInt64.nHigh =
                                        sal::static_int_cast< sal_Int32 >(
                                            nValue64 >> 32);
									aInt64.nLow = (sal_uInt32)( nValue64 & 0xffffffff );
									pVar->PutCurrency( aInt64 );
									break;
								}
							}
						}
					}
				}
			}
			// SbUnoObject instanzieren
			String aName;
			SbUnoObject* pSbUnoObject = new SbUnoObject( aName, aValue );
			//If this is called externally e.g. from the scripting 
			//framework then there is no 'active' runtime the default property will not be set up
			//only a vba object will have XDefaultProp set anyway so... this
			//test seems a bit of overkill
			//if ( SbiRuntime::isVBAEnabled() )
			{
				String sDfltPropName;

				if ( SbUnoObject::getDefaultPropName( pSbUnoObject, sDfltPropName ) )
						pSbUnoObject->SetDfltProperty( sDfltPropName );
			}
			SbxObjectRef xWrapper = (SbxObject*)pSbUnoObject;

			// #51475 Wenn das Objekt ungueltig ist null liefern
			if( pSbUnoObject->getUnoAny().getValueType().getTypeClass() == TypeClass_VOID )
			{
				pVar->PutObject( NULL );
			}
			else
			{
				pVar->PutObject( xWrapper );
			}
		}
		break;

		/* folgende Typen lassen wir erstmal weg
		case TypeClass_SERVICE:			break;
		case TypeClass_CLASS:			break;
		case TypeClass_TYPEDEF:			break;
		case TypeClass_UNION:			break;
		case TypeClass_ENUM:			break;
		case TypeClass_ARRAY:			break;
		*/

		case TypeClass_ENUM:
		{
			sal_Int32 nEnum = 0;
			enum2int( nEnum, aValue );
			pVar->PutLong( nEnum );
		}
			break;

		case TypeClass_SEQUENCE:
		{
			Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aType );
			Reference< XIdlArray > xIdlArray = xIdlTargetClass->getArray();
			sal_Int32 i, nLen = xIdlArray->getLen( aValue );

            typelib_TypeDescription * pTD = 0;
            aType.getDescription( &pTD );
            OSL_ASSERT( pTD && pTD->eTypeClass == typelib_TypeClass_SEQUENCE );
            Type aElementType( ((typelib_IndirectTypeDescription *)pTD)->pType );
            ::typelib_typedescription_release( pTD );

			// In Basic Array anlegen
			SbxDimArrayRef xArray;
            SbxDataType eSbxElementType = unoToSbxType( aElementType.getTypeClass() );
    		xArray = new SbxDimArray( eSbxElementType );
			if( nLen > 0 )
            {
				xArray->unoAddDim32( 0, nLen - 1 );

			    // Elemente als Variablen eintragen
			    for( i = 0 ; i < nLen ; i++ )
			    {
				    // Elemente wandeln
				    Any aElementAny = xIdlArray->get( aValue, (sal_uInt32)i );
				    SbxVariableRef xVar = new SbxVariable( eSbxElementType );
				    unoToSbxValue( (SbxVariable*)xVar, aElementAny );

				    // Ins Array braten
				    xArray->Put32( (SbxVariable*)xVar, &i );
			    }
            }
            else
            {
    			xArray->unoAddDim( 0, -1 );
            }

			// Array zurueckliefern
			sal_uInt16 nFlags = pVar->GetFlags();
			pVar->ResetFlag( SBX_FIXED );
			pVar->PutObject( (SbxDimArray*)xArray );
			pVar->SetFlags( nFlags );

			// #54548, Die Parameter duerfen hier nicht weggehauen werden
			//pVar->SetParameters( NULL );
		}
		break;

		/*
		case TypeClass_VOID:			break;
		case TypeClass_UNKNOWN:			break;

		case TypeClass_ANY:
		{
			// Any rausholen und konvertieren
			//Any* pAny = (Any*)aValue.get();
			//if( pAny )
				//unoToSbxValue( pVar, *pAny );
		}
		break;
		*/

		case TypeClass_BOOLEAN:			pVar->PutBool( *(sal_Bool*)aValue.getValue() );	break;
		case TypeClass_CHAR:
		{
			pVar->PutChar( *(sal_Unicode*)aValue.getValue() );
			break;
		}
		case TypeClass_STRING:			{ ::rtl::OUString val; aValue >>= val; pVar->PutString( String( val ) ); }	break;
		case TypeClass_FLOAT:			{ float val = 0; aValue >>= val; pVar->PutSingle( val ); } break;
		case TypeClass_DOUBLE:			{ double val = 0; aValue >>= val; pVar->PutDouble( val ); } break;
		//case TypeClass_OCTET:			break;
		case TypeClass_BYTE:			{ sal_Int8 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
		//case TypeClass_INT:			break;
		case TypeClass_SHORT:			{ sal_Int16 val = 0; aValue >>= val; pVar->PutInteger( val ); } break;
		case TypeClass_LONG:			{ sal_Int32 val = 0; aValue >>= val; pVar->PutLong( val ); } break;
		case TypeClass_HYPER:			{ sal_Int64 val = 0; aValue >>= val; pVar->PutInt64( val ); } break;
		//case TypeClass_UNSIGNED_OCTET:break;
		case TypeClass_UNSIGNED_SHORT:	{ sal_uInt16 val = 0; aValue >>= val; pVar->PutUShort( val ); } break;
		case TypeClass_UNSIGNED_LONG:	{ sal_uInt32 val = 0; aValue >>= val; pVar->PutULong( val ); } break;
		case TypeClass_UNSIGNED_HYPER:	{ sal_uInt64 val = 0; aValue >>= val; pVar->PutUInt64( val ); } break;
		//case TypeClass_UNSIGNED_INT:	break;
		//case TypeClass_UNSIGNED_BYTE:	break;
		default:						pVar->PutEmpty();						break;
	}
}

// Reflection fuer Sbx-Typen liefern
Type getUnoTypeForSbxBaseType( SbxDataType eType )
{
	Type aRetType = getCppuVoidType();
	switch( eType )
	{
		//case SbxEMPTY:		eRet = TypeClass_VOID; break;
		case SbxNULL:		aRetType = ::getCppuType( (const Reference< XInterface > *)0 ); break;
		case SbxINTEGER:	aRetType = ::getCppuType( (sal_Int16*)0 ); break;
		case SbxLONG:		aRetType = ::getCppuType( (sal_Int32*)0 ); break;
		case SbxSINGLE:		aRetType = ::getCppuType( (float*)0 ); break;
		case SbxDOUBLE:		aRetType = ::getCppuType( (double*)0 ); break;
		case SbxCURRENCY:	aRetType = ::getCppuType( (oleautomation::Currency*)0 ); break;
		case SbxDECIMAL:	aRetType = ::getCppuType( (oleautomation::Decimal*)0 ); break;
		case SbxDATE:		{
							SbiInstance* pInst = pINST;
							if( pInst && pInst->IsCompatibility() )
								aRetType = ::getCppuType( (double*)0 );
							else
								aRetType = ::getCppuType( (oleautomation::Date*)0 );
							}
							break;
		// case SbxDATE:		aRetType = ::getCppuType( (double*)0 ); break;
		case SbxSTRING:		aRetType = ::getCppuType( (::rtl::OUString*)0 ); break;
		//case SbxOBJECT:	break;
		//case SbxERROR:	break;
		case SbxBOOL:		aRetType = ::getCppuType( (sal_Bool*)0 ); break;
		case SbxVARIANT:	aRetType = ::getCppuType( (Any*)0 ); break;
		//case SbxDATAOBJECT: break;
		case SbxCHAR:		aRetType = ::getCppuType( (sal_Unicode*)0 ); break;
		case SbxBYTE:		aRetType = ::getCppuType( (sal_Int8*)0 ); break;
		case SbxUSHORT:		aRetType = ::getCppuType( (sal_uInt16*)0 ); break;
		case SbxULONG:		aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
		//case SbxLONG64:	break;
		//case SbxULONG64:	break;
		// Maschinenabhaengige zur Sicherheit auf Hyper abbilden
		case SbxINT:		aRetType = ::getCppuType( (sal_Int32*)0 ); break;
		case SbxUINT:		aRetType = ::getCppuType( (sal_uInt32*)0 ); break;
		//case SbxVOID:		break;
		//case SbxHRESULT:	break;
		//case SbxPOINTER:	break;
		//case SbxDIMARRAY:	break;
		//case SbxCARRAY:	break;
		//case SbxUSERDEF:	break;
		//case SbxLPSTR:	break;
		//case SbxLPWSTR:	break;
		//case SbxCoreSTRING: break;
		default: break;
	}
	return aRetType;
}

// Konvertierung von Sbx nach Uno ohne bekannte Zielklasse fuer TypeClass_ANY
Type getUnoTypeForSbxValue( SbxValue* pVal )
{
	Type aRetType = getCppuVoidType();
	if( !pVal )
		return aRetType;

	// SbxType nach Uno wandeln
	SbxDataType eBaseType = pVal->SbxValue::GetType();
	if( eBaseType == SbxOBJECT )
	{
		SbxBaseRef xObj = (SbxBase*)pVal->GetObject();
		if( !xObj )
		{
			// #109936 No error any more
			// StarBASIC::Error( SbERR_INVALID_OBJECT );
			aRetType = getCppuType( static_cast<Reference<XInterface> *>(0) );
			return aRetType;
		}

		if( xObj->ISA(SbxDimArray) )
		{
			SbxBase* pObj = (SbxBase*)xObj;
			SbxDimArray* pArray = (SbxDimArray*)pObj;

			short nDims = pArray->GetDims();
			Type aElementType = getUnoTypeForSbxBaseType( (SbxDataType)(pArray->GetType() & 0xfff) );
			TypeClass eElementTypeClass = aElementType.getTypeClass();

			// Normal case: One dimensional array
			sal_Int32 nLower, nUpper;
			if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
			{
				if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
				{
					// Wenn alle Elemente des Arrays vom gleichen Typ sind, wird
					// der genommen, sonst wird das ganze als Any-Sequence betrachtet
					sal_Bool bNeedsInit = sal_True;

    				sal_Int32 nSize = nUpper - nLower + 1;
					sal_Int32 nIdx = nLower;
					for( sal_Int32 i = 0 ; i < nSize ; i++,nIdx++ )
					{
						SbxVariableRef xVar = pArray->Get32( &nIdx );
						Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
						if( bNeedsInit )
						{
							if( aType.getTypeClass() == TypeClass_VOID )
							{
								// #88522
								// if only first element is void: different types  -> []any
								// if all elements are void: []void is not allowed -> []any
								aElementType = getCppuType( (Any*)0 );
								break;
							}
							aElementType = aType;
							bNeedsInit = sal_False;
						}
						else if( aElementType != aType )
						{
							// Verschiedene Typen -> AnySequence
							aElementType = getCppuType( (Any*)0 );
							break;
						}
					}
				}

				::rtl::OUString aSeqTypeName( aSeqLevelStr );
				aSeqTypeName += aElementType.getTypeName();
				aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
			}
			// #i33795 Map also multi dimensional arrays to corresponding sequences
			else if( nDims > 1 )
			{
				if( eElementTypeClass == TypeClass_VOID || eElementTypeClass == TypeClass_ANY )
				{
					// For this check the array's dim structure does not matter
					sal_uInt32 nFlatArraySize = pArray->Count32();

					sal_Bool bNeedsInit = sal_True;
					for( sal_uInt32 i = 0 ; i < nFlatArraySize ; i++ )
					{
						SbxVariableRef xVar = pArray->SbxArray::Get32( i );
						Type aType = getUnoTypeForSbxValue( (SbxVariable*)xVar );
						if( bNeedsInit )
						{
							if( aType.getTypeClass() == TypeClass_VOID )
							{
								// if only first element is void: different types  -> []any
								// if all elements are void: []void is not allowed -> []any
								aElementType = getCppuType( (Any*)0 );
								break;
							}
							aElementType = aType;
							bNeedsInit = sal_False;
						}
						else if( aElementType != aType )
						{
							// Verschiedene Typen -> AnySequence
							aElementType = getCppuType( (Any*)0 );
							break;
						}
					}
				}

				::rtl::OUString aSeqTypeName;
				for( short iDim = 0 ; iDim < nDims ; iDim++ )
					aSeqTypeName += aSeqLevelStr;
				aSeqTypeName += aElementType.getTypeName();
				aRetType = Type( TypeClass_SEQUENCE, aSeqTypeName );
			}
		}
		// Kein Array, sondern...
		else if( xObj->ISA(SbUnoObject) )
		{
			aRetType = ((SbUnoObject*)(SbxBase*)xObj)->getUnoAny().getValueType();
		}
		// SbUnoAnyObject?
		else if( xObj->ISA(SbUnoAnyObject) )
		{
			aRetType = ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue().getValueType();
		}
		// Sonst ist es ein Nicht-Uno-Basic-Objekt -> default==void liefern
	}
	// Kein Objekt, Basistyp konvertieren
	else
	{
		aRetType = getUnoTypeForSbxBaseType( eBaseType );
	}
	return aRetType;
}

// Deklaration Konvertierung von Sbx nach Uno mit bekannter Zielklasse
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty = NULL );

// Konvertierung von Sbx nach Uno ohne bekannte Zielklasse fuer TypeClass_ANY
Any sbxToUnoValueImpl( SbxVariable* pVar, bool bBlockConversionToSmallestType = false )
{
	SbxDataType eBaseType = pVar->SbxValue::GetType();
	if( eBaseType == SbxOBJECT )
    {
		SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
		if( xObj.Is() )
		{
			if( xObj->ISA(SbUnoAnyObject) )
				return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
			if( xObj->ISA(SbClassModuleObject) )
			{
				Any aRetAny;
				SbClassModuleObject* pClassModuleObj = (SbClassModuleObject*)(SbxBase*)xObj;
				SbModule* pClassModule = pClassModuleObj->getClassModule();
				if( pClassModule->createCOMWrapperForIface( aRetAny, pClassModuleObj ) )
					return aRetAny;
			}
			if( !xObj->ISA(SbUnoObject) )
			{
				// Create NativeObjectWrapper to identify object in case of callbacks
				SbxObject* pObj = PTR_CAST(SbxObject,pVar->GetObject());
				if( pObj != NULL )
				{
					NativeObjectWrapper aNativeObjectWrapper;
					sal_uInt32 nIndex = lcl_registerNativeObjectWrapper( pObj );
					aNativeObjectWrapper.ObjectId <<= nIndex;
					Any aRetAny;
					aRetAny <<= aNativeObjectWrapper;
					return aRetAny;
				}
			}
		}
    }

	Type aType = getUnoTypeForSbxValue( pVar );
    TypeClass eType = aType.getTypeClass();

	if( !bBlockConversionToSmallestType )
	{
		// #79615 Choose "smallest" represention for int values
		// because up cast is allowed, downcast not
		switch( eType )
		{
			case TypeClass_FLOAT:
			case TypeClass_DOUBLE:
			{
				double d = pVar->GetDouble();
				if( d == floor( d ) )
				{
					if( d >= -128 && d <= 127 )
						aType = ::getCppuType( (sal_Int8*)0 );
					else if( d >= SbxMININT && d <= SbxMAXINT )
						aType = ::getCppuType( (sal_Int16*)0 );
					else if( d >= -SbxMAXLNG && d <= SbxMAXLNG )
						aType = ::getCppuType( (sal_Int32*)0 );
				}
				break;
			}
			case TypeClass_SHORT:
			{
				sal_Int16 n = pVar->GetInteger();
				if( n >= -128 && n <= 127 )
					aType = ::getCppuType( (sal_Int8*)0 );
				break;
			}
			case TypeClass_LONG:
			{
				sal_Int32 n = pVar->GetLong();
				if( n >= -128 && n <= 127 )
					aType = ::getCppuType( (sal_Int8*)0 );
				else if( n >= SbxMININT && n <= SbxMAXINT )
					aType = ::getCppuType( (sal_Int16*)0 );
				break;
			}
			case TypeClass_UNSIGNED_SHORT:
			{
				sal_uInt16 n = pVar->GetUShort();
				if( n <= 255 )
					aType = ::getCppuType( (sal_uInt8*)0 );
				break;
			}
			case TypeClass_UNSIGNED_LONG:
			{
				sal_uInt32 n = pVar->GetLong();
				if( n <= 255 )
					aType = ::getCppuType( (sal_uInt8*)0 );
				else if( n <= SbxMAXUINT )
					aType = ::getCppuType( (sal_uInt16*)0 );
				break;
			}
			default: break;
		}
	}

	return sbxToUnoValue( pVar, aType );
}



// Helper function for StepREDIMP
static Any implRekMultiDimArrayToSequence( SbxDimArray* pArray,
	const Type& aElemType, short nMaxDimIndex, short nActualDim,
	sal_Int32* pActualIndices, sal_Int32* pLowerBounds, sal_Int32* pUpperBounds )
{
	sal_Int32 nSeqLevel = nMaxDimIndex - nActualDim + 1;
	::rtl::OUString aSeqTypeName;
	sal_Int32 i;
	for( i = 0 ; i < nSeqLevel ; i++ )
		aSeqTypeName += aSeqLevelStr;

	aSeqTypeName += aElemType.getTypeName();
	Type aSeqType( TypeClass_SEQUENCE, aSeqTypeName );

	// Create Sequence instance
	Any aRetVal;
	Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( aSeqType );
	xIdlTargetClass->createObject( aRetVal );

	// Alloc sequence according to array bounds
	sal_Int32 nUpper = pUpperBounds[nActualDim];
	sal_Int32 nLower = pLowerBounds[nActualDim];
	sal_Int32 nSeqSize = nUpper - nLower + 1;
	Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
	xArray->realloc( aRetVal, nSeqSize );

	sal_Int32& ri = pActualIndices[nActualDim];

	for( ri = nLower,i = 0 ; ri <= nUpper ; ri++,i++ )
	{
		Any aElementVal;

		if( nActualDim < nMaxDimIndex )
		{
			aElementVal = implRekMultiDimArrayToSequence( pArray, aElemType,
				nMaxDimIndex, nActualDim + 1, pActualIndices, pLowerBounds, pUpperBounds );
		}
		else
		{
			SbxVariable* pSource = pArray->Get32( pActualIndices );
			aElementVal = sbxToUnoValue( pSource, aElemType );
		}

		try
		{
			// In die Sequence uebernehmen
			xArray->set( aRetVal, i, aElementVal );
		}
		catch( const IllegalArgumentException& )
		{
			StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
				implGetExceptionMsg( ::cppu::getCaughtException() ) );
		}
		catch (IndexOutOfBoundsException&)
		{
			StarBASIC::Error( SbERR_OUT_OF_RANGE );
		}
	}
	return aRetVal;
}

// Map old interface
Any sbxToUnoValue( SbxVariable* pVar )
{
	return sbxToUnoValueImpl( pVar );
}


// Funktion, um einen globalen Bezeichner im
// UnoScope zu suchen und fuer Sbx zu wrappen
static bool implGetTypeByName( const String& rName, Type& rRetType )
{
	bool bSuccess = false;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
		Reference< XTypeDescription > xTypeDesc;
		aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
			rRetType = Type( xTypeDesc->getTypeClass(), xTypeDesc->getName() );
			bSuccess = true;
        }
    }
	return bSuccess;
}


// Konvertierung von Sbx nach Uno mit bekannter Zielklasse
Any sbxToUnoValue( SbxVariable* pVar, const Type& rType, Property* pUnoProperty )
{
	Any aRetVal;

	// #94560 No conversion of empty/void for MAYBE_VOID properties
	if( pUnoProperty && pUnoProperty->Attributes & PropertyAttribute::MAYBEVOID )
	{
		if( pVar->IsEmpty() )
			return aRetVal;
	}

	SbxDataType eBaseType = pVar->SbxValue::GetType();
	if( eBaseType == SbxOBJECT )
    {
		SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
		if( xObj.Is() && xObj->ISA(SbUnoAnyObject) )
        {
            return ((SbUnoAnyObject*)(SbxBase*)xObj)->getValue();
        }
    }

    TypeClass eType = rType.getTypeClass();
	switch( eType )
	{
		case TypeClass_INTERFACE:
		case TypeClass_STRUCT:
		case TypeClass_EXCEPTION:
		{
			Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );

			// Null-Referenz?
			if( pVar->IsNull() && eType == TypeClass_INTERFACE )
			{
				Reference< XInterface > xRef;
				::rtl::OUString aClassName = xIdlTargetClass->getName();
				Type aClassType( xIdlTargetClass->getTypeClass(), aClassName.getStr() );
				aRetVal.setValue( &xRef, aClassType );
			}
			else
			{
				// #112368 Special conversion for Decimal, Currency and Date
				if( eType == TypeClass_STRUCT )
				{
					SbiInstance* pInst = pINST;
					if( pInst && pInst->IsCompatibility() )
					{
						if( rType == ::getCppuType( (oleautomation::Decimal*)0 ) )
						{
							oleautomation::Decimal aDecimal;
							pVar->fillAutomationDecimal( aDecimal );
							aRetVal <<= aDecimal;
							break;
						}
						else if( rType == ::getCppuType( (oleautomation::Currency*)0 ) )
						{
							SbxINT64 aInt64 = pVar->GetCurrency();
							oleautomation::Currency aCurrency;
							sal_Int64& rnValue64 = aCurrency.Value;
							rnValue64 = aInt64.nHigh;
							rnValue64 <<= 32;
							rnValue64 |= aInt64.nLow;
							aRetVal <<= aCurrency;
							break;
						}
						else if( rType == ::getCppuType( (oleautomation::Date*)0 ) )
						{
							oleautomation::Date aDate;
							aDate.Value = pVar->GetDate();
							aRetVal <<= aDate;
							break;
						}
					}
				}

				SbxBaseRef pObj = (SbxBase*)pVar->GetObject();
				if( pObj && pObj->ISA(SbUnoObject) )
				{
					aRetVal = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
				}
				else
				{
					// #109936 NULL object -> NULL XInterface
					Reference<XInterface> xInt;
					aRetVal <<= xInt;
				}
			}
		}
		break;

		case TypeClass_TYPE:
		{
			if( eBaseType == SbxOBJECT )
			{
				// XIdlClass?
				Reference< XIdlClass > xIdlClass;

				SbxBaseRef pObj = (SbxBase*)pVar->GetObject();
				if( pObj && pObj->ISA(SbUnoObject) )
				{
					Any aUnoAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
					aUnoAny >>= xIdlClass;
				}

				if( xIdlClass.is() )
				{
					::rtl::OUString aClassName = xIdlClass->getName();
					Type aType( xIdlClass->getTypeClass(), aClassName.getStr() );
					aRetVal <<= aType;
				}
			}
			else if( eBaseType == SbxSTRING )
			{
				// String representing type?
				String aTypeName = pVar->GetString();
				Type aType;
				bool bSuccess = implGetTypeByName( aTypeName, aType );
				if( bSuccess )
					aRetVal <<= aType;
			}
		}
		break;

		/* folgende Typen lassen wir erstmal weg
		case TypeClass_SERVICE:			break;
		case TypeClass_CLASS:			break;
		case TypeClass_TYPEDEF:			break;
		case TypeClass_UNION:			break;
		case TypeClass_ENUM:			break;
		case TypeClass_ARRAY:			break;
		*/

		// Array -> Sequence
		case TypeClass_ENUM:
		{
			aRetVal = int2enum( pVar->GetLong(), rType );
		}
		break;

		case TypeClass_SEQUENCE:
		{
			SbxBaseRef xObj = (SbxBase*)pVar->GetObject();
			if( xObj && xObj->ISA(SbxDimArray) )
			{
				SbxBase* pObj = (SbxBase*)xObj;
				SbxDimArray* pArray = (SbxDimArray*)pObj;

				short nDims = pArray->GetDims();

				// Normal case: One dimensional array
				sal_Int32 nLower, nUpper;
				if( nDims == 1 && pArray->GetDim32( 1, nLower, nUpper ) )
				{
					sal_Int32 nSeqSize = nUpper - nLower + 1;

					// Instanz der geforderten Sequence erzeugen
					Reference< XIdlClass > xIdlTargetClass = TypeToIdlClass( rType );
					xIdlTargetClass->createObject( aRetVal );
					Reference< XIdlArray > xArray = xIdlTargetClass->getArray();
					xArray->realloc( aRetVal, nSeqSize );

					// Element-Type
					::rtl::OUString aClassName = xIdlTargetClass->getName();
					typelib_TypeDescription * pSeqTD = 0;
					typelib_typedescription_getByName( &pSeqTD, aClassName.pData );
					OSL_ASSERT( pSeqTD );
					Type aElemType( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
					// Reference< XIdlClass > xElementClass = TypeToIdlClass( aElemType );

					// Alle Array-Member umwandeln und eintragen
					sal_Int32 nIdx = nLower;
					for( sal_Int32 i = 0 ; i < nSeqSize ; i++,nIdx++ )
					{
						SbxVariableRef xVar = pArray->Get32( &nIdx );

						// Wert von Sbx nach Uno wandeln
						Any aAnyValue = sbxToUnoValue( (SbxVariable*)xVar, aElemType );

						try
						{
							// In die Sequence uebernehmen
							xArray->set( aRetVal, i, aAnyValue );
						}
						catch( const IllegalArgumentException& )
						{
							StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
                                implGetExceptionMsg( ::cppu::getCaughtException() ) );
						}
						catch (IndexOutOfBoundsException&)
						{
							StarBASIC::Error( SbERR_OUT_OF_RANGE );
						}
					}
				}
				// #i33795 Map also multi dimensional arrays to corresponding sequences
				else if( nDims > 1 )
				{
					// Element-Type
					typelib_TypeDescription * pSeqTD = 0;
					Type aCurType( rType );
					sal_Int32 nSeqLevel = 0;
					Type aElemType;
					do
					{
						::rtl::OUString aTypeName = aCurType.getTypeName();
						typelib_typedescription_getByName( &pSeqTD, aTypeName.pData );
						OSL_ASSERT( pSeqTD );
						if( pSeqTD->eTypeClass == typelib_TypeClass_SEQUENCE )
						{
							aCurType = Type( ((typelib_IndirectTypeDescription *)pSeqTD)->pType );
							nSeqLevel++;
						}
						else
						{
							aElemType = aCurType;
							break;
						}
					}
					while( true );

					if( nSeqLevel == nDims )
					{
						sal_Int32* pLowerBounds = new sal_Int32[nDims];
						sal_Int32* pUpperBounds = new sal_Int32[nDims];
						sal_Int32* pActualIndices = new sal_Int32[nDims];
						for( short i = 1 ; i <= nDims ; i++ )
						{
							sal_Int32 lBound, uBound;
							pArray->GetDim32( i, lBound, uBound );

							short j = i - 1;
							pActualIndices[j] = pLowerBounds[j] = lBound;
							pUpperBounds[j] = uBound;
						}

						aRetVal = implRekMultiDimArrayToSequence( pArray, aElemType,
							nDims - 1, 0, pActualIndices, pLowerBounds, pUpperBounds );

						delete[] pUpperBounds;
						delete[] pLowerBounds;
						delete[] pActualIndices;
					}
				}
			}
		}
		break;

		/*
		case TypeClass_VOID:			break;
		case TypeClass_UNKNOWN:			break;
		*/

		// Bei Any die Klassen-unabhaengige Konvertierungs-Routine nutzen
		case TypeClass_ANY:
		{
			aRetVal = sbxToUnoValueImpl( pVar );
		}
		break;

		case TypeClass_BOOLEAN:
		{
			sal_Bool b = pVar->GetBool();
			aRetVal.setValue( &b, getBooleanCppuType() );
			break;
		}
		case TypeClass_CHAR:
		{
			sal_Unicode c = pVar->GetChar();
			aRetVal.setValue( &c , getCharCppuType() );
			break;
		}
		case TypeClass_STRING:			aRetVal <<= pVar->GetOUString(); break;
		case TypeClass_FLOAT:			aRetVal <<= pVar->GetSingle(); break;
		case TypeClass_DOUBLE:			aRetVal <<= pVar->GetDouble(); break;
		//case TypeClass_OCTET:			break;

		case TypeClass_BYTE:
        {
            sal_Int16 nVal = pVar->GetInteger();
            sal_Bool bOverflow = sal_False;
            if( nVal < -128 )
            {
                bOverflow = sal_True;
                nVal = -128;
            }
            else if( nVal > 127 )
            {
                bOverflow = sal_True;
                nVal = 127;
            }
            if( bOverflow )
           		StarBASIC::Error( ERRCODE_BASIC_MATH_OVERFLOW );

            sal_Int8 nByteVal = (sal_Int8)nVal;
            aRetVal <<= nByteVal;
            break;
        }
		//case TypeClass_INT:			break;
		case TypeClass_SHORT:			aRetVal <<= (sal_Int16)( pVar->GetInteger() );	break;
		case TypeClass_LONG:			aRetVal <<= (sal_Int32)( pVar->GetLong() );     break;
		case TypeClass_HYPER:			aRetVal <<= (sal_Int64)( pVar->GetInt64() );    break;
		//case TypeClass_UNSIGNED_OCTET:break;
		case TypeClass_UNSIGNED_SHORT:	aRetVal <<= (sal_uInt16)( pVar->GetUShort() );	break;
		case TypeClass_UNSIGNED_LONG:	aRetVal <<= (sal_uInt32)( pVar->GetULong() );	break;
		case TypeClass_UNSIGNED_HYPER:  aRetVal <<= (sal_uInt64)( pVar->GetUInt64() );  break;
		//case TypeClass_UNSIGNED_INT:	break;
		//case TypeClass_UNSIGNED_BYTE:	break;
		default: break;
	}

	return aRetVal;
}

// Dbg-Hilfsmethode zum Auslesen der in einem Object implementierten Interfaces
String Impl_GetInterfaceInfo( const Reference< XInterface >& x, const Reference< XIdlClass >& xClass, sal_uInt16 nRekLevel )
{
	Type aIfaceType = ::getCppuType( (const Reference< XInterface > *)0 );
	static Reference< XIdlClass > xIfaceClass = TypeToIdlClass( aIfaceType );

	String aRetStr;
	for( sal_uInt16 i = 0 ; i < nRekLevel ; i++ )
		aRetStr.AppendAscii( "    " );
	aRetStr += String( xClass->getName() );
	::rtl::OUString aClassName = xClass->getName();
	Type aClassType( xClass->getTypeClass(), aClassName.getStr() );

	// Pruefen, ob das Interface wirklich unterstuetzt wird
	if( !x->queryInterface( aClassType ).hasValue() )
	{
		aRetStr.AppendAscii( " (ERROR: Not really supported!)\n" );
	}
	// Gibt es Super-Interfaces
	else
	{
		aRetStr.AppendAscii( "\n" );

		// Super-Interfaces holen
		Sequence< Reference< XIdlClass > > aSuperClassSeq = xClass->getSuperclasses();
		const Reference< XIdlClass >* pClasses = aSuperClassSeq.getConstArray();
		sal_uInt32 nSuperIfaceCount = aSuperClassSeq.getLength();
		for( sal_uInt32 j = 0 ; j < nSuperIfaceCount ; j++ )
		{
			const Reference< XIdlClass >& rxIfaceClass = pClasses[j];
			if( !rxIfaceClass->equals( xIfaceClass ) )
				aRetStr += Impl_GetInterfaceInfo( x, rxIfaceClass, nRekLevel + 1 );
		}
	}
	return aRetStr;
}

String getDbgObjectNameImpl( SbUnoObject* pUnoObj )
{
	String aName;
	if( pUnoObj )
	{
		aName = pUnoObj->GetClassName();
		if( !aName.Len() )
		{
			Any aToInspectObj = pUnoObj->getUnoAny();
			TypeClass eType = aToInspectObj.getValueType().getTypeClass();
			Reference< XInterface > xObj;
			if( eType == TypeClass_INTERFACE )
				xObj = *(Reference< XInterface >*)aToInspectObj.getValue();
			if( xObj.is() )
			{
				Reference< XServiceInfo > xServiceInfo( xObj, UNO_QUERY );
				if( xServiceInfo.is() )
					aName = xServiceInfo->getImplementationName();
			}
		}
	}
	return aName;
}

String getDbgObjectName( SbUnoObject* pUnoObj )
{
	String aName = getDbgObjectNameImpl( pUnoObj );
	if( !aName.Len() )
		aName.AppendAscii( "Unknown" );

	String aRet;
	if( aName.Len() > 20 )
		aRet.AppendAscii( "\n" );
	aRet.AppendAscii( "\"" );
	aRet += aName;
	aRet.AppendAscii( "\":" );
	return aRet;
}

String getBasicObjectTypeName( SbxObject* pObj )
{
	String aName;
	if( pObj )
	{
		SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
		if( pUnoObj )
			aName = getDbgObjectNameImpl( pUnoObj );
	}
	return aName;
}

bool checkUnoObjectType( SbUnoObject* pUnoObj, const ::rtl::OUString& rClass )
{
	Any aToInspectObj = pUnoObj->getUnoAny();
	TypeClass eType = aToInspectObj.getValueType().getTypeClass();
	if( eType != TypeClass_INTERFACE )
		return false;
	const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

	// Return true for XInvocation based objects as interface type names don't count then
	Reference< XInvocation > xInvocation( x, UNO_QUERY );
	if( xInvocation.is() )
		return true;

	bool result = false;
	Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );
	if( xTypeProvider.is() )
	{
        /*  Although interfaces in the ooo.vba namespace obey the IDL rules and
            have a leading 'X', in Basic we want to be able to do something
            like 'Dim wb As Workbooks' or 'Dim lb As MSForms.Label'. Here we
            add a leading 'X' to the class name and a leading dot to the entire
            type name. This results e.g. in '.XWorkbooks' or '.MSForms.XLabel'
            which matches the interface names 'ooo.vba.excel.XWorkbooks' or
            'ooo.vba.msforms.XLabel'.
         */
        ::rtl::OUString aClassName( sal_Unicode( '.' ) );
        sal_Int32 nClassNameDot = rClass.lastIndexOf( '.' );
        if( nClassNameDot >= 0 )
            aClassName += rClass.copy( 0, nClassNameDot + 1 ) + ::rtl::OUString( sal_Unicode( 'X' ) ) + rClass.copy( nClassNameDot + 1 );
        else
            aClassName += ::rtl::OUString( sal_Unicode( 'X' ) ) + rClass;

		Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
		const Type* pTypeArray = aTypeSeq.getConstArray();
		sal_uInt32 nIfaceCount = aTypeSeq.getLength();
		for( sal_uInt32 j = 0 ; j < nIfaceCount ; j++ )
		{
			const Type& rType = pTypeArray[j];

			Reference<XIdlClass> xClass = TypeToIdlClass( rType );
			if( !xClass.is() )
			{
				DBG_ERROR("failed to get XIdlClass for type");
				break;
			}
			::rtl::OUString aInterfaceName = xClass->getName();
			if ( aInterfaceName.equals( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.bridge.oleautomation.XAutomationObject" ) ) ) )
			{
				// there is a hack in the extensions/source/ole/oleobj.cxx  to return the typename of the automation object, lets check if it
				// matches
				Reference< XInvocation > xInv( aToInspectObj, UNO_QUERY );
				if ( xInv.is() )
				{
					rtl::OUString sTypeName;
					xInv->getValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("$GetTypeName") ) ) >>= sTypeName;
					if ( sTypeName.isEmpty() || sTypeName.equals(  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("IDispatch") ) ) )
						// can't check type, leave it pass
						result = true;	
					else
						result = sTypeName.equals( rClass );
				}
				break; // finished checking automation object
			}

            // match interface name with passed class name
			OSL_TRACE("Checking if object implements %s", OUStringToOString( aClassName, RTL_TEXTENCODING_UTF8 ).getStr() );
			if ( (aClassName.getLength() < aInterfaceName.getLength()) &&
                    aInterfaceName.matchIgnoreAsciiCase( aClassName, aInterfaceName.getLength() - aClassName.getLength() ) )
			{
				result = true;
				break;
			}
		}
	}
	return result;
}

// Dbg-Hilfsmethode zum Auslesen der in einem Object implementierten Interfaces
String Impl_GetSupportedInterfaces( SbUnoObject* pUnoObj )
{
	Any aToInspectObj = pUnoObj->getUnoAny();

	// #54898: Nur TypeClass Interface zulasssen
	TypeClass eType = aToInspectObj.getValueType().getTypeClass();
	String aRet;
	if( eType != TypeClass_INTERFACE )
	{
		aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM(ID_DBG_SUPPORTEDINTERFACES) );
		aRet.AppendAscii( " not available.\n(TypeClass is not TypeClass_INTERFACE)\n" );
	}
	else
	{
		// Interface aus dem Any besorgen
		const Reference< XInterface > x = *(Reference< XInterface >*)aToInspectObj.getValue();

		// XIdlClassProvider-Interface ansprechen
		Reference< XIdlClassProvider > xClassProvider( x, UNO_QUERY );
		Reference< XTypeProvider > xTypeProvider( x, UNO_QUERY );

		aRet.AssignAscii( "Supported interfaces by object " );
		String aObjName = getDbgObjectName( pUnoObj );
		aRet += aObjName;
		aRet.AppendAscii( "\n" );
		if( xTypeProvider.is() )
		{
			// Interfaces der Implementation holen
			Sequence< Type > aTypeSeq = xTypeProvider->getTypes();
			const Type* pTypeArray = aTypeSeq.getConstArray();
			sal_uInt32 nIfaceCount = aTypeSeq.getLength();
			for( sal_uInt32 j = 0 ; j < nIfaceCount ; j++ )
			{
				const Type& rType = pTypeArray[j];

                Reference<XIdlClass> xClass = TypeToIdlClass( rType );
                if( xClass.is() )
                {
				    aRet += Impl_GetInterfaceInfo( x, xClass, 1 );
                }
                else
                {
	                typelib_TypeDescription * pTD = 0;
	                rType.getDescription( &pTD );
	                String TypeName( ::rtl::OUString( pTD->pTypeName ) );

                    aRet.AppendAscii( "*** ERROR: No IdlClass for type \"" );
                    aRet += TypeName;
                    aRet.AppendAscii( "\"\n*** Please check type library\n" );
                }
			}
		}
		else if( xClassProvider.is() )
		{

			DBG_ERROR( "XClassProvider not supported in UNO3" );
		}
	}
	return aRet;
}



// Dbg-Hilfsmethode SbxDataType -> String
String Dbg_SbxDataType2String( SbxDataType eType )
{
	String aRet( RTL_CONSTASCII_USTRINGPARAM("Unknown Sbx-Type!") );
	switch( +eType )
	{
		case SbxEMPTY:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxEMPTY") ); break;
		case SbxNULL:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxNULL") ); break;
		case SbxINTEGER:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINTEGER") ); break;
		case SbxLONG:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG") ); break;
		case SbxSINGLE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSINGLE") ); break;
		case SbxDOUBLE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDOUBLE") ); break;
		case SbxCURRENCY:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCURRENCY") ); break;
		case SbxDECIMAL:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDECIMAL") ); break;
		case SbxDATE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATE") ); break;
		case SbxSTRING:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxSTRING") ); break;
		case SbxOBJECT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxOBJECT") ); break;
		case SbxERROR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxERROR") ); break;
		case SbxBOOL:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBOOL") ); break;
		case SbxVARIANT:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVARIANT") ); break;
		case SbxDATAOBJECT: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDATAOBJECT") ); break;
		case SbxCHAR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCHAR") ); break;
		case SbxBYTE:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxBYTE") ); break;
		case SbxUSHORT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSHORT") ); break;
		case SbxULONG:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG") ); break;
		case SbxLONG64:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLONG64") ); break;
		case SbxULONG64:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxULONG64") ); break;
		case SbxSALINT64:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT64") ); break;
		case SbxSALUINT64:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT64") ); break;
		case SbxINT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxINT") ); break;
		case SbxUINT:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUINT") ); break;
		case SbxVOID:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxVOID") ); break;
		case SbxHRESULT:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxHRESULT") ); break;
		case SbxPOINTER:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxPOINTER") ); break;
		case SbxDIMARRAY:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxDIMARRAY") ); break;
		case SbxCARRAY:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCARRAY") ); break;
		case SbxUSERDEF:	aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxUSERDEF") ); break;
		case SbxLPSTR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPSTR") ); break;
		case SbxLPWSTR:		aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxLPWSTR") ); break;
		case SbxCoreSTRING: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxCoreSTRING" ) ); break;
		case SbxOBJECT | SbxARRAY: aRet = String( RTL_CONSTASCII_USTRINGPARAM("SbxARRAY") ); break;
		default: break;
	}
	return aRet;
}

// Dbg-Hilfsmethode zum Anzeigen der Properties eines SbUnoObjects
String Impl_DumpProperties( SbUnoObject* pUnoObj )
{
	String aRet( RTL_CONSTASCII_USTRINGPARAM("Properties of object ") );
	String aObjName = getDbgObjectName( pUnoObj );
	aRet += aObjName;

	// Uno-Infos auswerten, um Arrays zu erkennen
	Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
	if( !xAccess.is() )
	{
		Reference< XInvocation > xInvok = pUnoObj->getInvocation();
		if( xInvok.is() )
			xAccess = xInvok->getIntrospection();
	}
	if( !xAccess.is() )
	{
		aRet.AppendAscii( "\nUnknown, no introspection available\n" );
		return aRet;
	}

	Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
	sal_uInt32 nUnoPropCount = props.getLength();
	const Property* pUnoProps = props.getConstArray();

	SbxArray* pProps = pUnoObj->GetProperties();
	sal_uInt16 nPropCount = pProps->Count();
	sal_uInt16 nPropsPerLine = 1 + nPropCount / 30;
	for( sal_uInt16 i = 0; i < nPropCount; i++ )
	{
		SbxVariable* pVar = pProps->Get( i );
		if( pVar )
		{
			String aPropStr;
			if( (i % nPropsPerLine) == 0 )
				aPropStr.AppendAscii( "\n" );

			// Typ und Namen ausgeben
			// Ist es in Uno eine Sequence?
			SbxDataType eType = pVar->GetFullType();

			sal_Bool bMaybeVoid = sal_False;
			if( i < nUnoPropCount )
			{
				const Property& rProp = pUnoProps[ i ];

				// #63133: Bei MAYBEVOID Typ aus Uno neu konvertieren,
				// damit nicht immer nur SbxEMPTY ausgegben wird.
				if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
				{
					eType = unoToSbxType( rProp.Type.getTypeClass() );
					bMaybeVoid = sal_True;
				}
				if( eType == SbxOBJECT )
				{
					Type aType = rProp.Type;
					if( aType.getTypeClass() == TypeClass_SEQUENCE )
						eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
				}
			}
			aPropStr += Dbg_SbxDataType2String( eType );
			if( bMaybeVoid )
				aPropStr.AppendAscii( "/void" );
			aPropStr.AppendAscii( " " );
			aPropStr += pVar->GetName();

			if( i == nPropCount - 1 )
				aPropStr.AppendAscii( "\n" );
			else
				aPropStr.AppendAscii( "; " );

			aRet += aPropStr;
		}
	}
	return aRet;
}

// Dbg-Hilfsmethode zum Anzeigen der Methoden eines SbUnoObjects
String Impl_DumpMethods( SbUnoObject* pUnoObj )
{
	String aRet( RTL_CONSTASCII_USTRINGPARAM("Methods of object ") );
	String aObjName = getDbgObjectName( pUnoObj );
	aRet += aObjName;

	// XIntrospectionAccess, um die Typen der Parameter auch ausgeben zu koennen
	Reference< XIntrospectionAccess > xAccess = pUnoObj->getIntrospectionAccess();
	if( !xAccess.is() )
	{
		Reference< XInvocation > xInvok = pUnoObj->getInvocation();
		if( xInvok.is() )
			xAccess = xInvok->getIntrospection();
	}
	if( !xAccess.is() )
	{
		aRet.AppendAscii( "\nUnknown, no introspection available\n" );
		return aRet;
	}
	Sequence< Reference< XIdlMethod > > methods = xAccess->getMethods
		( MethodConcept::ALL - MethodConcept::DANGEROUS );
	const Reference< XIdlMethod >* pUnoMethods = methods.getConstArray();

	SbxArray* pMethods = pUnoObj->GetMethods();
	sal_uInt16 nMethodCount = pMethods->Count();
	if( !nMethodCount )
	{
		aRet.AppendAscii( "\nNo methods found\n" );
		return aRet;
	}
	sal_uInt16 nPropsPerLine = 1 + nMethodCount / 30;
	for( sal_uInt16 i = 0; i < nMethodCount; i++ )
	{
		SbxVariable* pVar = pMethods->Get( i );
		if( pVar )
		{
			String aPropStr;
			if( (i % nPropsPerLine) == 0 )
				aPropStr.AppendAscii( "\n" );

			// Methode ansprechen
			const Reference< XIdlMethod >& rxMethod = pUnoMethods[i];

			// Ist es in Uno eine Sequence?
			SbxDataType eType = pVar->GetFullType();
			if( eType == SbxOBJECT )
			{
				Reference< XIdlClass > xClass = rxMethod->getReturnType();
				if( xClass.is() && xClass->getTypeClass() == TypeClass_SEQUENCE )
					eType = (SbxDataType) ( SbxOBJECT | SbxARRAY );
			}
			// Name und Typ ausgeben
			aPropStr += Dbg_SbxDataType2String( eType );
			aPropStr.AppendAscii( " " );
			aPropStr += pVar->GetName();
			aPropStr.AppendAscii( " ( " );

			// get-Methode darf keinen Parameter haben
			Sequence< Reference< XIdlClass > > aParamsSeq = rxMethod->getParameterTypes();
			sal_uInt32 nParamCount = aParamsSeq.getLength();
			const Reference< XIdlClass >* pParams = aParamsSeq.getConstArray();

			if( nParamCount > 0 )
			{
				for( sal_uInt16 j = 0; j < nParamCount; j++ )
				{
					String aTypeStr = Dbg_SbxDataType2String( unoToSbxType( pParams[ j ] ) );
					aPropStr += aTypeStr;

					if( j < nParamCount - 1 )
						aPropStr.AppendAscii( ", " );
				}
			}
			else
				aPropStr.AppendAscii( "void" );

			aPropStr.AppendAscii( " ) " );

			if( i == nMethodCount - 1 )
				aPropStr.AppendAscii( "\n" );
			else
				aPropStr.AppendAscii( "; " );

			aRet += aPropStr;
		}
	}
	return aRet;
}

TYPEINIT1(AutomationNamedArgsSbxArray,SbxArray)

// Implementation SbUnoObject
void SbUnoObject::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
						   const SfxHint& rHint, const TypeId& rHintType )
{
	if( bNeedIntrospection )
		doIntrospection();

	const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
	if( pHint )
	{
		SbxVariable* pVar = pHint->GetVar();
		SbxArray* pParams = pVar->GetParameters();
		SbUnoProperty* pProp = PTR_CAST(SbUnoProperty,pVar);
		SbUnoMethod* pMeth = PTR_CAST(SbUnoMethod,pVar);
		if( pProp )
		{
			bool bInvocation = pProp->isInvocationBased();
			if( pHint->GetId() == SBX_HINT_DATAWANTED )
			{
				// Test-Properties
				sal_Int32 nId = pProp->nId;
				if( nId < 0 )
				{
					// Id == -1: Implementierte Interfaces gemaess ClassProvider anzeigen
					if( nId == -1 )		// Property ID_DBG_SUPPORTEDINTERFACES"
					{
						String aRetStr = Impl_GetSupportedInterfaces( this );
						pVar->PutString( aRetStr );
					}
					// Id == -2: Properties ausgeben
					else if( nId == -2 )		// Property ID_DBG_PROPERTIES
					{
						// Jetzt muessen alle Properties angelegt werden
						implCreateAll();
						String aRetStr = Impl_DumpProperties( this );
						pVar->PutString( aRetStr );
					}
					// Id == -3: Methoden ausgeben
					else if( nId == -3 )		// Property ID_DBG_METHODS
					{
						// Jetzt muessen alle Properties angelegt werden
						implCreateAll();
						String aRetStr = Impl_DumpMethods( this );
						pVar->PutString( aRetStr );
					}
					return;
				}

				if( !bInvocation && mxUnoAccess.is() )
				{
					try
					{
						// Wert holen
						Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
						Any aRetAny = xPropSet->getPropertyValue( pProp->GetName() );
						// Die Nutzung von getPropertyValue (statt ueber den Index zu gehen) ist
						// nicht optimal, aber die Umstellung auf XInvocation steht ja ohnehin an
						// Ansonsten kann auch FastPropertySet genutzt werden

						// Wert von Uno nach Sbx uebernehmen
						unoToSbxValue( pVar, aRetAny );
					}
					catch( const Exception& )
					{
                        implHandleAnyException( ::cppu::getCaughtException() );
					}
				}
				else if( bInvocation && mxInvocation.is() )
				{
					try
					{
						// Wert holen
						Any aRetAny = mxInvocation->getValue( pProp->GetName() );

						// Wert von Uno nach Sbx uebernehmen
						unoToSbxValue( pVar, aRetAny );
					}
					catch( const Exception& )
					{
                        implHandleAnyException( ::cppu::getCaughtException() );
					}
				}
			}
			else if( pHint->GetId() == SBX_HINT_DATACHANGED )
			{
				if( !bInvocation && mxUnoAccess.is() )
				{
					if( pProp->aUnoProp.Attributes & PropertyAttribute::READONLY )
					{
						StarBASIC::Error( SbERR_PROP_READONLY );
						return;
					}

					// Wert von Uno nach Sbx uebernehmen
					Any aAnyValue = sbxToUnoValue( pVar, pProp->aUnoProp.Type, &pProp->aUnoProp );
					try
					{
						// Wert setzen
						Reference< XPropertySet > xPropSet( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
						xPropSet->setPropertyValue( pProp->GetName(), aAnyValue );
						// Die Nutzung von getPropertyValue (statt ueber den Index zu gehen) ist
						// nicht optimal, aber die Umstellung auf XInvocation steht ja ohnehin an
						// Ansonsten kann auch FastPropertySet genutzt werden
					}
					catch( const Exception& )
					{
                        implHandleAnyException( ::cppu::getCaughtException() );
					}
				}
				else if( bInvocation && mxInvocation.is() )
				{
					// Wert von Uno nach Sbx uebernehmen
					Any aAnyValue = sbxToUnoValueImpl( pVar );
					try
					{
						// Wert setzen
						mxInvocation->setValue( pProp->GetName(), aAnyValue );
					}
					catch( const Exception& )
					{
                        implHandleAnyException( ::cppu::getCaughtException() );
					}
				}
			}
		}
		else if( pMeth )
		{
			bool bInvocation = pMeth->isInvocationBased();
			if( pHint->GetId() == SBX_HINT_DATAWANTED )
			{
				// Anzahl Parameter -1 wegen Param0 == this
				sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
				Sequence<Any> args;
				sal_Bool bOutParams = sal_False;
				sal_uInt32 i;

				if( !bInvocation && mxUnoAccess.is() )
				{
					// Infos holen
					const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
					const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
					sal_uInt32 nUnoParamCount = rInfoSeq.getLength();
					sal_uInt32 nAllocParamCount = nParamCount;

					// Ueberschuessige Parameter ignorieren, Alternative: Error schmeissen
					if( nParamCount > nUnoParamCount )
					{
						nParamCount = nUnoParamCount;
						nAllocParamCount = nParamCount;
					}
					else if( nParamCount < nUnoParamCount )
					{
						SbiInstance* pInst = pINST;
						if( pInst && pInst->IsCompatibility() )
						{
							// Check types
							bool bError = false;
							for( i = nParamCount ; i < nUnoParamCount ; i++ )
							{
								const ParamInfo& rInfo = pParamInfos[i];
								const Reference< XIdlClass >& rxClass = rInfo.aType;
								if( rxClass->getTypeClass() != TypeClass_ANY )
								{
									bError = true;
									StarBASIC::Error( SbERR_NOT_OPTIONAL );
								}
							}
							if( !bError )
								nAllocParamCount = nUnoParamCount;
						}
					}

					if( nAllocParamCount > 0 )
					{
						args.realloc( nAllocParamCount );
						Any* pAnyArgs = args.getArray();
						for( i = 0 ; i < nParamCount ; i++ )
						{
							const ParamInfo& rInfo = pParamInfos[i];
							const Reference< XIdlClass >& rxClass = rInfo.aType;
							//const XIdlClassRef& rxClass = pUnoParams[i];

							com::sun::star::uno::Type aType( rxClass->getTypeClass(), rxClass->getName() );

							// ACHTUNG: Bei den Sbx-Parametern den Offset nicht vergessen!
							pAnyArgs[i] = sbxToUnoValue( pParams->Get( (sal_uInt16)(i+1) ), aType );

							// Wenn es nicht schon feststeht pruefen, ob Out-Parameter vorliegen
							if( !bOutParams )
							{
								ParamMode aParamMode = rInfo.aMode;
								if( aParamMode != ParamMode_IN )
									bOutParams = sal_True;
							}
						}
					}
				}
				else if( bInvocation && pParams && mxInvocation.is() )
				{
					bool bOLEAutomation = true;
					// TODO: bOLEAutomation = xOLEAutomation.is()

					AutomationNamedArgsSbxArray* pArgNamesArray = NULL;
					if( bOLEAutomation )
						pArgNamesArray = PTR_CAST(AutomationNamedArgsSbxArray,pParams);

					args.realloc( nParamCount );
					Any* pAnyArgs = args.getArray();
					bool bBlockConversionToSmallestType = pINST->IsCompatibility();
					if( pArgNamesArray )
					{
						Sequence< ::rtl::OUString >& rNameSeq = pArgNamesArray->getNames();
						::rtl::OUString* pNames = rNameSeq.getArray();

						Any aValAny;
						for( i = 0 ; i < nParamCount ; i++ )
						{
							sal_uInt16 iSbx = (sal_uInt16)(i+1);

							// ACHTUNG: Bei den Sbx-Parametern den Offset nicht vergessen!
							aValAny = sbxToUnoValueImpl( pParams->Get( iSbx ),
														bBlockConversionToSmallestType );

							::rtl::OUString aParamName = pNames[iSbx];
							if( !aParamName.isEmpty() )
							{
								oleautomation::NamedArgument aNamedArgument;
								aNamedArgument.Name = aParamName;
								aNamedArgument.Value = aValAny;
								pAnyArgs[i] <<= aNamedArgument;
							}
							else
							{
								pAnyArgs[i] = aValAny;
							}
						}
					}
					else
					{
						for( i = 0 ; i < nParamCount ; i++ )
						{
							// ACHTUNG: Bei den Sbx-Parametern den Offset nicht vergessen!
							pAnyArgs[i] = sbxToUnoValueImpl( pParams->Get( (sal_uInt16)(i+1) ),
															bBlockConversionToSmallestType );
						}
					}
				}

				// Methode callen
                GetSbData()->bBlockCompilerError = sal_True;  // #106433 Block compiler errors for API calls
				try
				{
					if( !bInvocation && mxUnoAccess.is() )
					{
						Any aRetAny = pMeth->m_xUnoMethod->invoke( getUnoAny(), args );

						// Wert von Uno nach Sbx uebernehmen
						unoToSbxValue( pVar, aRetAny );

						// Muessen wir Out-Parameter zurueckkopieren?
						if( bOutParams )
						{
							const Any* pAnyArgs = args.getConstArray();

							// Infos holen
							const Sequence<ParamInfo>& rInfoSeq = pMeth->getParamInfos();
							const ParamInfo* pParamInfos = rInfoSeq.getConstArray();

							sal_uInt32 j;
							for( j = 0 ; j < nParamCount ; j++ )
							{
								const ParamInfo& rInfo = pParamInfos[j];
								ParamMode aParamMode = rInfo.aMode;
								if( aParamMode != ParamMode_IN )
									unoToSbxValue( (SbxVariable*)pParams->Get( (sal_uInt16)(j+1) ), pAnyArgs[ j ] );
							}
						}
					}
					else if( bInvocation && mxInvocation.is() )
					{
                        Reference< XDirectInvocation > xDirectInvoke;
                        if ( pMeth->needsDirectInvocation() )
                            xDirectInvoke.set( mxInvocation, UNO_QUERY );
                        
                        Any aRetAny;
                        if ( xDirectInvoke.is() )
                            aRetAny = xDirectInvoke->directInvoke( pMeth->GetName(), args );
                        else
                        {
                            Sequence< sal_Int16 > OutParamIndex;
                            Sequence< Any > OutParam;
                            aRetAny = mxInvocation->invoke( pMeth->GetName(), args, OutParamIndex, OutParam );

                            const sal_Int16* pIndices = OutParamIndex.getConstArray();
                            sal_uInt32 nLen = OutParamIndex.getLength();
                            if( nLen )
                            {
                                const Any* pNewValues = OutParam.getConstArray();
                                for( sal_uInt32 j = 0 ; j < nLen ; j++ )
                                {
                                    sal_Int16 iTarget = pIndices[ j ];
                                    if( iTarget >= (sal_Int16)nParamCount )
                                        break;
                                    unoToSbxValue( (SbxVariable*)pParams->Get( (sal_uInt16)(j+1) ), pNewValues[ j ] );
                                }
                            }
                        }

                        // Wert von Uno nach Sbx uebernehmen
                        unoToSbxValue( pVar, aRetAny );
					}

					// #55460, Parameter hier weghauen, da das in unoToSbxValue()
					// bei Arrays wegen #54548 nicht mehr gemacht wird
					if( pParams )
						pVar->SetParameters( NULL );
				}
				catch( const Exception& )
				{
                    implHandleAnyException( ::cppu::getCaughtException() );
				}
                GetSbData()->bBlockCompilerError = sal_False;  // #106433 Unblock compiler errors
			}
		}
		else
			SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
	}
}


#ifdef INVOCATION_ONLY
// Aus USR
Reference< XInvocation > createDynamicInvocationFor( const Any& aAny );
#endif

SbUnoObject::SbUnoObject( const String& aName_, const Any& aUnoObj_ )
	: SbxObject( aName_ )
	, bNeedIntrospection( sal_True )
	, bNativeCOMObject( sal_False )
{
	static Reference< XIntrospection > xIntrospection;

	// Default-Properties von Sbx wieder rauspruegeln
	Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Name") ), SbxCLASS_DONTCARE );
	Remove( XubString( RTL_CONSTASCII_USTRINGPARAM("Parent") ), SbxCLASS_DONTCARE );

	// Typ des Objekts pruefen
	TypeClass eType = aUnoObj_.getValueType().getTypeClass();
	Reference< XInterface > x;
	if( eType == TypeClass_INTERFACE )
	{
		// Interface aus dem Any besorgen
		x = *(Reference< XInterface >*)aUnoObj_.getValue();
		if( !x.is() )
			return;
	}

	Reference< XTypeProvider > xTypeProvider;
#ifdef INVOCATION_ONLY
	// Invocation besorgen
	mxInvocation = createDynamicInvocationFor( aUnoObj_ );
#else
	// Hat das Object selbst eine Invocation?
	mxInvocation = Reference< XInvocation >( x, UNO_QUERY );

	xTypeProvider = Reference< XTypeProvider >( x, UNO_QUERY );
#endif

	if( mxInvocation.is() )
	{
		// #94670: This is WRONG because then the MaterialHolder doesn't refer
		// to the object implementing XInvocation but to the object passed to
		// the invocation service!!!
		// mxMaterialHolder = Reference< XMaterialHolder >::query( mxInvocation );

		// ExactName holen
		mxExactNameInvocation = Reference< XExactName >::query( mxInvocation );

		// Rest bezieht sich nur auf Introspection
		if( !xTypeProvider.is() )
		{
			bNeedIntrospection = sal_False;
			return;
		}

		// Ignore introspection based members for COM objects to avoid
		// hiding of equally named COM symbols, e.g. XInvocation::getValue
		Reference< oleautomation::XAutomationObject > xAutomationObject( aUnoObj_, UNO_QUERY );
		if( xAutomationObject.is() )
			bNativeCOMObject = sal_True;
	}

	maTmpUnoObj = aUnoObj_;


	//*** Namen bestimmen ***
	sal_Bool bFatalError = sal_True;

	// Ist es ein Interface oder eine struct?
	sal_Bool bSetClassName = sal_False;
	String aClassName_;
	if( eType == TypeClass_STRUCT || eType == TypeClass_EXCEPTION )
	{
		// Struct ist Ok
		bFatalError = sal_False;

		// #67173 Echten Klassen-Namen eintragen
		if( aName_.Len() == 0 )
		{
			aClassName_ = String( aUnoObj_.getValueType().getTypeName() );
			bSetClassName = sal_True;
		}
	}
	else if( eType == TypeClass_INTERFACE )
	{
		// #70197 Interface geht immer durch Typ im Any
		bFatalError = sal_False;

		// Nach XIdlClassProvider-Interface fragen
		Reference< XIdlClassProvider > xClassProvider( x, UNO_QUERY );
		if( xClassProvider.is() )
		{
			// #67173 Echten Klassen-Namen eintragen
			if( aName_.Len() == 0 )
			{
				Sequence< Reference< XIdlClass > > szClasses = xClassProvider->getIdlClasses();
				sal_uInt32 nLen = szClasses.getLength();
				if( nLen )
				{
					const Reference< XIdlClass > xImplClass = szClasses.getConstArray()[ 0 ];
					if( xImplClass.is() )
					{
						aClassName_ = String( xImplClass->getName() );
						bSetClassName = sal_True;
					}
				}
			}
		}
	}
	if( bSetClassName )
		SetClassName( aClassName_ );

	// Weder Interface noch Struct -> FatalError
	if( bFatalError )
	{
		StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
		return;
	}

	// #67781 Introspection erst on demand durchfuehren
}

SbUnoObject::~SbUnoObject()
{
}


// #76470 Introspection on Demand durchfuehren
void SbUnoObject::doIntrospection( void )
{
	static Reference< XIntrospection > xIntrospection;

	if( !bNeedIntrospection )
		return;
	bNeedIntrospection = sal_False;

	if( !xIntrospection.is() )
	{
		// Introspection-Service holen
		Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
		if ( xFactory.is() )
		{
			Reference< XInterface > xI = xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.beans.Introspection") );
			if (xI.is())
				xIntrospection = Reference< XIntrospection >::query( xI );
				//xI->queryInterface( ::getCppuType( (const Reference< XIntrospection > *)0 ), xIntrospection );
		}
	}
	if( !xIntrospection.is() )
	{
		StarBASIC::FatalError( ERRCODE_BASIC_EXCEPTION );
		return;
	}

	// Introspection durchfuehren
	try
	{
		mxUnoAccess = xIntrospection->inspect( maTmpUnoObj );
	}
	catch( RuntimeException& e )
	{
        StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
	}

	if( !mxUnoAccess.is() )
	{
		// #51475 Ungueltiges Objekt kennzeichnen (kein mxMaterialHolder)
		return;
	}

	// MaterialHolder vom Access holen
	mxMaterialHolder = Reference< XMaterialHolder >::query( mxUnoAccess );

	// ExactName vom Access holen
	mxExactName = Reference< XExactName >::query( mxUnoAccess );
}




// #67781 Start einer Liste aller SbUnoMethod-Instanzen
static SbUnoMethod* pFirst = NULL;

void clearUnoMethodsForBasic( StarBASIC* pBasic )
{
	SbUnoMethod* pMeth = pFirst;
	while( pMeth )
	{
        SbxObject* pObject = dynamic_cast< SbxObject* >( pMeth->GetParent() );
        if ( pObject )
        { 
            StarBASIC* pModBasic = dynamic_cast< StarBASIC* >( pObject->GetParent() );
            if ( pModBasic == pBasic )
            {
                // for now the solution is to remove the method from the list and to clear it,
                // but in case the element should be correctly transferred to another StarBASIC,
                // we should either set module parent to NULL without clearing it, or even
                // set the new StarBASIC as the parent of the module
                // pObject->SetParent( NULL );
                
                if( pMeth == pFirst )
                    pFirst = pMeth->pNext;
                else if( pMeth->pPrev )
                    pMeth->pPrev->pNext = pMeth->pNext;
                if( pMeth->pNext )
                    pMeth->pNext->pPrev = pMeth->pPrev;

                pMeth->pPrev = NULL;
                pMeth->pNext = NULL;

                pMeth->SbxValue::Clear();
                pObject->SbxValue::Clear();

                // start from the beginning after object clearing, the cycle will end since the method is removed each time
                pMeth = pFirst;
            }
            else
                pMeth = pMeth->pNext;
        }
        else
            pMeth = pMeth->pNext;
	}
}

void clearUnoMethods( void )
{
	SbUnoMethod* pMeth = pFirst;
	while( pMeth )
	{
		pMeth->SbxValue::Clear();
		pMeth = pMeth->pNext;
	}
}


SbUnoMethod::SbUnoMethod
(
	const String& aName_,
	SbxDataType eSbxType,
	Reference< XIdlMethod > xUnoMethod_,
	bool bInvocation,
    bool bDirect
)
	: SbxMethod( aName_, eSbxType )
	, mbInvocation( bInvocation )
    , mbDirectInvocation( bDirect )
{
	m_xUnoMethod = xUnoMethod_;
	pParamInfoSeq = NULL;

	// #67781 Methode in Liste eintragen
	pNext = pFirst;
	pPrev = NULL;
	pFirst = this;
	if( pNext )
		pNext->pPrev = this;
}

SbUnoMethod::~SbUnoMethod()
{
	delete pParamInfoSeq;

	if( this == pFirst )
		pFirst = pNext;
	else if( pPrev )
		pPrev->pNext = pNext;
	if( pNext )
		pNext->pPrev = pPrev;
}

SbxInfo* SbUnoMethod::GetInfo()
{
	if( !pInfo && m_xUnoMethod.is() )
	{
		SbiInstance* pInst = pINST;
		if( pInst && pInst->IsCompatibility() )
		{
			pInfo = new SbxInfo();

			const Sequence<ParamInfo>& rInfoSeq = getParamInfos();
			const ParamInfo* pParamInfos = rInfoSeq.getConstArray();
			sal_uInt32 nParamCount = rInfoSeq.getLength();

			for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
			{
				const ParamInfo& rInfo = pParamInfos[i];
				::rtl::OUString aParamName = rInfo.aName;

				// const Reference< XIdlClass >& rxClass = rInfo.aType;
				SbxDataType t = SbxVARIANT;
				sal_uInt16 nFlags_ = SBX_READ;
				pInfo->AddParam( aParamName, t, nFlags_ );
			}
		}
	}
	return pInfo;
}

const Sequence<ParamInfo>& SbUnoMethod::getParamInfos( void )
{
	if( !pParamInfoSeq && m_xUnoMethod.is() )
	{
		Sequence<ParamInfo> aTmp = m_xUnoMethod->getParameterInfos() ;
		pParamInfoSeq = new Sequence<ParamInfo>( aTmp );
	}
	return *pParamInfoSeq;
}

SbUnoProperty::SbUnoProperty
(
	const String& aName_,
	SbxDataType eSbxType,
	const Property& aUnoProp_,
	sal_Int32 nId_,
	bool bInvocation
)
	: SbxProperty( aName_, eSbxType )
	, aUnoProp( aUnoProp_ )
	, nId( nId_ )
	, mbInvocation( bInvocation )
{
	// #54548, bei bedarf Dummy-Array einsetzen, damit SbiRuntime::CheckArray() geht
	static SbxArrayRef xDummyArray = new SbxArray( SbxVARIANT );
	if( eSbxType & SbxARRAY )
		PutObject( xDummyArray );
}

SbUnoProperty::~SbUnoProperty()
{}


SbxVariable* SbUnoObject::Find( const String& rName, SbxClassType t )
{
	static Reference< XIdlMethod > xDummyMethod;
	static Property aDummyProp;

	SbxVariable* pRes = SbxObject::Find( rName, t );

	if( bNeedIntrospection )
		doIntrospection();

	// Neu 4.3.1999: Properties on Demand anlegen, daher jetzt perIntrospectionAccess
	// suchen, ob doch eine Property oder Methode des geforderten Namens existiert
	if( !pRes )
	{
		::rtl::OUString aUName( rName );
		if( mxUnoAccess.is() && !bNativeCOMObject )
		{
			if( mxExactName.is() )
			{
				::rtl::OUString aUExactName = mxExactName->getExactName( aUName );
				if( !aUExactName.isEmpty() )
					aUName = aUExactName;
			}
			if( mxUnoAccess->hasProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS ) )
			{
				const Property& rProp = mxUnoAccess->
					getProperty( aUName, PropertyConcept::ALL - PropertyConcept::DANGEROUS );

				// #58455 Wenn die Property void sein kann, muss als Typ Variant gesetzt werden
				SbxDataType eSbxType;
				if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
					eSbxType = SbxVARIANT;
				else
					eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

				// Property anlegen und reinbraten
				SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, 0, false );
				QuickInsert( (SbxVariable*)xVarRef );
				pRes = xVarRef;
			}
			else if( mxUnoAccess->hasMethod( aUName,
				MethodConcept::ALL - MethodConcept::DANGEROUS ) )
			{
				// Methode ansprechen
				const Reference< XIdlMethod >& rxMethod = mxUnoAccess->
					getMethod( aUName, MethodConcept::ALL - MethodConcept::DANGEROUS );

				// SbUnoMethode anlegen und reinbraten
				SbxVariableRef xMethRef = new SbUnoMethod( rxMethod->getName(),
					unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
				QuickInsert( (SbxVariable*)xMethRef );
				pRes = xMethRef;
			}

			// Wenn immer noch nichts gefunden wurde, muss geprueft werden, ob NameAccess vorliegt
			if( !pRes )
			{
				try
				{
					Reference< XNameAccess > xNameAccess( mxUnoAccess->queryAdapter( ::getCppuType( (const Reference< XPropertySet > *)0 ) ), UNO_QUERY );
					::rtl::OUString aUName2( rName );

					if( xNameAccess.is() && xNameAccess->hasByName( aUName2 ) )
					{
						Any aAny = xNameAccess->getByName( aUName2 );

						// ACHTUNG: Die hier erzeugte Variable darf wegen bei XNameAccess
						// nicht als feste Property in das Object aufgenommen werden und
						// wird daher nirgendwo gehalten.
						// Wenn das Probleme gibt, muss das kuenstlich gemacht werden oder
						// es muss eine Klasse SbUnoNameAccessProperty geschaffen werden,
						// bei der die Existenz staendig neu ueberprueft und die ggf. weg-
						// geworfen wird, wenn der Name nicht mehr gefunden wird.
						pRes = new SbxVariable( SbxVARIANT );
						unoToSbxValue( pRes, aAny );
					}
				}
				catch( NoSuchElementException& e )
				{
					StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
				}
				catch( const Exception& )
				{
					// Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
					if( !pRes )
						pRes = new SbxVariable( SbxVARIANT );

                    implHandleAnyException( ::cppu::getCaughtException() );
				}
			}
		}
		if( !pRes && mxInvocation.is() )
		{
			if( mxExactNameInvocation.is() )
			{
				::rtl::OUString aUExactName = mxExactNameInvocation->getExactName( aUName );
				if( !aUExactName.isEmpty() )
					aUName = aUExactName;
			}

			try
			{
				if( mxInvocation->hasProperty( aUName ) )
				{
					// Property anlegen und reinbraten
					SbxVariableRef xVarRef = new SbUnoProperty( aUName, SbxVARIANT, aDummyProp, 0, true );
					QuickInsert( (SbxVariable*)xVarRef );
					pRes = xVarRef;
				}
				else if( mxInvocation->hasMethod( aUName ) )
				{
					// SbUnoMethode anlegen und reinbraten
					SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod, true );
					QuickInsert( (SbxVariable*)xMethRef );
					pRes = xMethRef;
				}
                else
                {
                    Reference< XDirectInvocation > xDirectInvoke( mxInvocation, UNO_QUERY );
                    if ( xDirectInvoke.is() && xDirectInvoke->hasMember( aUName ) )
                    {
                        SbxVariableRef xMethRef = new SbUnoMethod( aUName, SbxVARIANT, xDummyMethod, true, true );
                        QuickInsert( (SbxVariable*)xMethRef );
                        pRes = xMethRef;
                    }

                }
			}
			catch( RuntimeException& e )
			{
				// Anlegen, damit der Exception-Fehler nicht ueberschrieben wird
				if( !pRes )
					pRes = new SbxVariable( SbxVARIANT );

				StarBASIC::Error( ERRCODE_BASIC_EXCEPTION, implGetExceptionMsg( e ) );
			}
		}
	}

	// Ganz am Schluss noch pruefen, ob die Dbg_-Properties gemeint sind

	if( !pRes )
	{
		if( rName.EqualsIgnoreCaseAscii( ID_DBG_SUPPORTEDINTERFACES ) ||
			rName.EqualsIgnoreCaseAscii( ID_DBG_PROPERTIES ) ||
			rName.EqualsIgnoreCaseAscii( ID_DBG_METHODS ) )
		{
			// Anlegen
			implCreateDbgProperties();

			// Jetzt muessen sie regulaer gefunden werden
			pRes = SbxObject::Find( rName, SbxCLASS_DONTCARE );
		}
	}
	return pRes;
}


// Hilfs-Methode zum Anlegen der dbg_-Properties
void SbUnoObject::implCreateDbgProperties( void )
{
	Property aProp;

	// Id == -1: Implementierte Interfaces gemaess ClassProvider anzeigen
	SbxVariableRef xVarRef = new SbUnoProperty( String(RTL_CONSTASCII_USTRINGPARAM(ID_DBG_SUPPORTEDINTERFACES)), SbxSTRING, aProp, -1, false );
	QuickInsert( (SbxVariable*)xVarRef );

	// Id == -2: Properties ausgeben
	xVarRef = new SbUnoProperty( String(RTL_CONSTASCII_USTRINGPARAM(ID_DBG_PROPERTIES)), SbxSTRING, aProp, -2, false );
	QuickInsert( (SbxVariable*)xVarRef );

	// Id == -3: Methoden ausgeben
	xVarRef = new SbUnoProperty( String(RTL_CONSTASCII_USTRINGPARAM(ID_DBG_METHODS)), SbxSTRING, aProp, -3, false );
	QuickInsert( (SbxVariable*)xVarRef );
}

void SbUnoObject::implCreateAll( void )
{
	// Bestehende Methoden und Properties alle wieder wegwerfen
	pMethods   = new SbxArray;
	pProps     = new SbxArray;

	if( bNeedIntrospection ) doIntrospection();

	// Instrospection besorgen
	Reference< XIntrospectionAccess > xAccess = mxUnoAccess;
	if( !xAccess.is() || bNativeCOMObject )
	{
		if( mxInvocation.is() )
			xAccess = mxInvocation->getIntrospection();
		else if( bNativeCOMObject )
			return;
	}
	if( !xAccess.is() )
		return;

	// Properties anlegen
	Sequence<Property> props = xAccess->getProperties( PropertyConcept::ALL - PropertyConcept::DANGEROUS );
	sal_uInt32 nPropCount = props.getLength();
	const Property* pProps_ = props.getConstArray();

	sal_uInt32 i;
	for( i = 0 ; i < nPropCount ; i++ )
	{
		const Property& rProp = pProps_[ i ];

		// #58455 Wenn die Property void sein kann, muss als Typ Variant gesetzt werden
		SbxDataType eSbxType;
		if( rProp.Attributes & PropertyAttribute::MAYBEVOID )
			eSbxType = SbxVARIANT;
		else
			eSbxType = unoToSbxType( rProp.Type.getTypeClass() );

		// Property anlegen und reinbraten
		SbxVariableRef xVarRef = new SbUnoProperty( rProp.Name, eSbxType, rProp, i, false );
		QuickInsert( (SbxVariable*)xVarRef );
	}

	// Dbg_-Properties anlegen
	implCreateDbgProperties();

	// Methoden anlegen
	Sequence< Reference< XIdlMethod > > aMethodSeq = xAccess->getMethods
		( MethodConcept::ALL - MethodConcept::DANGEROUS );
	sal_uInt32 nMethCount = aMethodSeq.getLength();
	const Reference< XIdlMethod >* pMethods_ = aMethodSeq.getConstArray();
	for( i = 0 ; i < nMethCount ; i++ )
	{
		// Methode ansprechen
		const Reference< XIdlMethod >& rxMethod = pMethods_[i];

		// SbUnoMethode anlegen und reinbraten
		SbxVariableRef xMethRef = new SbUnoMethod
			( rxMethod->getName(), unoToSbxType( rxMethod->getReturnType() ), rxMethod, false );
		QuickInsert( (SbxVariable*)xMethRef );
	}
}


// Wert rausgeben
Any SbUnoObject::getUnoAny( void )
{
	Any aRetAny;
	if( bNeedIntrospection ) doIntrospection();
	if( mxMaterialHolder.is() )
		aRetAny = mxMaterialHolder->getMaterial();
	else if( mxInvocation.is() )
		aRetAny <<= mxInvocation;
	return aRetAny;
}

// Hilfsmethode zum Anlegen einer Uno-Struct per CoreReflection
SbUnoObject* Impl_CreateUnoStruct( const String& aClassName )
{
	// CoreReflection holen
	Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
	if( !xCoreReflection.is() )
		return NULL;

	// Klasse suchen
	Reference< XIdlClass > xClass;
	Reference< XHierarchicalNameAccess > xHarryName =
		getCoreReflection_HierarchicalNameAccess_Impl();
	if( xHarryName.is() && xHarryName->hasByHierarchicalName( aClassName ) )
		xClass = xCoreReflection->forName( aClassName );
	if( !xClass.is() )
		return NULL;

	// Ist es ueberhaupt ein struct?
	TypeClass eType = xClass->getTypeClass();
	if ( ( eType != TypeClass_STRUCT ) && ( eType != TypeClass_EXCEPTION ) )
		return NULL;

	// Instanz erzeugen
	Any aNewAny;
	xClass->createObject( aNewAny );

	// SbUnoObject daraus basteln
	SbUnoObject* pUnoObj = new SbUnoObject( aClassName, aNewAny );
	return pUnoObj;
}


// Factory-Klasse fuer das Anlegen von Uno-Structs per DIM AS NEW
SbxBase* SbUnoFactory::Create( sal_uInt16, sal_uInt32 )
{
	// Ueber SbxId laeuft in Uno nix
	return NULL;
}

SbxObject* SbUnoFactory::CreateObject( const String& rClassName )
{
	return Impl_CreateUnoStruct( rClassName );
}


// Provisorische Schnittstelle fuer UNO-Anbindung
// Liefert ein SbxObject, das ein Uno-Interface wrappt
SbxObjectRef GetSbUnoObject( const String& aName, const Any& aUnoObj_ )
{
	return new SbUnoObject( aName, aUnoObj_ );
}

// Force creation of all properties for debugging
void createAllObjectProperties( SbxObject* pObj )
{
	if( !pObj )
		return;

	SbUnoObject* pUnoObj = PTR_CAST(SbUnoObject,pObj);
	if( pUnoObj )
		pUnoObj->createAllProperties();
	else
		pObj->GetAll( SbxCLASS_DONTCARE );
}


void RTL_Impl_CreateUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	// Wir brauchen mindestens 1 Parameter
	if ( rPar.Count() < 2 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Klassen-Name der struct holen
	String aClassName = rPar.Get(1)->GetString();

	// Versuchen, gleichnamige Struct zu erzeugen
	SbUnoObjectRef xUnoObj = Impl_CreateUnoStruct( aClassName );
	if( !xUnoObj )
		return;

	// Objekt zurueckliefern
	SbxVariableRef refVar = rPar.Get(0);
	refVar->PutObject( (SbUnoObject*)xUnoObj );
}

void RTL_Impl_CreateUnoService( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	// Wir brauchen mindestens 1 Parameter
	if ( rPar.Count() < 2 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Klassen-Name der struct holen
	String aServiceName = rPar.Get(1)->GetString();

	// Service suchen und instanzieren
	Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
	Reference< XInterface > xInterface;
	if ( xFactory.is() )
	{
		try
		{
			xInterface = xFactory->createInstance( aServiceName );
		}
		catch( const Exception& )
		{
            implHandleAnyException( ::cppu::getCaughtException() );
		}
	}

	SbxVariableRef refVar = rPar.Get(0);
	if( xInterface.is() )
	{
		Any aAny;
		aAny <<= xInterface;

		// SbUnoObject daraus basteln und zurueckliefern
		SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
		if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
		{
			// Objekt zurueckliefern
			refVar->PutObject( (SbUnoObject*)xUnoObj );
		}
		else
		{
			refVar->PutObject( NULL );
		}
	}
	else
	{
		refVar->PutObject( NULL );
	}
}

void RTL_Impl_CreateUnoServiceWithArguments( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;
    
	// Wir brauchen mindestens 2 Parameter
	if ( rPar.Count() < 3 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Klassen-Name der struct holen
	String aServiceName = rPar.Get(1)->GetString();
	Any aArgAsAny = sbxToUnoValue( rPar.Get(2),
				getCppuType( (Sequence<Any>*)0 ) );
	Sequence< Any > aArgs;
	aArgAsAny >>= aArgs;

	// Service suchen und instanzieren
	Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
	Reference< XInterface > xInterface;
	if ( xFactory.is() )
	{
		try
		{
			xInterface = xFactory->createInstanceWithArguments( aServiceName, aArgs );
		}
		catch( const Exception& )
		{
            implHandleAnyException( ::cppu::getCaughtException() );
		}
	}

	SbxVariableRef refVar = rPar.Get(0);
	if( xInterface.is() )
	{
		Any aAny;
		aAny <<= xInterface;

		// SbUnoObject daraus basteln und zurueckliefern
		SbUnoObjectRef xUnoObj = new SbUnoObject( aServiceName, aAny );
		if( xUnoObj->getUnoAny().getValueType().getTypeClass() != TypeClass_VOID )
		{
			// Objekt zurueckliefern
			refVar->PutObject( (SbUnoObject*)xUnoObj );
		}
		else
		{
			refVar->PutObject( NULL );
		}
	}
	else
	{
		refVar->PutObject( NULL );
	}
}

void RTL_Impl_GetProcessServiceManager( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	SbxVariableRef refVar = rPar.Get(0);

	// Globalen Service-Manager holen
	Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
	if( xFactory.is() )
	{
		Any aAny;
		aAny <<= xFactory;

		// SbUnoObject daraus basteln und zurueckliefern
		SbUnoObjectRef xUnoObj = new SbUnoObject( String( RTL_CONSTASCII_USTRINGPARAM("ProcessServiceManager") ), aAny );
		refVar->PutObject( (SbUnoObject*)xUnoObj );
	}
	else
	{
		refVar->PutObject( NULL );
	}
}

void RTL_Impl_HasInterfaces( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	// Wir brauchen mindestens 2 Parameter
	sal_uInt16 nParCount = rPar.Count();
	if( nParCount < 3 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Variable fuer Rueckgabewert
	SbxVariableRef refVar = rPar.Get(0);
	refVar->PutBool( sal_False );

	// Uno-Objekt holen
	SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
	if( !(pObj && pObj->ISA(SbUnoObject)) )
		return;
	Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
	TypeClass eType = aAny.getValueType().getTypeClass();
	if( eType != TypeClass_INTERFACE )
		return;

	// Interface aus dem Any besorgen
	Reference< XInterface > x = *(Reference< XInterface >*)aAny.getValue();

	// CoreReflection holen
	Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
	if( !xCoreReflection.is() )
		return;

	for( sal_uInt16 i = 2 ; i < nParCount ; i++ )
	{
		// Interface-Name der struct holen
		String aIfaceName = rPar.Get( i )->GetString();

		// Klasse suchen
		Reference< XIdlClass > xClass = xCoreReflection->forName( aIfaceName );
		if( !xClass.is() )
			return;

		// Pruefen, ob das Interface unterstuetzt wird
		::rtl::OUString aClassName = xClass->getName();
		Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
		if( !x->queryInterface( aClassType ).hasValue() )
			return;
	}

	// Alles hat geklappt, dann sal_True liefern
	refVar->PutBool( sal_True );
}

void RTL_Impl_IsUnoStruct( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	// Wir brauchen mindestens 1 Parameter
	if ( rPar.Count() < 2 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Variable fuer Rueckgabewert
	SbxVariableRef refVar = rPar.Get(0);
	refVar->PutBool( sal_False );

	// Uno-Objekt holen
	SbxVariableRef xParam = rPar.Get( 1 );
	if( !xParam->IsObject() )
		return;
	SbxBaseRef pObj = (SbxBase*)rPar.Get( 1 )->GetObject();
	if( !(pObj && pObj->ISA(SbUnoObject)) )
		return;
	Any aAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
	TypeClass eType = aAny.getValueType().getTypeClass();
	if( eType == TypeClass_STRUCT )
		refVar->PutBool( sal_True );
}


void RTL_Impl_EqualUnoObjects( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	if ( rPar.Count() < 3 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Variable fuer Rueckgabewert
	SbxVariableRef refVar = rPar.Get(0);
	refVar->PutBool( sal_False );

	// Uno-Objekte holen
	SbxVariableRef xParam1 = rPar.Get( 1 );
	if( !xParam1->IsObject() )
		return;
	SbxBaseRef pObj1 = (SbxBase*)xParam1->GetObject();
	if( !(pObj1 && pObj1->ISA(SbUnoObject)) )
		return;
	Any aAny1 = ((SbUnoObject*)(SbxBase*)pObj1)->getUnoAny();
	TypeClass eType1 = aAny1.getValueType().getTypeClass();
	if( eType1 != TypeClass_INTERFACE )
		return;
	Reference< XInterface > x1;
	aAny1 >>= x1;
	//XInterfaceRef x1 = *(XInterfaceRef*)aAny1.get();

	SbxVariableRef xParam2 = rPar.Get( 2 );
	if( !xParam2->IsObject() )
		return;
	SbxBaseRef pObj2 = (SbxBase*)xParam2->GetObject();
	if( !(pObj2 && pObj2->ISA(SbUnoObject)) )
		return;
	Any aAny2 = ((SbUnoObject*)(SbxBase*)pObj2)->getUnoAny();
	TypeClass eType2 = aAny2.getValueType().getTypeClass();
	if( eType2 != TypeClass_INTERFACE )
		return;
	Reference< XInterface > x2;
	aAny2 >>= x2;
	//XInterfaceRef x2 = *(XInterfaceRef*)aAny2.get();

	if( x1 == x2 )
		refVar->PutBool( sal_True );
}

typedef std::hash_map< ::rtl::OUString, std::vector< ::rtl::OUString >, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > ModuleHash;


// helper wrapper function to interact with TypeProvider and
// XTypeDescriptionEnumerationAccess.
// if it fails for whatever reason
// returned Reference<> be null e.g. .is() will be false

Reference< XTypeDescriptionEnumeration >
getTypeDescriptorEnumeration( const ::rtl::OUString& sSearchRoot,
	const Sequence< TypeClass >& types, TypeDescriptionSearchDepth depth )
{
	Reference< XTypeDescriptionEnumeration > xEnum;
	Reference< XTypeDescriptionEnumerationAccess> xTypeEnumAccess( getTypeProvider_Impl(), UNO_QUERY );
	if ( xTypeEnumAccess.is() )
	{
		try
		{
			xEnum = xTypeEnumAccess->createTypeDescriptionEnumeration(
				sSearchRoot, types, depth );
		}
		catch( NoSuchTypeNameException& /*nstne*/ ) {}
		catch( InvalidTypeNameException& /*nstne*/ ) {}
	}
	return xEnum;
}

typedef std::hash_map< ::rtl::OUString, Any, ::rtl::OUStringHash, ::std::equal_to< ::rtl::OUString > > VBAConstantsHash;

SbxVariable* getVBAConstant( const String& rName )
{
	SbxVariable* pConst = NULL;
	static VBAConstantsHash aConstCache;
	static bool isInited = false;
	if ( !isInited )
	{
		Sequence< TypeClass > types(1);
		types[ 0 ] = TypeClass_CONSTANTS;
		Reference< XTypeDescriptionEnumeration > xEnum = getTypeDescriptorEnumeration( defaultNameSpace, types, TypeDescriptionSearchDepth_INFINITE  );

		if ( !xEnum.is() )
			return NULL;

		while ( xEnum->hasMoreElements() )
		{
			Reference< XConstantsTypeDescription > xConstants( xEnum->nextElement(), UNO_QUERY );
			if ( xConstants.is() )
			{
				Sequence< Reference< XConstantTypeDescription > > aConsts = xConstants->getConstants();
				Reference< XConstantTypeDescription >* pSrc = aConsts.getArray();
				sal_Int32 nLen = aConsts.getLength();
				for ( sal_Int32 index =0;  index<nLen; ++pSrc, ++index )
				{
					Reference< XConstantTypeDescription >& rXConst =
						*pSrc;
					::rtl::OUString sFullName = rXConst->getName();
					sal_Int32 indexLastDot = sFullName.lastIndexOf('.');
					::rtl::OUString sLeafName;
					if ( indexLastDot > -1 )
						sLeafName = sFullName.copy( indexLastDot + 1);
					aConstCache[ sLeafName.toAsciiLowerCase() ] = rXConst->getConstantValue();
				}
			}
		}
		isInited = true;
	}
	::rtl::OUString sKey( rName );
	VBAConstantsHash::const_iterator it = aConstCache.find( sKey.toAsciiLowerCase() );
	if ( it != aConstCache.end() )
	{
		pConst = new SbxVariable( SbxVARIANT );
		pConst->SetName( rName );
		unoToSbxValue( pConst, it->second );
	}
	return pConst;
}

// Funktion, um einen globalen Bezeichner im
// UnoScope zu suchen und fuer Sbx zu wrappen
SbUnoClass* findUnoClass( const String& rName )
{
    // #105550 Check if module exists
	SbUnoClass* pUnoClass = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
		Reference< XTypeDescription > xTypeDesc;
		aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_MODULE || eTypeClass == TypeClass_CONSTANTS )
        		pUnoClass = new SbUnoClass( rName );
        }
    }
	return pUnoClass;
}

SbxVariable* SbUnoClass::Find( const XubString& rName, SbxClassType t )
{
    (void)t;

	SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_VARIABLE );

	// Wenn nichts gefunden wird, ist das Sub-Modul noch nicht bekannt
	if( !pRes )
	{
		// Wenn es schon eine Klasse ist, nach einen Feld fragen
		if( m_xClass.is() )
		{
			// Ist es ein Field
			::rtl::OUString aUStr( rName );
			Reference< XIdlField > xField = m_xClass->getField( aUStr );
			Reference< XIdlClass > xClass;
			if( xField.is() )
			{
				try
				{
					Any aAny;
					aAny = xField->get( aAny );

					// Nach Sbx wandeln
					pRes = new SbxVariable( SbxVARIANT );
					pRes->SetName( rName );
					unoToSbxValue( pRes, aAny );
				}
		        catch( const Exception& )
		        {
                    implHandleAnyException( ::cppu::getCaughtException() );
		        }
			}
		}
		else
		{
			// Vollqualifizierten Namen erweitern
			String aNewName = GetName();
			aNewName.AppendAscii( "." );
			aNewName += rName;

			// CoreReflection holen
			Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
			if( xCoreReflection.is() )
			{
				// Ist es eine Konstante?
				Reference< XHierarchicalNameAccess > xHarryName( xCoreReflection, UNO_QUERY );
				if( xHarryName.is() )
				{
					try
					{
						Any aValue = xHarryName->getByHierarchicalName( aNewName );
						TypeClass eType = aValue.getValueType().getTypeClass();

						// Interface gefunden? Dann ist es eine Klasse
						if( eType == TypeClass_INTERFACE )
						{
							Reference< XInterface > xIface = *(Reference< XInterface >*)aValue.getValue();
							Reference< XIdlClass > xClass( xIface, UNO_QUERY );
							if( xClass.is() )
							{
								pRes = new SbxVariable( SbxVARIANT );
								SbxObjectRef xWrapper = (SbxObject*)new SbUnoClass( aNewName, xClass );
								pRes->PutObject( xWrapper );
							}
						}
						else
						{
							pRes = new SbxVariable( SbxVARIANT );
							unoToSbxValue( pRes, aValue );
						}
					}
					catch( NoSuchElementException& e1 )
					{
						String aMsg = implGetExceptionMsg( e1 );
					}
				}

				// Sonst wieder als Klasse annehmen
				if( !pRes )
				{
                    SbUnoClass* pNewClass = findUnoClass( aNewName );
					if( pNewClass )
					{
						pRes = new SbxVariable( SbxVARIANT );
						SbxObjectRef xWrapper = (SbxObject*)pNewClass;
						pRes->PutObject( xWrapper );
					}
				}

				// An UNO service?
				if( !pRes )
				{
					SbUnoService* pUnoService = findUnoService( aNewName );
					if( pUnoService )
					{
						pRes = new SbxVariable( SbxVARIANT );
						SbxObjectRef xWrapper = (SbxObject*)pUnoService;
						pRes->PutObject( xWrapper );
					}
				}

				// An UNO singleton?
				if( !pRes )
				{
					SbUnoSingleton* pUnoSingleton = findUnoSingleton( aNewName );
					if( pUnoSingleton )
					{
						pRes = new SbxVariable( SbxVARIANT );
						SbxObjectRef xWrapper = (SbxObject*)pUnoSingleton;
						pRes->PutObject( xWrapper );
					}
				}
			}
		}

		if( pRes )
		{
			pRes->SetName( rName );

			// Variable einfuegen, damit sie spaeter im Find gefunden wird
			QuickInsert( pRes );

			// Uns selbst gleich wieder als Listener rausnehmen,
			// die Werte sind alle konstant
			if( pRes->IsBroadcaster() )
				EndListening( pRes->GetBroadcaster(), sal_True );
		}
	}
	return pRes;
}


SbUnoService* findUnoService( const String& rName )
{
	SbUnoService* pSbUnoService = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
	    Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_SERVICE )
			{
				Reference< XServiceTypeDescription2 > xServiceTypeDesc( xTypeDesc, UNO_QUERY );
				if( xServiceTypeDesc.is() )
        			pSbUnoService = new SbUnoService( rName, xServiceTypeDesc );
			}
        }
    }
	return pSbUnoService;
}

SbxVariable* SbUnoService::Find( const String& rName, SbxClassType )
{
	SbxVariable* pRes = SbxObject::Find( rName, SbxCLASS_METHOD );

	if( !pRes )
	{
		// Wenn es schon eine Klasse ist, nach einen Feld fragen
		if( m_bNeedsInit && m_xServiceTypeDesc.is() )
		{
			m_bNeedsInit = false;

			Sequence< Reference< XServiceConstructorDescription > > aSCDSeq = m_xServiceTypeDesc->getConstructors();
			const Reference< XServiceConstructorDescription >* pCtorSeq = aSCDSeq.getConstArray();
			int nCtorCount = aSCDSeq.getLength();
			for( int i = 0 ; i < nCtorCount ; ++i )
			{
				Reference< XServiceConstructorDescription > xCtor = pCtorSeq[i];
				
				String aName( xCtor->getName() );
				if( !aName.Len() )
				{
					if( xCtor->isDefaultConstructor() )
						aName = String::CreateFromAscii( "create" );
				}

				if( aName.Len() )
				{
					// Create and insert SbUnoServiceCtor
					SbxVariableRef xSbCtorRef = new SbUnoServiceCtor( aName, xCtor );
					QuickInsert( (SbxVariable*)xSbCtorRef );
				}
			}

			pRes = SbxObject::Find( rName, SbxCLASS_METHOD );
		}
	}

	return pRes;
}

void SbUnoService::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
						   const SfxHint& rHint, const TypeId& rHintType )
{
	const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
	if( pHint )
	{
		SbxVariable* pVar = pHint->GetVar();
		SbxArray* pParams = pVar->GetParameters();
		SbUnoServiceCtor* pUnoCtor = PTR_CAST(SbUnoServiceCtor,pVar);
		if( pUnoCtor && pHint->GetId() == SBX_HINT_DATAWANTED )
		{
			// Parameter count -1 because of Param0 == this
			sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
			Sequence<Any> args;
			sal_Bool bOutParams = sal_False;

			Reference< XServiceConstructorDescription > xCtor = pUnoCtor->getServiceCtorDesc();
			Sequence< Reference< XParameter > > aParameterSeq = xCtor->getParameters();
			const Reference< XParameter >* pParameterSeq = aParameterSeq.getConstArray();
			sal_uInt32 nUnoParamCount = aParameterSeq.getLength();

			// Default: Ignore not needed parameters
			bool bParameterError = false;

			// Is the last parameter a rest parameter?
			bool bRestParameterMode = false;
			if( nUnoParamCount > 0 )
			{
				Reference< XParameter > xLastParam = pParameterSeq[ nUnoParamCount - 1 ];
				if( xLastParam.is() )
				{
					if( xLastParam->isRestParameter() )
						bRestParameterMode = true;
				}
			}

			// Too many parameters with context as first parameter?
			sal_uInt16 nSbxParameterOffset = 1;
			sal_uInt16 nParameterOffsetByContext = 0;
			Reference < XComponentContext > xFirstParamContext;
			if( nParamCount > nUnoParamCount )
			{
				// Check if first parameter is a context and use it
				// then in createInstanceWithArgumentsAndContext
				Any aArg0 = sbxToUnoValue( pParams->Get( nSbxParameterOffset ) );
				if( (aArg0 >>= xFirstParamContext) && xFirstParamContext.is() )
					nParameterOffsetByContext = 1;
			}

			sal_uInt32 nEffectiveParamCount = nParamCount - nParameterOffsetByContext;
			sal_uInt32 nAllocParamCount = nEffectiveParamCount;
			if( nEffectiveParamCount > nUnoParamCount )
			{
				if( !bRestParameterMode )
				{
					nEffectiveParamCount = nUnoParamCount;
					nAllocParamCount = nUnoParamCount;
				}
			}
			// Not enough parameters?
			else if( nUnoParamCount > nEffectiveParamCount )
			{
				// RestParameterMode only helps if one (the last) parameter is missing
				int nDiff = nUnoParamCount - nEffectiveParamCount;
				if( !bRestParameterMode || nDiff > 1 )
				{
					bParameterError = true;
					StarBASIC::Error( SbERR_NOT_OPTIONAL );
				}
			}

			if( !bParameterError )
			{
				if( nAllocParamCount > 0 )
				{
					args.realloc( nAllocParamCount );
					Any* pAnyArgs = args.getArray();
					for( sal_uInt32 i = 0 ; i < nEffectiveParamCount ; i++ )
					{
						sal_uInt16 iSbx = (sal_uInt16)(i + nSbxParameterOffset + nParameterOffsetByContext);

						// bRestParameterMode allows nEffectiveParamCount > nUnoParamCount
						Reference< XParameter > xParam;
						if( i < nUnoParamCount )
						{
							xParam = pParameterSeq[i];
							if( !xParam.is() )
								continue;

							Reference< XTypeDescription > xParamTypeDesc = xParam->getType();
							if( !xParamTypeDesc.is() )
								continue;
							com::sun::star::uno::Type aType( xParamTypeDesc->getTypeClass(), xParamTypeDesc->getName() );

							// sbx parameter needs offset 1
							pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ), aType );

							// Check for out parameter if not already done
							if( !bOutParams )
							{
								if( xParam->isOut() )
									bOutParams = sal_True;
							}
						}
						else
						{
							pAnyArgs[i] = sbxToUnoValue( pParams->Get( iSbx ) );
						}
					}
				}

				// "Call" ctor using createInstanceWithArgumentsAndContext
				Reference < XComponentContext > xContext;
				if( xFirstParamContext.is() )
				{
					xContext = xFirstParamContext;
				}
				else
				{
					Reference < XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
					xContext.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" )) ), UNO_QUERY_THROW );
				}
				Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() ); 

				Any aRetAny;
				if( xServiceMgr.is() )
				{
					String aServiceName = GetName();
					Reference < XInterface > xRet;
					try
					{
						xRet = xServiceMgr->createInstanceWithArgumentsAndContext( aServiceName, args, xContext );
					}
					catch( const Exception& )
					{
						implHandleAnyException( ::cppu::getCaughtException() );
					}
					aRetAny <<= xRet;
				}
				unoToSbxValue( pVar, aRetAny );

				// Copy back out parameters?
				if( bOutParams )
				{
					const Any* pAnyArgs = args.getConstArray();

					for( sal_uInt32 j = 0 ; j < nUnoParamCount ; j++ )
					{
						Reference< XParameter > xParam = pParameterSeq[j];
						if( !xParam.is() )
							continue;

						if( xParam->isOut() )
							unoToSbxValue( (SbxVariable*)pParams->Get( (sal_uInt16)(j+1) ), pAnyArgs[ j ] );
					}
				}
			}
		}
		else
			SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
	}
}



static SbUnoServiceCtor* pFirstCtor = NULL;

void clearUnoServiceCtors( void )
{
	SbUnoServiceCtor* pCtor = pFirstCtor;
	while( pCtor )
	{
		pCtor->SbxValue::Clear();
		pCtor = pCtor->pNext;
	}
}

SbUnoServiceCtor::SbUnoServiceCtor( const String& aName_, Reference< XServiceConstructorDescription > xServiceCtorDesc )
	: SbxMethod( aName_, SbxOBJECT )
	, m_xServiceCtorDesc( xServiceCtorDesc )
{
}

SbUnoServiceCtor::~SbUnoServiceCtor()
{
}

SbxInfo* SbUnoServiceCtor::GetInfo()
{
	SbxInfo* pRet = NULL;

	return pRet;
}


SbUnoSingleton* findUnoSingleton( const String& rName )
{
	SbUnoSingleton* pSbUnoSingleton = NULL;

    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    if( xTypeAccess->hasByHierarchicalName( rName ) )
    {
        Any aRet = xTypeAccess->getByHierarchicalName( rName );
	    Reference< XTypeDescription > xTypeDesc;
        aRet >>= xTypeDesc;

        if( xTypeDesc.is() )
        {
            TypeClass eTypeClass = xTypeDesc->getTypeClass();
            if( eTypeClass == TypeClass_SINGLETON )
			{
				Reference< XSingletonTypeDescription > xSingletonTypeDesc( xTypeDesc, UNO_QUERY );
				if( xSingletonTypeDesc.is() )
        			pSbUnoSingleton = new SbUnoSingleton( rName, xSingletonTypeDesc );
			}
        }
    }
	return pSbUnoSingleton;
}

SbUnoSingleton::SbUnoSingleton( const String& aName_,
	const Reference< XSingletonTypeDescription >& xSingletonTypeDesc )
		: SbxObject( aName_ )
		, m_xSingletonTypeDesc( xSingletonTypeDesc )
{
	SbxVariableRef xGetMethodRef =
		new SbxMethod( String( RTL_CONSTASCII_USTRINGPARAM( "get" ) ), SbxOBJECT );
	QuickInsert( (SbxVariable*)xGetMethodRef );
}

void SbUnoSingleton::SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
						   const SfxHint& rHint, const TypeId& rHintType )
{
	const SbxHint* pHint = PTR_CAST(SbxHint,&rHint);
	if( pHint )
	{
		SbxVariable* pVar = pHint->GetVar();
		SbxArray* pParams = pVar->GetParameters();
		sal_uInt32 nParamCount = pParams ? ((sal_uInt32)pParams->Count() - 1) : 0;
		sal_uInt32 nAllowedParamCount = 1;

		Reference < XComponentContext > xContextToUse;
		if( nParamCount > 0 )
		{
			// Check if first parameter is a context and use it then
			Reference < XComponentContext > xFirstParamContext;
			Any aArg1 = sbxToUnoValue( pParams->Get( 1 ) );
			if( (aArg1 >>= xFirstParamContext) && xFirstParamContext.is() )
				xContextToUse = xFirstParamContext;
		}

		if( !xContextToUse.is() )
		{
			Reference < XPropertySet > xProps( ::comphelper::getProcessServiceFactory(), UNO_QUERY_THROW );
			xContextToUse.set( xProps->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "DefaultContext" )) ), UNO_QUERY_THROW );
			--nAllowedParamCount;
		}

		if( nParamCount > nAllowedParamCount )
		{
			StarBASIC::Error( SbERR_BAD_ARGUMENT );
			return;
		}

		Any aRetAny;
		if( xContextToUse.is() )
		{
			String aSingletonName( RTL_CONSTASCII_USTRINGPARAM("/singletons/") );
			aSingletonName += GetName();
			Reference < XInterface > xRet;
			xContextToUse->getValueByName( aSingletonName ) >>= xRet;
			aRetAny <<= xRet;
		}
		unoToSbxValue( pVar, aRetAny );
	}
	else
		SbxObject::SFX_NOTIFY( rBC, rBCType, rHint, rHintType );
}

 
//========================================================================
//========================================================================
//========================================================================

// Implementation eines EventAttacher-bezogenen AllListeners, der
// nur einzelne Events an einen allgemeinen AllListener weiterleitet
class BasicAllListener_Impl : public BasicAllListenerHelper
{
	virtual void firing_impl(const AllEventObject& Event, Any* pRet);

public:
	SbxObjectRef	xSbxObj;
	::rtl::OUString		aPrefixName;

	BasicAllListener_Impl( const ::rtl::OUString& aPrefixName );
	~BasicAllListener_Impl();

	// Methoden von XInterface
	//virtual sal_Bool queryInterface( Uik aUik, Reference< XInterface > & rOut );

	// Methoden von XAllListener
	virtual void SAL_CALL firing(const AllEventObject& Event) throw ( RuntimeException );
	virtual Any SAL_CALL approveFiring(const AllEventObject& Event) throw ( RuntimeException );

	// Methoden von XEventListener
	virtual void SAL_CALL disposing(const EventObject& Source) throw ( RuntimeException );
};


//========================================================================
BasicAllListener_Impl::BasicAllListener_Impl
(
	const ::rtl::OUString	& aPrefixName_
)
	: aPrefixName( aPrefixName_ )
{
}

//========================================================================
BasicAllListener_Impl::~BasicAllListener_Impl()
{
}

//========================================================================

void BasicAllListener_Impl::firing_impl( const AllEventObject& Event, Any* pRet )
{
	vos::OGuard guard( Application::GetSolarMutex() );

	if( xSbxObj.Is() )
	{
		::rtl::OUString aMethodName = aPrefixName;
		aMethodName = aMethodName + Event.MethodName;

		SbxVariable * pP = xSbxObj;
		while( pP->GetParent() )
		{
			pP = pP->GetParent();
			StarBASIC * pLib = PTR_CAST(StarBASIC,pP);
			if( pLib )
			{
				// In Basic Array anlegen
				SbxArrayRef xSbxArray = new SbxArray( SbxVARIANT );
				const Any * pArgs = Event.Arguments.getConstArray();
				sal_Int32 nCount = Event.Arguments.getLength();
				for( sal_Int32 i = 0; i < nCount; i++ )
				{
					// Elemente wandeln
					SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
					unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
					xSbxArray->Put( xVar, sal::static_int_cast< sal_uInt16 >(i+1) );
				}

				pLib->Call( aMethodName, xSbxArray );

				// Return-Wert aus dem Param-Array holen, wenn verlangt
				if( pRet )
				{
					SbxVariable* pVar = xSbxArray->Get( 0 );
					if( pVar )
					{
						// #95792 Avoid a second call
						sal_uInt16 nFlags = pVar->GetFlags();
						pVar->SetFlag( SBX_NO_BROADCAST );
						*pRet = sbxToUnoValueImpl( pVar );
						pVar->SetFlags( nFlags );
					}
				}
				break;
			}
		}
	}
}


// Methoden von XAllListener
void BasicAllListener_Impl::firing( const AllEventObject& Event ) throw ( RuntimeException )
{
	firing_impl( Event, NULL );
}

Any BasicAllListener_Impl::approveFiring( const AllEventObject& Event ) throw ( RuntimeException )
{
	Any aRetAny;
	firing_impl( Event, &aRetAny );
	return aRetAny;
}

//========================================================================
// Methoden von XEventListener
void BasicAllListener_Impl ::disposing(const EventObject& ) throw ( RuntimeException )
{
	vos::OGuard guard( Application::GetSolarMutex() );

	xSbxObj.Clear();
}



//*************************************************************************
//  class InvocationToAllListenerMapper
//  helper class to map XInvocation to XAllListener (also in project eventattacher!)
//*************************************************************************
class InvocationToAllListenerMapper : public WeakImplHelper1< XInvocation >
{
public:
	InvocationToAllListenerMapper( const Reference< XIdlClass >& ListenerType,
		const Reference< XAllListener >& AllListener, const Any& Helper );

	// XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection(void) throw( RuntimeException );
    virtual Any SAL_CALL invoke(const ::rtl::OUString& FunctionName, const Sequence< Any >& Params, Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
		throw( IllegalArgumentException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual void SAL_CALL setValue(const ::rtl::OUString& PropertyName, const Any& Value)
		throw( UnknownPropertyException, CannotConvertException, InvocationTargetException, RuntimeException );
    virtual Any SAL_CALL getValue(const ::rtl::OUString& PropertyName) throw( UnknownPropertyException, RuntimeException );
    virtual sal_Bool SAL_CALL hasMethod(const ::rtl::OUString& Name) throw( RuntimeException );
    virtual sal_Bool SAL_CALL hasProperty(const ::rtl::OUString& Name) throw( RuntimeException );

private:
	Reference< XIdlReflection >  m_xCoreReflection;
	Reference< XAllListener >	 m_xAllListener;
	Reference< XIdlClass >  	 m_xListenerType;
	Any 						 m_Helper;
};


// Function to replace AllListenerAdapterService::createAllListerAdapter
Reference< XInterface > createAllListenerAdapter
(
	const Reference< XInvocationAdapterFactory >& xInvocationAdapterFactory,
	const Reference< XIdlClass >& xListenerType,
	const Reference< XAllListener >& xListener,
	const Any& Helper
)
{
	Reference< XInterface > xAdapter;
	if( xInvocationAdapterFactory.is() && xListenerType.is() && xListener.is() )
	{
	   Reference< XInvocation >	xInvocationToAllListenerMapper =
			(XInvocation*)new InvocationToAllListenerMapper( xListenerType, xListener, Helper );
		Type aListenerType( xListenerType->getTypeClass(), xListenerType->getName() );
		xAdapter = xInvocationAdapterFactory->createAdapter( xInvocationToAllListenerMapper, aListenerType );
	}
	return xAdapter;
}


//--------------------------------------------------------------------------------------------------
// InvocationToAllListenerMapper
InvocationToAllListenerMapper::InvocationToAllListenerMapper
	( const Reference< XIdlClass >& ListenerType, const Reference< XAllListener >& AllListener, const Any& Helper )
		: m_xAllListener( AllListener )
		, m_xListenerType( ListenerType )
		, m_Helper( Helper )
{
}

//*************************************************************************
Reference< XIntrospectionAccess > SAL_CALL InvocationToAllListenerMapper::getIntrospection(void)
	throw( RuntimeException )
{
	return Reference< XIntrospectionAccess >();
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::invoke(const ::rtl::OUString& FunctionName, const Sequence< Any >& Params,
	Sequence< sal_Int16 >& OutParamIndex, Sequence< Any >& OutParam)
		throw( IllegalArgumentException, CannotConvertException,
		InvocationTargetException, RuntimeException )
{
    (void)OutParamIndex;
    (void)OutParam     ;

	Any aRet;

	// Check if to firing or approveFiring has to be called
	Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( FunctionName );
	sal_Bool bApproveFiring = sal_False;
	if( !xMethod.is() )
		return aRet;
    Reference< XIdlClass > xReturnType = xMethod->getReturnType();
    Sequence< Reference< XIdlClass > > aExceptionSeq = xMethod->getExceptionTypes();
	if( ( xReturnType.is() && xReturnType->getTypeClass() != TypeClass_VOID ) ||
		aExceptionSeq.getLength() > 0 )
	{
		bApproveFiring = sal_True;
	}
	else
	{
	    Sequence< ParamInfo > aParamSeq = xMethod->getParameterInfos();
		sal_uInt32 nParamCount = aParamSeq.getLength();
		if( nParamCount > 1 )
		{
			const ParamInfo* pInfos = aParamSeq.getConstArray();
			for( sal_uInt32 i = 0 ; i < nParamCount ; i++ )
			{
				if( pInfos[ i ].aMode != ParamMode_IN )
				{
					bApproveFiring = sal_True;
					break;
				}
			}
		}
	}

    AllEventObject aAllEvent;
    aAllEvent.Source = (OWeakObject*) this;
    aAllEvent.Helper = m_Helper;
    aAllEvent.ListenerType = Type(m_xListenerType->getTypeClass(), m_xListenerType->getName() );
    aAllEvent.MethodName = FunctionName;
    aAllEvent.Arguments = Params;
	if( bApproveFiring )
		aRet = m_xAllListener->approveFiring( aAllEvent );
	else
		m_xAllListener->firing( aAllEvent );
	return aRet;
}

//*************************************************************************
void SAL_CALL InvocationToAllListenerMapper::setValue(const ::rtl::OUString& PropertyName, const Any& Value)
	throw( UnknownPropertyException, CannotConvertException,
		   InvocationTargetException, RuntimeException )
{
    (void)PropertyName;
    (void)Value;
}

//*************************************************************************
Any SAL_CALL InvocationToAllListenerMapper::getValue(const ::rtl::OUString& PropertyName)
	throw( UnknownPropertyException, RuntimeException )
{
    (void)PropertyName;

	return Any();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasMethod(const ::rtl::OUString& Name)
	throw( RuntimeException )
{
	Reference< XIdlMethod > xMethod = m_xListenerType->getMethod( Name );
	return xMethod.is();
}

//*************************************************************************
sal_Bool SAL_CALL InvocationToAllListenerMapper::hasProperty(const ::rtl::OUString& Name)
	throw( RuntimeException )
{
	Reference< XIdlField > xField = m_xListenerType->getField( Name );
	return xField.is();
}

//========================================================================
// Uno-Service erzeugen
// 1. Parameter == Prefix-Name der Makros
// 2. Parameter == voll qualifizierter Name des Listeners
void SbRtl_CreateUnoListener( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
//RTLFUNC(CreateUnoListener)
{
    (void)bWrite;

	// Wir brauchen 2 Parameter
	if ( rPar.Count() != 3 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Klassen-Name der struct holen
	String aPrefixName = rPar.Get(1)->GetString();
	String aListenerClassName = rPar.Get(2)->GetString();

	// CoreReflection holen
	Reference< XIdlReflection > xCoreReflection = getCoreReflection_Impl();
	if( !xCoreReflection.is() )
		return;

	// AllListenerAdapterService holen
	Reference< XMultiServiceFactory > xFactory( comphelper::getProcessServiceFactory() );
	if( !xFactory.is() )
		return;

	// Klasse suchen
	Reference< XIdlClass > xClass = xCoreReflection->forName( aListenerClassName );
	if( !xClass.is() )
		return;

	// AB, 30.11.1999 InvocationAdapterFactory holen
	Reference< XInvocationAdapterFactory > xInvocationAdapterFactory = Reference< XInvocationAdapterFactory >(
		xFactory->createInstance( rtl::OUString::createFromAscii("com.sun.star.script.InvocationAdapterFactory") ), UNO_QUERY );

	BasicAllListener_Impl * p;
	Reference< XAllListener > xAllLst = p = new BasicAllListener_Impl( aPrefixName );
	Any aTmp;
	Reference< XInterface > xLst = createAllListenerAdapter( xInvocationAdapterFactory, xClass, xAllLst, aTmp );
	if( !xLst.is() )
		return;

	::rtl::OUString aClassName = xClass->getName();
	Type aClassType( xClass->getTypeClass(), aClassName.getStr() );
	aTmp = xLst->queryInterface( aClassType );
	if( !aTmp.hasValue() )
		return;

	SbUnoObject* pUnoObj = new SbUnoObject( aListenerClassName, aTmp );
    p->xSbxObj = pUnoObj;
	p->xSbxObj->SetParent( pBasic );

    // #100326 Register listener object to set Parent NULL in Dtor
    SbxArrayRef xBasicUnoListeners = pBasic->getUnoListeners();
	xBasicUnoListeners->Insert( pUnoObj, xBasicUnoListeners->Count() );

	// Objekt zurueckliefern
	SbxVariableRef refVar = rPar.Get(0);
	refVar->PutObject( p->xSbxObj );
}

//========================================================================
// Represents the DefaultContext property of the ProcessServiceManager
// in the Basic runtime system.
void RTL_Impl_GetDefaultContext( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	SbxVariableRef refVar = rPar.Get(0);

	Reference< XMultiServiceFactory > xFactory = comphelper::getProcessServiceFactory();
	Reference< XPropertySet> xPSMPropertySet( xFactory, UNO_QUERY );
	if( xPSMPropertySet.is() )
	{
		Any aContextAny = xPSMPropertySet->getPropertyValue(
			String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ) );

		SbUnoObjectRef xUnoObj = new SbUnoObject
			( String( RTL_CONSTASCII_USTRINGPARAM("DefaultContext") ),
			  aContextAny );
		refVar->PutObject( (SbUnoObject*)xUnoObj );
	}
	else
	{
		refVar->PutObject( NULL );
	}
}

//========================================================================
// Creates a Basic wrapper object for a strongly typed Uno value
// 1. parameter: Uno type as full qualified type name, e.g. "byte[]"
void RTL_Impl_CreateUnoValue( StarBASIC* pBasic, SbxArray& rPar, sal_Bool bWrite )
{
    (void)pBasic;
    (void)bWrite;

	static String aTypeTypeString( RTL_CONSTASCII_USTRINGPARAM("type") );

	// 2 parameters needed
	if ( rPar.Count() != 3 )
	{
		StarBASIC::Error( SbERR_BAD_ARGUMENT );
		return;
	}

	// Klassen-Name der struct holen
	String aTypeName = rPar.Get(1)->GetString();
    SbxVariable* pVal = rPar.Get(2);

	if( aTypeName == aTypeTypeString )
	{
		SbxDataType eBaseType = pVal->SbxValue::GetType();
		String aValTypeName;
		if( eBaseType == SbxSTRING )
		{
			aValTypeName = pVal->GetString();
		}
		else if( eBaseType == SbxOBJECT )
		{
			// XIdlClass?
			Reference< XIdlClass > xIdlClass;

			SbxBaseRef pObj = (SbxBase*)pVal->GetObject();
			if( pObj && pObj->ISA(SbUnoObject) )
			{
				Any aUnoAny = ((SbUnoObject*)(SbxBase*)pObj)->getUnoAny();
				aUnoAny >>= xIdlClass;
			}

			if( xIdlClass.is() )
				aValTypeName = xIdlClass->getName();
		}
		Type aType;
		bool bSuccess = implGetTypeByName( aValTypeName, aType );
		if( bSuccess )
		{
			Any aTypeAny( aType );
			SbxVariableRef refVar = rPar.Get(0);
			SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aTypeAny );
			refVar->PutObject( xUnoAnyObject );
		}
		return;
	}

    // Check the type
    Reference< XHierarchicalNameAccess > xTypeAccess = getTypeProvider_Impl();
    Any aRet;
	try
	{
        aRet = xTypeAccess->getByHierarchicalName( aTypeName );
	}
	catch( NoSuchElementException& e1 )
	{
        String aNoSuchElementExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.container.NoSuchElementException" ) );
		StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e1, aNoSuchElementExceptionName ) );
		return;
	}
    Reference< XTypeDescription > xTypeDesc;
    aRet >>= xTypeDesc;
    TypeClass eTypeClass = xTypeDesc->getTypeClass();
	Type aDestType( eTypeClass, aTypeName );


    // Preconvert value
	Any aVal = sbxToUnoValueImpl( pVal );
    Any aConvertedVal = convertAny( aVal, aDestType );

    /*
    // Convert
    Reference< XTypeConverter > xConverter = getTypeConverter_Impl();
	try
	{
        aConvertedVal = xConverter->convertTo( aVal, aDestType );
	}
	catch( IllegalArgumentException& e1 )
	{
		StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( ::cppu::getCaughtException() ) );
		return;
	}
	catch( CannotConvertException& e2 )
	{
        String aCannotConvertExceptionName
            ( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.lang.IllegalArgumentException" ) );
		StarBASIC::Error( ERRCODE_BASIC_EXCEPTION,
            implGetExceptionMsg( e2, aCannotConvertExceptionName ) );
		return;
	}
    */

	SbxVariableRef refVar = rPar.Get(0);
	SbxObjectRef xUnoAnyObject = new SbUnoAnyObject( aConvertedVal );
	refVar->PutObject( xUnoAnyObject );
}

//==========================================================================

namespace {
class OMutexBasis
{
protected:
    // this mutex is necessary for OInterfaceContainerHelper
    ::osl::Mutex m_aMutex;
};
} // namespace

typedef WeakImplHelper2< XInvocation, XComponent > ModuleInvocationProxyHelper;

class ModuleInvocationProxy : public OMutexBasis,
                              public ModuleInvocationProxyHelper
{
	::rtl::OUString		m_aPrefix;
	SbxObjectRef		m_xScopeObj;
	bool				m_bProxyIsClassModuleObject;

    ::cppu::OInterfaceContainerHelper m_aListeners;

public:
	ModuleInvocationProxy( const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj );
	~ModuleInvocationProxy()
	{}

	// XInvocation
    virtual Reference< XIntrospectionAccess > SAL_CALL getIntrospection() throw();
    virtual void SAL_CALL setValue( const ::rtl::OUString& rProperty, const Any& rValue )
        throw( UnknownPropertyException );
    virtual Any SAL_CALL getValue( const ::rtl::OUString& rProperty )
        throw( UnknownPropertyException );
    virtual sal_Bool SAL_CALL hasMethod( const ::rtl::OUString& rName ) throw();
    virtual sal_Bool SAL_CALL hasProperty( const ::rtl::OUString& rProp ) throw();
    
    virtual Any SAL_CALL invoke( const ::rtl::OUString& rFunction,
                                 const Sequence< Any >& rParams,
                                 Sequence< sal_Int16 >& rOutParamIndex,
                                 Sequence< Any >& rOutParam )
        throw( CannotConvertException, InvocationTargetException );

    // XComponent
    virtual void SAL_CALL dispose() throw(RuntimeException);
    virtual void SAL_CALL addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException);
    virtual void SAL_CALL removeEventListener( const Reference< XEventListener >& aListener ) throw (RuntimeException);
};

ModuleInvocationProxy::ModuleInvocationProxy( const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj )
	: m_aPrefix( aPrefix + ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_") ) )
	, m_xScopeObj( xScopeObj )
    , m_aListeners( m_aMutex )
{
	m_bProxyIsClassModuleObject = xScopeObj.Is() ? xScopeObj->ISA(SbClassModuleObject) : false;
}

Reference< XIntrospectionAccess > SAL_CALL ModuleInvocationProxy::getIntrospection() throw()
{
    return Reference< XIntrospectionAccess >();
}

void SAL_CALL ModuleInvocationProxy::setValue( const ::rtl::OUString& rProperty, const Any& rValue ) throw( UnknownPropertyException )
{
	if( !m_bProxyIsClassModuleObject )
		throw UnknownPropertyException();

	vos::OGuard guard( Application::GetSolarMutex() );

	::rtl::OUString aPropertyFunctionName( RTL_CONSTASCII_USTRINGPARAM( "Property Set ") );
	aPropertyFunctionName += m_aPrefix;
	aPropertyFunctionName += rProperty;

	SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxCLASS_METHOD );
	SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
	if( pMeth == NULL )
	{
		// TODO: Check vba behavior concernig missing function
		//StarBASIC::Error( SbERR_NO_METHOD, aFunctionName );
		throw UnknownPropertyException();
	}

	// Setup parameter
	SbxArrayRef xArray = new SbxArray;
	SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
	unoToSbxValue( (SbxVariable*)xVar, rValue );
	xArray->Put( xVar, 1 );

	// Call property method
	SbxVariableRef xValue = new SbxVariable;
	pMeth->SetParameters( xArray );
	pMeth->Call( xValue );
	//aRet = sbxToUnoValue( xValue );
	pMeth->SetParameters( NULL );

	// TODO: OutParameter?

	// throw InvocationTargetException();

    //return aRet;

}

Any SAL_CALL ModuleInvocationProxy::getValue( const ::rtl::OUString& rProperty ) throw( UnknownPropertyException )
{
	if( !m_bProxyIsClassModuleObject )
	    throw UnknownPropertyException();

	vos::OGuard guard( Application::GetSolarMutex() );

	::rtl::OUString aPropertyFunctionName( RTL_CONSTASCII_USTRINGPARAM( "Property Get ") );
	aPropertyFunctionName += m_aPrefix;
	aPropertyFunctionName += rProperty;

	SbxVariable* p = m_xScopeObj->Find( aPropertyFunctionName, SbxCLASS_METHOD );
	SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
	if( pMeth == NULL )
	{
		// TODO: Check vba behavior concernig missing function
		//StarBASIC::Error( SbERR_NO_METHOD, aFunctionName );
	    throw UnknownPropertyException();
	}

	// Call method
	SbxVariableRef xValue = new SbxVariable;
	pMeth->Call( xValue );
	Any aRet = sbxToUnoValue( xValue );
    return aRet;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasMethod( const ::rtl::OUString& ) throw()
{
    return sal_False;
}

sal_Bool SAL_CALL ModuleInvocationProxy::hasProperty( const ::rtl::OUString& ) throw()
{
    return sal_False;
}

Any SAL_CALL ModuleInvocationProxy::invoke( const ::rtl::OUString& rFunction,
											const Sequence< Any >& rParams,
											Sequence< sal_Int16 >&,
											Sequence< Any >& )
    throw( CannotConvertException, InvocationTargetException )
{
	vos::OGuard guard( Application::GetSolarMutex() );

    Any aRet;
	SbxObjectRef xScopeObj = m_xScopeObj;
	if( !xScopeObj.Is() )
		return aRet;

	::rtl::OUString aFunctionName = m_aPrefix;
	aFunctionName += rFunction;

    sal_Bool bSetRescheduleBack = sal_False;
    sal_Bool bOldReschedule = sal_True;
    SbiInstance* pInst = pINST;
    if( pInst && pInst->IsCompatibility() )
    {
        bOldReschedule = pInst->IsReschedule();
        if ( bOldReschedule )
        {
            pInst->EnableReschedule( sal_False );
            bSetRescheduleBack = sal_True;
        }
    }

	SbxVariable* p = xScopeObj->Find( aFunctionName, SbxCLASS_METHOD );
	SbMethod* pMeth = p != NULL ? PTR_CAST(SbMethod,p) : NULL;
	if( pMeth == NULL )
	{
		// TODO: Check vba behavior concernig missing function
		//StarBASIC::Error( SbERR_NO_METHOD, aFunctionName );
		return aRet;
	}

	// Setup parameters
	SbxArrayRef xArray;
	sal_Int32 nParamCount = rParams.getLength();
	if( nParamCount )
	{
		xArray = new SbxArray;
		const Any *pArgs = rParams.getConstArray();
		for( sal_Int32 i = 0 ; i < nParamCount ; i++ )
		{
			SbxVariableRef xVar = new SbxVariable( SbxVARIANT );
			unoToSbxValue( (SbxVariable*)xVar, pArgs[i] );
			xArray->Put( xVar, sal::static_int_cast< sal_uInt16 >(i+1) );
		}
	}

	// Call method
	SbxVariableRef xValue = new SbxVariable;
	if( xArray.Is() )
		pMeth->SetParameters( xArray );
	pMeth->Call( xValue );
	aRet = sbxToUnoValue( xValue );
	pMeth->SetParameters( NULL );

    if( bSetRescheduleBack )
        pInst->EnableReschedule( bOldReschedule );

	// TODO: OutParameter?

    return aRet;
}

void SAL_CALL ModuleInvocationProxy::dispose()
    throw(RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );

    EventObject aEvent( (XComponent*)this );
    m_aListeners.disposeAndClear( aEvent );

    m_xScopeObj = NULL;
}

void SAL_CALL ModuleInvocationProxy::addEventListener( const Reference< XEventListener >& xListener )
    throw (RuntimeException)
{
    m_aListeners.addInterface( xListener );
}

void SAL_CALL ModuleInvocationProxy::removeEventListener( const Reference< XEventListener >& xListener )
    throw (RuntimeException)
{
    m_aListeners.removeInterface( xListener );
}


Reference< XInterface > createComListener( const Any& aControlAny, const ::rtl::OUString& aVBAType,
										   const ::rtl::OUString& aPrefix, SbxObjectRef xScopeObj )
{
	Reference< XInterface > xRet;

	Reference< XComponentContext > xContext = getComponentContext_Impl();
	Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() ); 

	Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPrefix, xScopeObj );

	Sequence<Any> args( 3 );
	args[0] <<= aControlAny;
	args[1] <<= aVBAType;
	args[2] <<= xProxy;

	try
	{
		xRet = xServiceMgr->createInstanceWithArgumentsAndContext(
			::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.custom.UnoComListener")),
			args, xContext );
	}
	catch( const Exception& )
	{
		implHandleAnyException( ::cppu::getCaughtException() );
	}

	return xRet;
}

typedef std::vector< WeakReference< XComponent > >	ComponentRefVector;

struct StarBasicDisposeItem
{
	StarBASIC*				m_pBasic;
	SbxArrayRef				m_pRegisteredVariables;
	ComponentRefVector		m_vComImplementsObjects;

	StarBasicDisposeItem( StarBASIC* pBasic )
		: m_pBasic( pBasic )
	{
		m_pRegisteredVariables = new SbxArray();
	}
};

typedef std::vector< StarBasicDisposeItem* > DisposeItemVector;

static DisposeItemVector GaDisposeItemVector;

DisposeItemVector::iterator lcl_findItemForBasic( StarBASIC* pBasic )
{
	DisposeItemVector::iterator it;
	for( it = GaDisposeItemVector.begin() ; it != GaDisposeItemVector.end() ; ++it )
	{
		StarBasicDisposeItem* pItem = *it;
		if( pItem->m_pBasic == pBasic )
			return it;
	}
	return GaDisposeItemVector.end();
}

StarBasicDisposeItem* lcl_getOrCreateItemForBasic( StarBASIC* pBasic )
{
	DisposeItemVector::iterator it = lcl_findItemForBasic( pBasic );
	StarBasicDisposeItem* pItem = (it != GaDisposeItemVector.end()) ? *it : NULL;
	if( pItem == NULL )
	{
		pItem = new StarBasicDisposeItem( pBasic );
		GaDisposeItemVector.push_back( pItem );
	}
	return pItem;
}

void registerComponentToBeDisposedForBasic
	( Reference< XComponent > xComponent, StarBASIC* pBasic )
{
	StarBasicDisposeItem* pItem = lcl_getOrCreateItemForBasic( pBasic );
	pItem->m_vComImplementsObjects.push_back( xComponent );
}

void registerComListenerVariableForBasic( SbxVariable* pVar, StarBASIC* pBasic )
{
	StarBasicDisposeItem* pItem = lcl_getOrCreateItemForBasic( pBasic );
	SbxArray* pArray = pItem->m_pRegisteredVariables;
	pArray->Put( pVar, pArray->Count() );
}

void disposeComVariablesForBasic( StarBASIC* pBasic )
{
	DisposeItemVector::iterator it = lcl_findItemForBasic( pBasic );
	if( it != GaDisposeItemVector.end() )
	{
		StarBasicDisposeItem* pItem = *it;

		SbxArray* pArray = pItem->m_pRegisteredVariables;
		sal_uInt16 nCount = pArray->Count();
		for( sal_uInt16 i = 0 ; i < nCount ; ++i )
		{
			SbxVariable* pVar = pArray->Get( i );
			pVar->ClearComListener();
		}

		ComponentRefVector& rv = pItem->m_vComImplementsObjects;
		ComponentRefVector::iterator itCRV;
		for( itCRV = rv.begin() ; itCRV != rv.end() ; ++itCRV )
		{
            try
            {
                Reference< XComponent > xComponent( (*itCRV).get(), UNO_QUERY_THROW );
                xComponent->dispose();
            }
            catch( Exception& )
            {}
		}

		delete pItem;
		GaDisposeItemVector.erase( it );
	}
}


// Handle module implements mechanism for OLE types
bool SbModule::createCOMWrapperForIface( Any& o_rRetAny, SbClassModuleObject* pProxyClassModuleObject )
{
	// For now: Take first interface that allows to instantiate COM wrapper
	// TODO: Check if support for multiple interfaces is needed

	Reference< XComponentContext > xContext = getComponentContext_Impl();
	Reference< XMultiComponentFactory > xServiceMgr( xContext->getServiceManager() ); 
	Reference< XSingleServiceFactory > xComImplementsFactory
	(
        xServiceMgr->createInstanceWithContext(
	        ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.custom.ComImplementsFactory")), xContext ),
		UNO_QUERY
	);
	if( !xComImplementsFactory.is() )
		return false;

	bool bSuccess = false;

	SbxArray* pModIfaces = pClassData->mxIfaces;
	sal_uInt16 nCount = pModIfaces->Count();
	for( sal_uInt16 i = 0 ; i < nCount ; ++i )
	{
		SbxVariable* pVar = pModIfaces->Get( i );
		::rtl::OUString aIfaceName = pVar->GetName();

		if( !aIfaceName.isEmpty() )
		{
			::rtl::OUString aPureIfaceName = aIfaceName;
			sal_Int32 indexLastDot = aIfaceName.lastIndexOf('.');
			if ( indexLastDot > -1 )
				aPureIfaceName = aIfaceName.copy( indexLastDot + 1 );

			Reference< XInvocation > xProxy = new ModuleInvocationProxy( aPureIfaceName, pProxyClassModuleObject );

			Sequence<Any> args( 2 );
			args[0] <<= aIfaceName;
			args[1] <<= xProxy;

			Reference< XInterface > xRet;
			bSuccess = false;
			try
			{
				xRet = xComImplementsFactory->createInstanceWithArguments( args );
				bSuccess = true;
			}
			catch( const Exception& )
			{
				implHandleAnyException( ::cppu::getCaughtException() );
			}

			if( bSuccess )
			{
				Reference< XComponent > xComponent( xProxy, UNO_QUERY );
				if( xComponent.is() )
				{
					StarBASIC* pParentBasic = NULL;
					SbxObject* pCurObject = this;
					do
					{
						SbxObject* pObjParent = pCurObject->GetParent();
						pParentBasic = PTR_CAST( StarBASIC, pObjParent );
						pCurObject = pObjParent;
					}
					while( pParentBasic == NULL && pCurObject != NULL );

					OSL_ASSERT( pParentBasic != NULL );
					registerComponentToBeDisposedForBasic( xComponent, pParentBasic );
				}

				o_rRetAny <<= xRet;
				break;
			}
		}
 	}

	return bSuccess;
}


// Due to an incorrect behavior IE returns an object instead of a string
// in some scenarios. Calling toString at the object may correct this.
// Helper function used in sbxvalue.cxx
bool handleToStringForCOMObjects( SbxObject* pObj, SbxValue* pVal )
{
	bool bSuccess = false;

	SbUnoObject* pUnoObj = NULL;
	if( pObj != NULL && (pUnoObj = PTR_CAST(SbUnoObject,(SbxObject*)pObj)) != NULL )
	{
		// Only for native COM objects
		if( pUnoObj->isNativeCOMObject() )
		{
			SbxVariableRef pMeth = pObj->Find( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "toString" ) ), SbxCLASS_METHOD );
			if ( pMeth.Is() )
			{
				SbxValues aRes;
				pMeth->Get( aRes );
				pVal->Put( aRes );
                bSuccess = true;
			}
		}
	}
	return bSuccess;
}

