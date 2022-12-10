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



PRJ=.

PRJNAME=hyphen
TARGET=hyphen

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.IF "$(ENABLE_HYPHEN)" != "YES"

all:
	@echo "hyphen is disabled"

.ELSE

TARFILE_NAME=hyphen-2.7.1
TARFILE_MD5=48a9f787f43a09c0a9b7b00cd1fddbbf

ADDITIONAL_FILES += makefile.mk

PATCH_FILES= \
	hyphen-2.7.1.patch \
	hyphen-2.7.1-read-charset.patch

.IF "$(GUI)"=="UNX"
CONFIGURE_DIR=$(BUILD_DIR)

#relative to CONFIGURE_DIR
# still needed also in system-hyphen case as it creates the makefile
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic

.IF "$(COM)"=="C52" && "$(CPU)"=="U"
LCL_CONFIGURE_CFLAGS+=-m64
.ENDIF

.IF "$(SYSBASE)"!=""
.IF "$(EXTRA_CFLAGS)"!=""
LCL_CONFIGURE_CFLAGS+=$(EXTRA_CFLAGS)
CONFIGURE_FLAGS+=CXXFLAGS="$(EXTRA_CFLAGS)"
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ELIF "$(OS)"=="MACOSX" # "$(SYSBASE)"!=""
CONFIGURE_FLAGS+=CPPFLAGS="$(EXTRA_CDEFS)"
.ENDIF

.IF "$(LCL_CONFIGURE_CFLAGS)"!=""
CONFIGURE_FLAGS+=CFLAGS='$(LCL_CONFIGURE_CFLAGS)'
.ENDIF

.IF "$(SYSTEM_HYPH)" == "YES" && "$(WITH_MYSPELL_DICTS)" == "YES"
BUILD_ACTION=make hyph_en_US.dic
.ELIF "$(SYSTEM_HYPH)" == "YES" && "$(WITH_MYSPELL_DICTS)" != "YES"
@all:
	echo "Nothing to do here."
.ELSE
BUILD_ACTION=$(GNUMAKE) -j$(EXTMAXPROCESS)
OUT2INC += hyphen.h 
.ENDIF

.ENDIF # "$(GUI)"=="UNX"


.IF "$(GUI)"=="WNT"
.IF "$(COM)"=="GCC"
CONFIGURE_ACTION=configure
CONFIGURE_FLAGS= --disable-shared --with-pic

BUILD_ACTION=make

.ELSE
BUILD_ACTION=dmake
.ENDIF # "$(COM)"=="GCC"
OUT2INC += hyphen.h
.ENDIF # "$(GUI)"=="WNT"

.IF "$(GUI)"=="OS2"
BUILD_ACTION=dmake
OUT2INC += hyphen.h
.ENDIF # "$(GUI)"=="OS2"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE : target.mk
.INCLUDE : tg_ext.mk

.ENDIF # "$(ENABLE_HYPHEN)"
