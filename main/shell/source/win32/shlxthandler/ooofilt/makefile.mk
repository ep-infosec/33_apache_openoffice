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



PRJ=..$/..$/..$/..
PRJNAME=shell
TARGET=ooofilt
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# Do not use the dynamic STLport library.
NO_DEFAULT_STL=YES

# Do not use the uwinapi library
UWINAPILIB=


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS+=-D_WIN32_IE=0x501
# SCPCDEFS+=-D_STLP_USE_STATIC_LIB 

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/ooofilt.obj\
        $(SLO)$/propspec.obj\
        $(SLO)$/stream_helper.obj

#       $(SLO)$/utilities.obj
#        $(SLO)$/dbgmacros.obj

SHL1TARGET=$(TARGET)
.IF "$(COM)"=="GCC"
SHL1STDLIBS=$(ZLIB3RDLIB) $(EXPAT3RDLIB)
SHL1LIBS=
.ELSE
SHL1STDLIBS=
SHL1LIBS=$(SOLARLIBDIR)$/zlib.lib\
    $(SOLARLIBDIR)$/expat_xmlparse.lib\
    $(SOLARLIBDIR)$/expat_xmltok.lib
.ENDIF
SHL1STDLIBS+=$(OLE32LIB)\
     $(ADVAPI32LIB)\
     $(COMCTL32LIB)\
     $(UUIDLIB)\
     $(SHELL32LIB)\
     $(KERNEL32LIB)\
     $(OLDNAMESLIB)

.IF "$(COM)"!="GCC"
SHL1STDLIBS+=msvcprt.lib

.IF "$(PRODUCT)"!="full"
SHL1STDLIBS+=msvcrt.lib
.ENDIF
.ENDIF

	 
#     $(LIBSTLPORTST)
     
SHL1LIBS+=$(SLB)$/util.lib\
    $(SLB)$/ooofilereader.lib
    
SHL1DEPN=
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp


.IF "$(BUILD_X64)"!=""
#---------------------------- x64 -------------------------
CFLAGS_X64+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CFLAGS_X64+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
CDEFS_X64+=-D_WIN32_IE=0x501
USE_DEFFILE_X64=TRUE

SLOFILES_X64=$(SLO_X64)$/ooofilt.obj\
        $(SLO_X64)$/propspec.obj\
        $(SLO_X64)$/stream_helper.obj

SHL1TARGET_X64=$(TARGET)

SHL1LIBS_X64=$(SOLARLIBDIR_X64)$/zlib.lib\
    $(SOLARLIBDIR_X64)$/expat_xmlparse.lib\
    $(SOLARLIBDIR_X64)$/expat_xmltok.lib

SHL1STDLIBS_X64+=$(OLE32LIB_X64)\
     $(ADVAPI32LIB_X64)\
     $(COMCTL32LIB_X64)\
     $(UUIDLIB_X64)\
	 $(USER32LIB_X64) \
     $(SHELL32LIB_X64)\
     $(KERNEL32LIB_X64)\
	 $(MSVCPRT_X64) \
	 $(MSVCRT_X64) \
     $(OLDNAMESLIB_X64)

SHL1LIBS_X64+=$(SLB_X64)$/util.lib\
    $(SLB_X64)$/ooofilereader.lib
SHL1OBJS_X64=$(SLOFILES_X64)
SHL1DEF_X64=$(MISC_X64)$/$(SHL1TARGET_X64).def
DEF1NAME_X64=$(SHL1TARGET_X64)
DEF1EXPORTFILE_X64=exports.dxp
.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))

.INCLUDE :	tg_wntx64.mk
