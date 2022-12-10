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



#ifndef INCLUDED_RTL_TEXTENC_CONVERTISO2022KR_H
#define INCLUDED_RTL_TEXTENC_CONVERTISO2022KR_H

#include "tenchelp.h"
#include "sal/types.h"

#if defined __cplusplus
extern "C" {
#endif /* __cpluscplus */

typedef struct
{
    ImplDBCSToUniLeadTab const * m_pKsX1001ToUnicodeData;
    ImplUniToDBCSHighTab const * m_pUnicodeToKsX1001Data;
} ImplIso2022KrConverterData;

void * ImplCreateIso2022KrToUnicodeContext(void) SAL_THROW_EXTERN_C();

void ImplResetIso2022KrToUnicodeContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertIso2022KrToUnicode(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Char const * pSrcBuf,
                                       sal_Size nSrcBytes,
                                       sal_Unicode * pDestBuf,
                                       sal_Size nDestChars,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtBytes)
    SAL_THROW_EXTERN_C();

void * ImplCreateUnicodeToIso2022KrContext(void) SAL_THROW_EXTERN_C();

void ImplResetUnicodeToIso2022KrContext(void * pContext) SAL_THROW_EXTERN_C();

sal_Size ImplConvertUnicodeToIso2022Kr(ImplTextConverterData const * pData,
                                       void * pContext,
                                       sal_Unicode const * pSrcBuf,
                                       sal_Size nSrcChars,
                                       sal_Char * pDestBuf,
                                       sal_Size nDestBytes,
                                       sal_uInt32 nFlags,
                                       sal_uInt32 * pInfo,
                                       sal_Size * pSrcCvtChars)
    SAL_THROW_EXTERN_C();

#if defined __cplusplus
}
#endif /* __cpluscplus */

#endif /* INCLUDED_RTL_TEXTENC_CONVERTISO2022KR_H */
