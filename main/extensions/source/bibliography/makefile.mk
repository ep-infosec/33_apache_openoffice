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

PRJNAME=extensions
TARGET=bib
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :	$(PRJ)$/util$/makefile.pmk

# --- Files --------------------------------------------------------

SLOFILES=			$(SLO)$/formcontrolcontainer.obj \
					$(SLO)$/loadlisteneradapter.obj \
					$(SLO)$/bibconfig.obj \
					$(SLO)$/bibcont.obj \
					$(SLO)$/bibload.obj \
					$(SLO)$/bibmod.obj \
					$(SLO)$/general.obj \
					$(SLO)$/framectr.obj \
					$(SLO)$/bibview.obj \
					$(SLO)$/bibbeam.obj  \
					$(SLO)$/toolbar.obj  \
					$(SLO)$/datman.obj	\


SRS1NAME=$(TARGET)
SRC1FILES=			bib.src	 \
					datman.src \
					sections.src \
					toolbar.src

RESLIB1NAME=bib
RESLIB1SRSFILES= $(SRS)$/bib.srs
RESLIB1DEPN= bib.src bib.hrc


SHL1STDLIBS= \
        $(SOTLIB)           \
        $(SVTOOLLIB) \
		$(TKLIB) \
		$(VCLLIB) \
		$(SVLLIB) \
		$(UNOTOOLSLIB) \
		$(TOOLSLIB) \
		$(DBTOOLSLIB) \
		$(COMPHELPERLIB) \
		$(CPPUHELPERLIB) \
		$(CPPULIB) \
		$(SALLIB)


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=		i$(TARGET)
SHL1DEPN=		$(SHL1LIBS)
SHL1DEF=		$(MISC)$/$(SHL1TARGET).def
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
DEF1NAME=		$(SHL1TARGET)

# --- Targets ------------------------------------------------------

.INCLUDE :	target.mk


ALLTAR : $(MISC)/bib.component

$(MISC)/bib.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        bib.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt bib.component
