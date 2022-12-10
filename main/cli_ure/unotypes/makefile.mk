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



PRJ = ..
PRJNAME = cli_ure

# for dummy
TARGET = unotypes

.INCLUDE : settings.mk
.INCLUDE : $(PRJ)$/util$/makefile.pmk

.INCLUDE : $(PRJ)$/util$/target.pmk
.INCLUDE : target.mk

.IF "$(BUILD_FOR_CLI)" != ""

.INCLUDE : $(BIN)$/cliureversion.mk

POLICY_ASSEMBLY_FILE=$(BIN)/$(CLI_URETYPES_POLICY_ASSEMBLY).dll

ALLTAR : \
	$(OUT)$/bin$/cli_uretypes.dll \
	$(POLICY_ASSEMBLY_FILE)

CLIMAKERFLAGS =
.IF "$(debug)" != ""
CLIMAKERFLAGS += --verbose
.ENDIF

#When changing the assembly version then this must also be done in scp2
$(OUT)$/bin$/cli_uretypes.dll : $(BIN)$/climaker.exe $(SOLARBINDIR)$/types.rdb $(BIN)$/cliureversion.mk
	$(subst,$(SOLARBINDIR)$/climaker,$(BIN)$/climaker $(CLIMAKER)) $(CLIMAKERFLAGS) \
		--out $@ \
		--keyfile $(BIN)$/cliuno.snk \
		--assembly-version $(CLI_URETYPES_NEW_VERSION) \
		--assembly-description "This assembly contains metadata for the Apache OpenOffice API." \
		--assembly-company "Apache Software Foundation" \
		$(SOLARBINDIR)$/udkapi.rdb

#do not forget to deliver cli_uretypes.config. It is NOT embedded in the policy file.
#see i62886 for the dependency on cli_uretypes.dll
$(POLICY_ASSEMBLY_FILE) : $(BIN)$/cli_uretypes.config $(OUT)$/bin$/cli_uretypes.dll
	$(WRAPCMD) AL.exe -out:$@ \
			-version:$(CLI_URETYPES_POLICY_VERSION) \
			-keyfile:$(BIN)$/cliuno.snk \
			-link:$(BIN)$/cli_uretypes.config

#Create the config file that is used with the policy assembly
$(BIN)$/cli_uretypes.config: cli_uretypes_config $(BIN)$/cliureversion.mk 
	$(PERL) $(SOLARENV)$/bin$/clipatchconfig.pl \
	$< $@


.ENDIF
