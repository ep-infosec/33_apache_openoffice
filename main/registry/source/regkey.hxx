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



#ifndef INCLUDED_REGISTRY_SOURCE_REGKEY_HXX
#define INCLUDED_REGISTRY_SOURCE_REGKEY_HXX

#include "sal/config.h"
#include "registry/regtype.h"
#include "rtl/ustring.h"
#include "sal/types.h"

extern "C" {

void REGISTRY_CALLTYPE acquireKey(RegKeyHandle);
void REGISTRY_CALLTYPE releaseKey(RegKeyHandle);
sal_Bool REGISTRY_CALLTYPE isKeyReadOnly(RegKeyHandle);
RegError REGISTRY_CALLTYPE getKeyName(RegKeyHandle, rtl_uString**);
RegError REGISTRY_CALLTYPE createKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openSubKeys(
    RegKeyHandle, rtl_uString*, RegKeyHandle**, sal_uInt32*);
RegError REGISTRY_CALLTYPE closeSubKeys(RegKeyHandle*, sal_uInt32);
RegError REGISTRY_CALLTYPE deleteKey(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE closeKey(RegKeyHandle);
RegError REGISTRY_CALLTYPE setValue(
    RegKeyHandle, rtl_uString*, RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE setLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32*, sal_uInt32);
RegError REGISTRY_CALLTYPE setStringListValue(
    RegKeyHandle, rtl_uString*, sal_Char**, sal_uInt32);
RegError REGISTRY_CALLTYPE setUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
RegError REGISTRY_CALLTYPE getValueInfo(
    RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
RegError REGISTRY_CALLTYPE getValue(RegKeyHandle, rtl_uString*, RegValue);
RegError REGISTRY_CALLTYPE getLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
RegError REGISTRY_CALLTYPE getStringListValue(
    RegKeyHandle, rtl_uString*, sal_Char***, sal_uInt32*);
RegError REGISTRY_CALLTYPE getUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeValueList(RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE createLink(RegKeyHandle, rtl_uString*, rtl_uString*);
RegError REGISTRY_CALLTYPE deleteLink(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE getKeyType(RegKeyHandle, rtl_uString*, RegKeyType*);
RegError REGISTRY_CALLTYPE getLinkTarget(
    RegKeyHandle, rtl_uString*, rtl_uString**);
RegError REGISTRY_CALLTYPE getResolvedKeyName(
    RegKeyHandle, rtl_uString*, sal_Bool, rtl_uString**);
RegError REGISTRY_CALLTYPE getKeyNames(
    RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeKeyNames(rtl_uString**, sal_uInt32);

}

#endif
