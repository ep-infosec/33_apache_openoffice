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



#if ! defined INCLUDED_CLI_ENVIRONMENT_H
#define INCLUDED_CLI_ENVIRONMENT_H

#include "cli_base.h"
#using <mscorlib.dll>

using namespace System;
using namespace System::Collections;
using namespace System::Runtime::Serialization;

namespace cli_uno
{

public __gc class Cli_environment
{
    static System::String* sOidPart;
    static  Hashtable* m_objects;
    static System::Runtime::Serialization::ObjectIDGenerator* m_IDGen;
    inline static System::String* createKey(System::String* oid, System::Type* t);

#if OSL_DEBUG_LEVEL >= 1
    int _numRegisteredObjects;
#endif     

public:
    
    static Cli_environment()
    {
        m_objects = Hashtable::Synchronized(new Hashtable());
        m_IDGen = new System::Runtime::Serialization::ObjectIDGenerator();
        System::Text::StringBuilder* buffer = new System::Text::StringBuilder(256);
        Guid gd = Guid::NewGuid();
        buffer->Append(S";cli[0];");
		buffer->Append(gd.ToString());
        sOidPart = buffer->ToString();
    }

    inline Cli_environment();

    ~Cli_environment();
    
    /**
       Registers an UNO object as being mapped by this bridge. The resulting
       cli object is represents all interfaces of the UNO object. Therefore the
       object can be registered only with its OID; a type is not necessary.
    */
    Object* registerInterface(Object* obj, System::String* oid);
    /**
       Registers a CLI object as being mapped by this bridge. The resulting
       object represents exactly one UNO interface.
     */
    Object* registerInterface(Object* obj, System::String* oid, System::Type* type);

    /**
       By revoking an interface it is declared that the respective interface has
       not been mapped. The proxy implementations call revoke interface in their
       destructors.
     */
    inline void revokeInterface(System::String* oid);
    
    void revokeInterface(System::String* oid, System::Type* type);
    /**
     * Retrieves an interface identified by its object id and type from this
     * environment.
     *
     * @param oid object id of interface to be retrieved
     * @param type the type description of the interface to be retrieved
     * @see com.sun.star.uno.IEnvironment#getRegisteredInterface
     */
    Object* getRegisteredInterface(System::String* oid, System::Type* type);

	/**
	 * Generates a worldwide unique object identifier (oid) for the given object. It is
     * guaranteed, that subsequent calls to the method with the same object
     * will give the same id.
	 * <p>
	 * @return     the generated oid.
	 * @param      object     the object for which a Oid should be generated.
	 */
    static System::String* getObjectIdentifier(Object* obj);

};

} //namespace cli_uno


#endif
