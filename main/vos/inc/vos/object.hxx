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




#ifndef _VOS_OBJECT_HXX_
#define _VOS_OBJECT_HXX_

#	include <vos/types.hxx>
#	include <vos/macros.hxx>
#	include <vos/vosdllapi.h>

namespace vos
{     

// ***************************************
// Object super class

struct OClassInfo;
struct OCreateParam;

/** OObject
	common base class for all framework classes. Used for memory-management
	and runtime type-info.
*/
class VOS_DLLPUBLIC OObject
{
public:

	///
	OObject();

	///
	OObject(const OCreateParam& rParam);

	// Disable the copy constructor and assignment by default so you will get
	// compiler errors instead of unexpected behaviour if you pass objects
	// by value or assign objects.
private:
	OObject(const OObject& objectSrc);			// no implementation
	void SAL_CALL operator=(const OObject& objectSrc);	// no implementation

public:
	virtual ~OObject();

public:

	/** Define private new and delete operator because of compiler bug, 
		when allocating and deleteing a exported class
	*/
	void* SAL_CALL operator new(size_t size);
	void* SAL_CALL operator new(size_t size, void* p);

	void  SAL_CALL operator delete(void* p);

// Attributes
public:

	///
	virtual const OClassInfo& SAL_CALL getClassInfo() const;

	///
	sal_Bool SAL_CALL isKindOf(const OClassInfo& rClass) const;

// Implementation
public:
	static const OClassInfo& SAL_CALL classInfo();

public:
	static OClassInfo __ClassInfo__;
};


/**
	Basic class information
*/
struct OCreateParam
{
	sal_uInt32 m_Size;
	void*	 m_pParam;

	///
	OCreateParam(void *pParam)
	{	
		m_Size = sizeof(OCreateParam);
		m_pParam = pParam;
	}
};

/**
*/
struct VOS_DLLPUBLIC OClassInfo
{
	///
	const sal_Char  *m_pClassName;
	///
	sal_Int32			 m_nObjectSize;
	/// schema number of the loaded class
	sal_uInt32	 m_wSchema;				

	///
	OObject* (SAL_CALL * m_pfnCreateObject)(const OCreateParam&);	// NULL => abstract class

	/// linked list of registered classes
	const OClassInfo* m_pBaseClass;
	/// linked list of registered classes
	const OClassInfo* m_pNextClass;			

	///
	OObject* SAL_CALL createObject(const OCreateParam& rParam) const;

	///
	sal_Bool SAL_CALL isDerivedFrom(const OClassInfo& rBaseClass) const;

	///
	static const OClassInfo* SAL_CALL getClassInfo(const sal_Char* pClassName);

	///
	OClassInfo(const sal_Char *pClassName, sal_Int32 ObjectSize,
		       const OClassInfo* pBaseClass = NULL, sal_uInt32 Schema = (sal_uInt32)-1,
			   OObject* (SAL_CALL * fnCreateObject)(const OCreateParam&) = NULL); 
};

// *****************************************************************
// Helper macros for declaring OClassInfo data


#define VOS_STRINGIZE(name) #name

#define VOS_CLASSNAME(class_name, domain_name) VOS_STRINGIZE(domain_name.class_name)

#define VOS_CLASSINFO(class_name) (class_name::classInfo())

// generate static object constructor for class registration
struct VOS_CLASSINIT
{ VOS_CLASSINIT(VOS_NAMESPACE(OClassInfo, vos)* pNewClass); };

#define VOS_CLASSDATA(class_spec, class_name, base_class_name, wSchema, constructor) \
	VOS_NAMESPACE(OClassInfo, vos) class_name::__ClassInfo__(class_spec, \
	    sizeof(class_name), &VOS_CLASSINFO(base_class_name), wSchema, constructor); \
    const VOS_NAMESPACE(VOS_CLASSINIT, vos) class_name::__ClassInit__(&class_name::__ClassInfo__); \
    const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL class_name::getClassInfo() const \
		{ return (VOS_CLASSINFO(class_name)); } \
    const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL class_name::classInfo() \
		{ return (__ClassInfo__); }
	
#define VOS_DECLARE_CLASSINFO(class_name) \
public: \
	static const VOS_NAMESPACE(VOS_CLASSINIT, vos) __ClassInit__; \
    static VOS_NAMESPACE(OClassInfo, vos) __ClassInfo__; \
public: \
	virtual const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL getClassInfo() const; \
    static const VOS_NAMESPACE(OClassInfo, vos)& SAL_CALL classInfo()

#define VOS_IMPLEMENT_CLASSINFO(class_spec, class_name, base_class_name, wSchema) \
	VOS_CLASSDATA(class_spec, class_name, base_class_name, wSchema, NULL)

#define VOS_DECLARE_CLASSTYPE(class_name) \
	VOS_DECLARE_CLASSINFO(class_name); \
public: \
	static VOS_NAMESPACE(OObject, vos)* SAL_CALL createObject(const VOS_NAMESPACE(OCreateParam, vos)& rParam);

#define VOS_IMPLEMENT_CLASSTYPE(class_spec, class_name, base_class_name, wSchema) \
	VOS_CLASSDATA(class_spec, class_name, base_class_name, wSchema, class_name::createObject) \
	VOS_NAMESPACE(OObject, vos)* class_name::createObject(const VOS_NAMESPACE(OCreateParam, vos)& rParam) \
		{ return new class_name(rParam); }

}

#endif // _VOS_OBJECT_HXX_

