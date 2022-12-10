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

PRJNAME=cpputools
TARGET=regcomp
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

NO_OFFUH=TRUE
CPPUMAKERFLAGS += -C

UNOTYPES=\
		 	com.sun.star.uno.TypeClass \
		 	com.sun.star.lang.XMultiServiceFactory \
		 	com.sun.star.lang.XSingleServiceFactory \
		 	com.sun.star.lang.XMultiComponentFactory \
		 	com.sun.star.lang.XSingleComponentFactory \
		 	com.sun.star.lang.XComponent \
		 	com.sun.star.container.XContentEnumerationAccess \
		 	com.sun.star.container.XSet \
		 	com.sun.star.loader.CannotActivateFactoryException \
			com.sun.star.registry.XImplementationRegistration2

# --- Files --------------------------------------------------------

DEPOBJFILES=   $(OBJ)$/registercomponent.obj 

APP1TARGET= $(TARGET)
APP1OBJS=$(DEPOBJFILES)  
APP1RPATH=UREBIN

APP1STDLIBS=\
			$(SALLIB) \
			$(CPPULIB)	\
			$(CPPUHELPERLIB)

.IF "$(GUI)"=="WNT"
APP1STDLIBS+= \
			$(LIBCMT)
.ENDIF


.INCLUDE :  target.mk
