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



$(eval $(call gb_Module_Module,svl))

$(eval $(call gb_Module_add_targets,svl,\
	AllLangResTarget_svl \
	Library_fsstorage \
	Library_passwordcontainer \
	Library_svl \
	Package_inc \
))

ifeq ($(ENABLE_UNIT_TESTS),YES)
$(eval $(call gb_Module_add_check_targets,svl,\
	GoogleTest_svl_urihelper \
))
endif

ifneq ($(OOO_JUNIT_JAR),)
$(eval $(call gb_Module_add_subsequentcheck_targets,svl,\
	JunitTest_svl_complex \
))
endif

#todo: dde platform dependent
#todo: package_inc
#todo: map file

# vim: set noet ts=4 sw=4:
