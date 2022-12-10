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



PRJ = ..$/..
TARGET  = StorageUnitTest
PRJNAME = package
PACKAGE = complex$/storages

# --- Settings -----------------------------------------------------
.INCLUDE: settings.mk


#----- compile .java files -----------------------------------------

JARFILES        = ridl.jar unoil.jar jurt.jar juh.jar java_uno.jar OOoRunner.jar

JAVAFILES       =\
				StorageUnitTest.java\
				StorageTest.java\
				TestHelper.java\
				BorderedStream.java\
				Test01.java\
				Test02.java\
				Test03.java\
				Test04.java\
				Test05.java\
				Test06.java\
				Test07.java\
				Test08.java\
				Test09.java\
				Test10.java\
				Test11.java\
				Test12.java\
				Test13.java\
				Test14.java\
				Test15.java\
				Test16.java\
				Test17.java\
				Test18.java\
				RegressionTest_114358.java\
				RegressionTest_i29169.java\
				RegressionTest_i30400.java\
				RegressionTest_i29321.java\
				RegressionTest_i30677.java\
				RegressionTest_i27773.java\
				RegressionTest_i46848.java\
				RegressionTest_i55821.java\
				RegressionTest_i35095.java\
				RegressionTest_i49755.java\
				RegressionTest_i59886.java\
				RegressionTest_i61909.java\
				RegressionTest_i84234.java\
				RegressionTest_125919.java

JAVACLASSFILES	= $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:b).class)

#----- make a jar from compiled files ------------------------------

MAXLINELENGTH = 100000

JARCLASSDIRS    = $(PACKAGE)
JARTARGET       = $(TARGET).jar
JARCOMPRESS 	= TRUE

# --- Parameters for the test --------------------------------------

# start an office if the parameter is set for the makefile
.IF "$(OFFICE)" == ""
CT_APPEXECCOMMAND =
.ELSE
CT_APPEXECCOMMAND = -AppExecutionCommand "$(OFFICE)$/soffice -accept=socket,host=localhost,port=8100;urp;"
.ENDIF

# test base is java complex
CT_TESTBASE = -TestBase java_complex

# test looks something like the.full.package.TestName
CT_TEST     = -o $(PACKAGE:s\$/\.\).$(JAVAFILES:b)

# start the runner application
CT_APP      = org.openoffice.Runner

# --- Targets ------------------------------------------------------

.INCLUDE: target.mk

RUN: run

run:
    java -cp $(CLASSPATH) $(CT_APP) $(CT_TESTBASE) $(CT_APPEXECCOMMAND) $(CT_TEST)


