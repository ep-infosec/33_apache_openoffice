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

PRJNAME=sal
TARGET=cbptest
LIBTARGET=NO

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- Files --------------------------------------------------------

#
# test clipboard paste
#
	# --- Resources ----------------------------------------------------
	RCFILES=  cbptest.rc

	OBJFILES= $(OBJ)$/cbptest.obj

	APP1TARGET=	$(TARGET)
	APP1OBJS=	$(OBJFILES)
	APP1NOSAL=  TRUE
	APP1NOSVRES= $(RES)$/$(TARGET).res

	APP1STDLIBS+=$(OLE32LIB) $(USER32LIB) $(KERNEL32LIB)
	
	APP1LIBS=$(LB)$/ole9x.lib \
			 $(LB)$/tools32.lib 

	APP1DEPN=   makefile.mk $(APP1NOSVRES)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk


