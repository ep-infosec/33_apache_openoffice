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



PRJ := ..$/..
PRJNAME := testtools
TARGET := test_cli
PACKAGE = clitest

.INCLUDE: settings.mk

#----- compile .java files -----------------------------------------

JARFILES = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar
JAVAFILES       = CLITest.java
JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

ALLTAR:

EXETARGET = $(BIN)$/cli_bridgetest_inprocess.exe
EXEARG_WIN= $(BIN)$/cli_bridgetest_inprocess.ini

EXEARG= $(strip $(subst,$/,/ $(EXEARG_WIN))) 

.IF "$(depend)" == ""
ALL: ALLTAR
.ELSE
ALL: ALLDEP
.ENDIF

.INCLUDE: target.mk

# --- Parameters for the test --------------------------------------

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

CT_NOOFFICE = -NoOffice
# --- Targets ------------------------------------------------------

RUN:
.IF "$(GUI)"=="WNT" 
    java -cp $(CLASSPATH) -Dcli_test=$(EXETARGET) -Dcli_test_arg=$(EXEARG) $(CT_APP) $(CT_NOOFFICE) $(CT_TESTBASE) $(CT_TEST) 	
.ENDIF
run: RUN
