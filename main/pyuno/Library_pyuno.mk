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



$(eval $(call gb_Library_Library,pyuno))

$(eval $(call gb_Library_set_include,pyuno,\
	$$(INCLUDE) \
	-I$(SRCDIR)/pyuno/inc \
))

$(eval $(call gb_Library_add_api,pyuno, \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,pyuno,\
	cppu \
	cppuhelper \
	sal \
	stl \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_external,pyuno,python)

$(eval $(call gb_Library_add_exception_objects,pyuno,\
	pyuno/source/module/pyuno_runtime \
	pyuno/source/module/pyuno \
	pyuno/source/module/pyuno_callable \
	pyuno/source/module/pyuno_module \
	pyuno/source/module/pyuno_type \
	pyuno/source/module/pyuno_util \
	pyuno/source/module/pyuno_except \
	pyuno/source/module/pyuno_adapter \
	pyuno/source/module/pyuno_gc \
))

# vim: set noet sw=4 ts=4:

