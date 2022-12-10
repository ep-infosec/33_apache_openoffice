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


PRJ=..$/..

PRJNAME=	stoc
TARGET = reflection.uno
ENABLE_EXCEPTIONS=TRUE
COMP1TYPELIST = corefl

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
DLLPRE =

# ------------------------------------------------------------------

.INCLUDE :  ..$/cppumaker.mk

SLOFILES=	\
		$(SLO)$/crefl.obj	\
		$(SLO)$/crbase.obj	\
		$(SLO)$/crarray.obj	\
		$(SLO)$/crcomp.obj	\
		$(SLO)$/criface.obj	\
		$(SLO)$/crenum.obj

# internal compiler error with Forte 6 update 1 (x86)
# to be reconsidered after compiler upgrade
.IF "$(OS)$(CPU)"=="SOLARISI"
NOOPTFILES += $(SLO)$/criface.obj
.ENDIF

SHL1TARGET=	$(TARGET)
SHL1VERSIONMAP = $(SOLARENV)/src/unloadablecomponent.map
SHL1STDLIBS= \
		$(CPPULIB)		\
		$(CPPUHELPERLIB)	\
		$(SALLIB)

SHL1DEPN=
SHL1IMPLIB=	i$(TARGET)
SHL1LIBS=	$(SLB)$/$(TARGET).lib
SHL1DEF=	$(MISC)$/$(SHL1TARGET).def
SHL1RPATH=  URELIB

DEF1NAME=	$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk

ALLTAR : $(MISC)/reflection.component

$(MISC)/reflection.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        reflection.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_URE_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt reflection.component
