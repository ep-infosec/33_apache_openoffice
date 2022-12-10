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



PRJ := ..$/..$/..
PRJNAME := sal

TARGET := qa_rtl_strings

ENABLE_EXCEPTIONS := TRUE

.INCLUDE: settings.mk

.IF "$(ENABLE_UNIT_TESTS)" != "YES"
all:
	@echo unit tests are disabled. Nothing to do.

.ELSE


CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

APP1TARGET := $(TARGET)
APP1OBJS := \
    $(SLO)$/test_oustringbuffer_utf32.obj \
    $(SLO)$/test_oustring_compare.obj \
    $(SLO)$/test_oustring_convert.obj \
    $(SLO)$/test_oustring_endswith.obj \
    $(SLO)$/test_oustring_noadditional.obj
APP1STDLIBS := $(SALLIB) $(GTESTLIB) $(TESTSHL2LIB)
APP1RPATH = NONE
APP1TEST = enabled

.INCLUDE: target.mk

.ENDIF # "$(ENABLE_UNIT_TESTS)" != "YES"
