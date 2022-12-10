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



#ifndef _OSL_SECURITYIMPL_H_
#define _OSL_SECURITYIMPL_H_

#include <winnetwk.h>

#include <osl/security.h>

#define USER_BUFFER_SIZE 256

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _oslSecurityImpl {
    HANDLE m_hProfile;
	HANDLE m_hToken;
    sal_Unicode   m_User[USER_BUFFER_SIZE];
/* ts: Erweiterung um Fileserver-login */
	NETRESOURCEW *m_pNetResource;	
} oslSecurityImpl;

#ifdef __cplusplus
}
#endif

#endif 

