###############################################################
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
###############################################################


$(eval $(call gb_GoogleTest_GoogleTest,cppuhelper_ifcontainer))

$(eval $(call gb_GoogleTest_add_exception_objects,cppuhelper_ifcontainer, \
	cppuhelper/qa/ifcontainer/cppu_ifcontainer \
	cppuhelper/qa/ifcontainer/main \
))

$(eval $(call gb_GoogleTest_set_include,cppuhelper_ifcontainer,\
        $$(INCLUDE) \
	-I$(SRCDIR)/cppuhelper/inc \
	-I$(SRCDIR)/cppuhelper/inc/pch \
))

$(eval $(call gb_GoogleTest_add_api,cppuhelper_ifcontainer, \
 	udkapi \
))

$(eval $(call gb_GoogleTest_add_linked_libs,cppuhelper_ifcontainer, \
	cppu \
	cppuhelper \
	sal \
	stl \
	$(gb_STDLIBS) \
))

# vim: set noet sw=4 ts=4:
