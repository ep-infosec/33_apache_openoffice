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

# if no environment is set, try to find one, source it and re-exec
ifeq ($(strip $(SOLARENV)),)

all:
	if test -f ./source_soenv.sh; then . ./source_soenv.sh; fi && \
	if test -z "$${SOLARENV}"; then echo "No environment set!"; exit 1; fi && \
	$(MAKE)

else # SOLARENV

GBUILDDIR := $(SOLARENV)/gbuild
include $(GBUILDDIR)/gbuild.mk

$(foreach repo,$(gb_REPOS),$(eval $(call gb_Module_make_global_targets,$(wildcard $(repo)/Module_*.mk))))

endif # SOLARENV

# vim: set noet sw=4 ts=4:
