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

PRJNAME=xmlhelp
TARGET=searchdemo
TARGETTYPE = CUI
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings ---

.INCLUDE : settings.mk


.IF "$(GUI)"=="WNT"
CFLAGS+=-GR
.ENDIF

# --- Files ---

OBJFILES=\
	$(OBJ)$/searchdemo.obj

APP1TARGET=	searchdemo
APP1OBJS=\
	$(OBJ)$/searchdemo.obj

APP1STDLIBS=\
	$(SALLIB) \
	$(VOSLIB)

APP1LIBS=\
	$(SLB)/jaqe.lib \
	$(SLB)/jadb.lib \
	$(SLB)/jautil.lib

APP1DEF=	$(MISC)\$(APP1TARGET).def

# --- Targets ---

.INCLUDE : target.mk

