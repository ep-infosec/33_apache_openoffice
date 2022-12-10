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
INCPRE+= $(PRJ)$/qa$/inc

PRJNAME=sal
TARGET=rtl_Process

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
	@echo unit tests are disabled. Nothing to do.

.ELSE


CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# BEGIN ----------------------------------------------------------------
# auto generated Target:joblist by codegen.pl
APP1OBJS=  \
	$(SLO)$/rtl_Process.obj

APP1TARGET= rtl_Process
APP1STDLIBS= $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)
APP1RPATH = NONE
APP1TEST = enabled

# END ------------------------------------------------------------------

OBJ3FILES=$(OBJ)$/child_process.obj
APP3TARGET=child_process
APP3OBJS=$(OBJ3FILES)

#.IF "$(GUI)" == "UNX"
#APP3STDLIBS=$(LB)$/libsal.so
#.ENDIF
#.IF "$(GUI)" == "WNT"
#APP3STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
#.ENDIF
APP3STDLIBS=$(SALLIB)

OBJ4FILES=$(OBJ)$/child_process_id.obj
APP4TARGET=child_process_id
APP4OBJS=$(OBJ4FILES)

# .IF "$(GUI)" == "UNX"
# APP4STDLIBS=$(LB)$/libsal.so
# .ENDIF
# .IF "$(GUI)" == "WNT"
# APP4STDLIBS=$(KERNEL32LIB) $(LB)$/isal.lib
# .ENDIF
APP4STDLIBS=$(SALLIB)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
