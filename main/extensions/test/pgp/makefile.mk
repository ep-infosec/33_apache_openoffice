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

PRJNAME := extensions
#PACKAGE := com$/sun$/star$/pgp
PACKAGE := 
TARGET  := test_com_sun_star_pgp


# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk


# Files --------------------------------------------------------


APPLICATRDB := $(SOLARBINDIR)$/applicat.rdb
RDB := $(APPLICATRDB)

JARFILES= jurt.jar

GENJAVACLASSFILES= \
	$(CLASSDIR)$/com$/sun$/star$/beans$/PropertyValue.class				\
	$(CLASSDIR)$/com$/sun$/star$/beans$/PropertyState.class				\
	$(CLASSDIR)$/com$/sun$/star$/container$/XSet.class				\


#$(CLASSDIR)$/com$/sun$/star$/awt$/FocusEvent.class

#	$(CLASSDIR)$/com$/sun$/star$/awt$/InputEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/KeyEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/MouseEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/WindowEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/PaintEvent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/Rectangle.class					\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XFocusListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XKeyListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XMouseListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XMouseMotionListener.class		\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XWindow.class						\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XWindowListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/awt$/XPaintListener.class				\
#	$(CLASSDIR)$/com$/sun$/star$/bridge$/XBridge.class					\
#	$(CLASSDIR)$/com$/sun$/star$/container$/XIndexAccess.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/FrameAction.class				\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/FrameActionEvent.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XComponentLoader.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XController.class				\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFrame.class					\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFrameActionListener.class		\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFrames.class					\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XFramesSupplier.class			\
#	$(CLASSDIR)$/com$/sun$/star$/frame$/XModel.class					\
#	$(CLASSDIR)$/com$/sun$/star$/io$/BufferSizeExceededException.class	\
#	$(CLASSDIR)$/com$/sun$/star$/io$/NotConnectedException.class		\
#	$(CLASSDIR)$/com$/sun$/star$/io$/XInputStream.class					\
#	$(CLASSDIR)$/com$/sun$/star$/io$/XOutputStream.class				\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/IndexOutOfBoundsException.class	\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/XComponent.class					\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/XMultiServiceFactory.class		\
#	$(CLASSDIR)$/com$/sun$/star$/lang$/XServiceInfo.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XSimpleText.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XText.class						\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextContent.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextCursor.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextDocument.class				\
#	$(CLASSDIR)$/com$/sun$/star$/text$/XTextRange.class					\
#	$(CLASSDIR)$/com$/sun$/star$/uno$/XNamingService.class		\
#	$(CLASSDIR)$/com$/sun$/star$/uno$/Enum.class


JAVACLASSFILES= \
	$(CLASSDIR)$/$(PACKAGE)$/TestPGP.class


TYPES={$(subst,.class, $(subst,$/,.  $(subst,$(CLASSDIR)$/,-T  $(GENJAVACLASSFILES))))}
GENJAVAFILES = {$(subst,.class,.java $(subst,$/class, $(GENJAVACLASSFILES)))}
JAVAFILES= $(subst,$(CLASSDIR)$/$(PACKAGE)$/, $(subst,.class,.java $(JAVACLASSFILES))) $(GENJAVAFILES) 


# --- Targets ------------------------------------------------------


.IF "$(depend)" == ""
ALL : $(GENJAVAFILES) ALLTAR 
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE :  target.mk

$(GENJAVAFILES) : $(RDB)
#	javamaker -BUCR -O$(OUT) $(TYPES) $(RDB)
	javamaker @$(mktmp -BUCR -O$(OUT) $(TYPES) $(RDB))

$(JAVACLASSFILES) : $(GENJAVAFILES)
