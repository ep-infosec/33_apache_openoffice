#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



PRJ=..$/..$/..

PRJNAME = xmlsecurity
TARGET = xs_mscrypt

ENABLE_EXCEPTIONS = TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :	$(PRJ)$/util$/target.pmk

.IF "$(CRYPTO_ENGINE)" != "mscrypto"
LIBTARGET=NO
.ENDIF

.IF "$(CRYPTO_ENGINE)" == "mscrypto"

.IF "$(WITH_MOZILLA)" == "NO" || "$(ENABLE_NSS_MODULE)"!="YES"
.IF "$(SYSTEM_MOZILLA)" != "YES"
@all:
	@echo "No mozilla -> no nss -> no libxmlsec -> no xmlsecurity/nss"
.ENDIF
.ENDIF

CDEFS += -DXMLSEC_CRYPTO_MSCRYPTO -DXMLSEC_NO_XSLT

# --- Files --------------------------------------------------------
INCLOCAL = \
	..

SLOFILES = \
	$(SLO)$/securityenvironment_mscryptimpl.obj \
	$(SLO)$/xmlencryption_mscryptimpl.obj \
	$(SLO)$/xmlsecuritycontext_mscryptimpl.obj \
	$(SLO)$/xmlsignature_mscryptimpl.obj \
	$(SLO)$/x509certificate_mscryptimpl.obj \
	$(SLO)$/seinitializer_mscryptimpl.obj \
	$(SLO)$/xsec_mscrypt.obj  \
    $(SLO)$/sanextension_mscryptimpl.obj

.ENDIF

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

