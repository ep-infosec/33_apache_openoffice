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



#ifndef INCLUDED_CONFIGMGR_SOURCE_TYPE_HXX
#define INCLUDED_CONFIGMGR_SOURCE_TYPE_HXX

#include "sal/config.h"

namespace com { namespace sun { namespace star { namespace uno {
    class Any;
    class Type;
} } } }

namespace configmgr {

enum Type {
    TYPE_ERROR, TYPE_NIL, TYPE_ANY, TYPE_BOOLEAN, TYPE_SHORT, TYPE_INT,
    TYPE_LONG, TYPE_DOUBLE, TYPE_STRING, TYPE_HEXBINARY, TYPE_BOOLEAN_LIST,
    TYPE_SHORT_LIST, TYPE_INT_LIST, TYPE_LONG_LIST, TYPE_DOUBLE_LIST,
    TYPE_STRING_LIST, TYPE_HEXBINARY_LIST };

bool isListType(Type type);

Type elementType(Type type);

com::sun::star::uno::Type mapType(Type type);

Type getDynamicType(com::sun::star::uno::Any const & value);

}

#endif
