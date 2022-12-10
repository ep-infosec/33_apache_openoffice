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
PRJNAME=odk
TARGET=copying

#----------------------------------------------------------------
.INCLUDE: settings.mk
.INCLUDE: $(PRJ)$/util$/makefile.pmk
#----------------------------------------------------------------

#----------------------------------------------------
# this makefile is only used for copying the example 
# files into the SDK
#----------------------------------------------------

FORMS_FILES=\
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/BooleanValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ButtonOperator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ComponentTreeTraversal.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ControlLock.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ControlValidation.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ControlValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DataAwareness.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DateValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentBasedExample.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentHelper.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentType.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/DocumentViewHelper.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/FLTools.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/FormLayer.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/GridFieldValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/InteractionRequest.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/KeyGenerator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ListSelectionValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/Makefile \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/NumericValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ProgrammaticScriptAssignment.odt \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SalesFilter.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SingleControlValidation.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SpreadsheetDocument.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SpreadsheetValueBinding.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/SpreadsheetView.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/TableCellTextBinding.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/TextValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/TimeValidator.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/UNO.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/ValueBinding.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/URLHelper.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/HsqlDatabase.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/RowSet.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Forms$/WaitForInput.java

DIR_FILE_LIST= \
	$(FORMS_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_forms_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_forms.txt

#--------------------------------------------------
# TARGETS
#--------------------------------------------------
all : \
	$(DIR_FILE_LIST) \
	$(DIR_FILE_FLAG)

#--------------------------------------------------
# use global rules
#--------------------------------------------------   
.INCLUDE: $(PRJ)$/util$/odk_rules.pmk
    
