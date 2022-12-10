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

CHARTS_FILES=\
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/AddInChart.ods \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/CalcHelper.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/ChartHelper.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/ChartInCalc.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/ChartInDraw.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/ChartInWriter.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/Helper.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/JavaSampleChartAddIn.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/ListenAtCalcRangeInDraw.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/SelectionChangeListener.java \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/bullet.gif \
	$(DESTDIRDEVGUIDEEXAMPLES)$/Charts$/Makefile

DIR_FILE_LIST= \
    $(CHARTS_FILES)

DIR_DIRECTORY_LIST=$(uniq $(DIR_FILE_LIST:d))
DIR_CREATE_FLAG=$(MISC)$/devguide_charts_dirs_created.txt
DIR_FILE_FLAG=$(MISC)$/devguide_charts.txt

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


