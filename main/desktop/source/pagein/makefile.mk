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

PRJNAME=desktop
TARGET=pagein
TARGETTYPE=CUI
LIBTARGET=NO

NO_DEFAULT_STL=TRUE
LIBSALCPPRT=$(0)

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.INCLUDE .IGNORE : icuversion.mk

# --- Files --------------------------------------------------------

OBJFILES= \
	$(OBJ)$/pagein.obj \
	$(OBJ)$/file_image_unx.obj

APP1TARGET=$(TARGET)
APP1OBJS=$(OBJFILES)
APP1CODETYPE=C

# depends on libc only.
STDLIB=

# --- Targets ------------------------------------------------------

ALL: \
	$(MISC)$/$(TARGET)-calc    \
	$(MISC)$/$(TARGET)-draw    \
	$(MISC)$/$(TARGET)-impress \
	$(MISC)$/$(TARGET)-writer  \
	$(MISC)$/$(TARGET)-common  \
    ALLTAR

.INCLUDE :  target.mk

ICUDLLPOST=$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR)
UDKDLLPOST=$(DLLPOST).$(UDK_MAJOR)
UNODLLPOST=.uno$(DLLPOST)
DFTDLLPOST=$(DLLPOSTFIX)$(DLLPOST) # Default 

URELIBPATH=..$/ure-link$/lib

$(MISC)$/$(TARGET)-calc : makefile.mk
	@echo Making: $@
	@-echo $(DLLPRE)sc$(DFTDLLPOST)  >  $@
	@-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
	@-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-draw : makefile.mk
	@echo Making: $@
	@-echo $(DLLPRE)sd$(DFTDLLPOST)  >  $@
	@-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
	@-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-impress : makefile.mk
	@echo Making: $@
	@-echo $(DLLPRE)sd$(DFTDLLPOST)  >  $@
	@-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
	@-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

$(MISC)$/$(TARGET)-writer : makefile.mk
	@echo Making: $@
	@-echo $(DLLPRE)sw$(DFTDLLPOST)  >  $@
	@-echo $(DLLPRE)svx$(DFTDLLPOST) >> $@
	@-echo $(DLLPRE)svxcore$(DFTDLLPOST) >> $@

# sorted in approx. reverse load order (ld.so.1)
$(MISC)$/$(TARGET)-common : makefile.mk
	@echo Making: $@
	@-echo i18npool$(UNODLLPOST)         >  $@
.IF "$(SYSTEM_ICU)" != "YES"
	@-echo $(DLLPRE)icui18n$(ICUDLLPOST) >> $@
	@-echo $(DLLPRE)icule$(ICUDLLPOST)   >> $@
	@-echo $(DLLPRE)icuuc$(ICUDLLPOST)   >> $@
	@-echo $(DLLPRE)icudata$(ICUDLLPOST) >> $@
.ENDIF # SYSTEM_ICU
#
	@-echo $(DLLPRE)lng$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)xo$(DFTDLLPOST)    >> $@
#

	@-echo $(DLLPRE)fwe$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)fwk$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)fwi$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)fwl$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)package2$(DLLPOST) >> $@
	@-echo $(DLLPRE)ucpfile1$(DLLPOST) >> $@
	@-echo $(DLLPRE)ucb1$(DLLPOST)     >> $@
	@-echo $(DLLPRE)configmgr$(DLLPOST) >> $@
#
	@-echo $(DLLPRE)dtrans$(DLLPOST)   >> $@
	@-echo $(DLLPRE)vclplug_gen$(DFTDLLPOST) >> $@
.IF "$(ENABLE_GTK)" != ""
	@-echo $(DLLPRE)vclplug_gtk$(DFTDLLPOST) >> $@
.ENDIF # ENABLE_GTK
.IF "$(ENABLE_KDE)" != ""
	@-echo $(DLLPRE)vclplug_kde$(DFTDLLPOST) >> $@
.ENDIF # ENABLE_KDE
#
	@-echo $(DLLPRE)basegfx$(DFTDLLPOST) >> $@
	@-echo $(DLLPRE)sot$(DFTDLLPOST)     >> $@
	@-echo $(DLLPRE)xcr$(DFTDLLPOST)     >> $@
	@-echo $(DLLPRE)sb$(DFTDLLPOST)      >> $@
#
# uno runtime environment
#
	@-echo $(URELIBPATH)$/stocservices$(UNODLLPOST)         >> $@
	@-echo $(URELIBPATH)$/bootstrap$(UNODLLPOST)            >> $@
	@-echo $(URELIBPATH)$/$(DLLPRE)reg$(UDKDLLPOST)         >> $@
	@-echo $(URELIBPATH)$/$(DLLPRE)store$(UDKDLLPOST)       >> $@
	@-echo $(URELIBPATH)$/$(DLLPRE)uno_cppuhelper$(COMID)$(UDKDLLPOST) >> $@
	@-echo $(URELIBPATH)$/$(DLLPRE)uno_cppu$(UDKDLLPOST)               >> $@
	@-echo $(URELIBPATH)$/$(DLLPRE)uno_sal$(UDKDLLPOST)                >> $@
#
	@-echo $(DLLPRE)ucbhelper$(UCBHELPER_MAJOR)$(COMID)$(DLLPOST) >> $@
	@-echo $(DLLPRE)comphelp$(COMPHLP_MAJOR)$(COMID)$(DLLPOST)    >> $@
	@-echo $(DLLPRE)tl$(DFTDLLPOST)    >> $@
	@-echo $(DLLPRE)utl$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)svl$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)vcl$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)tk$(DFTDLLPOST)    >> $@
	@-echo $(DLLPRE)svt$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)sfx$(DFTDLLPOST)   >> $@
	@-echo $(DLLPRE)sofficeapp$(DLLPOST) >> $@
