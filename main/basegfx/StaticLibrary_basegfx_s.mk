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



$(eval $(call gb_StaticLibrary_StaticLibrary,basegfx_s))

$(eval $(call gb_StaticLibrary_add_package_headers,basegfx_s,basegfx_inc))

$(eval $(call gb_StaticLibrary_add_precompiled_header,basegfx_s,$(SRCDIR)/basegfx/inc/pch/precompiled_basegfx))

$(eval $(call gb_StaticLibrary_add_api,basegfx_s,\
	udkapi \
	offapi \
))

$(eval $(call gb_StaticLibrary_set_include,basegfx_s,\
	-I$(SRCDIR)/basegfx/inc \
	-I$(SRCDIR)/basegfx/source/inc \
	$$(INCLUDE) \
	-I$(SRCDIR)/basegfx/inc/pch \
))

# the whole library is a hack
# we only build it to prevent the pdfimporter shared lib from linking to the basegfx shared lib
# this means that all code of this static library will end in a dll
# the BASEGFX_STATICLIBRARY define will expand all BASEGFX_DLLPRIVATE/PUBLIC macros to nothing
$(eval $(call gb_StaticLibrary_add_defs,basegfx_s,\
	-DBASEGFX_STATICLIBRARY \
))

# as we have to build all sources a second time, we must copy them to the workdir and build from there
$(WORKDIR)/CustomTarget/basegfx/source/%.cxx : $(SRCDIR)/basegfx/source/%.cxx
	mkdir -p $(dir $@) && \
	cp $< $@

# copied sources are generated cxx sources
$(eval $(call gb_StaticLibrary_add_generated_exception_objects,basegfx_s,\
	CustomTarget/basegfx/source/tools/liangbarsky \
	CustomTarget/basegfx/source/tools/debugplotter \
	CustomTarget/basegfx/source/tools/canvastools \
	CustomTarget/basegfx/source/tools/gradienttools \
	CustomTarget/basegfx/source/tools/keystoplerp \
	CustomTarget/basegfx/source/tools/unopolypolygon \
	CustomTarget/basegfx/source/tools/tools \
	CustomTarget/basegfx/source/tools/b2dclipstate \
	CustomTarget/basegfx/source/tools/stringconversiontools \
	CustomTarget/basegfx/source/numeric/ftools \
	CustomTarget/basegfx/source/tuple/b3ituple \
	CustomTarget/basegfx/source/tuple/b3dtuple \
	CustomTarget/basegfx/source/tuple/b2ituple \
	CustomTarget/basegfx/source/tuple/b2dtuple \
	CustomTarget/basegfx/source/tuple/b3i64tuple \
	CustomTarget/basegfx/source/tuple/b2i64tuple \
	CustomTarget/basegfx/source/range/b1drange \
	CustomTarget/basegfx/source/range/b2dpolyrange \
	CustomTarget/basegfx/source/range/b2xrange \
	CustomTarget/basegfx/source/range/b3drange \
	CustomTarget/basegfx/source/range/b2drangeclipper \
	CustomTarget/basegfx/source/range/b2drange \
	CustomTarget/basegfx/source/vector/b3ivector \
	CustomTarget/basegfx/source/vector/b2ivector \
	CustomTarget/basegfx/source/vector/b2dvector \
	CustomTarget/basegfx/source/vector/b3dvector \
	CustomTarget/basegfx/source/polygon/b3dpolygon \
	CustomTarget/basegfx/source/polygon/b2dtrapezoid \
	CustomTarget/basegfx/source/polygon/b3dpolygontools \
	CustomTarget/basegfx/source/polygon/b2dlinegeometry \
	CustomTarget/basegfx/source/polygon/b3dpolypolygontools \
	CustomTarget/basegfx/source/polygon/b3dpolygonclipper \
	CustomTarget/basegfx/source/polygon/b2dpolygonclipper \
	CustomTarget/basegfx/source/polygon/b2dpolypolygoncutter \
	CustomTarget/basegfx/source/polygon/b2dpolypolygonrasterconverter \
	CustomTarget/basegfx/source/polygon/b2dpolygon \
	CustomTarget/basegfx/source/polygon/b2dpolygontriangulator \
	CustomTarget/basegfx/source/polygon/b2dpolypolygon \
	CustomTarget/basegfx/source/polygon/b3dpolypolygon \
	CustomTarget/basegfx/source/polygon/b2dsvgpolypolygon \
	CustomTarget/basegfx/source/polygon/b2dpolygoncutandtouch \
	CustomTarget/basegfx/source/polygon/b2dpolypolygontools \
	CustomTarget/basegfx/source/polygon/b2dpolygontools \
	CustomTarget/basegfx/source/pixel/bpixel \
	CustomTarget/basegfx/source/matrix/b2dhommatrixtools \
	CustomTarget/basegfx/source/matrix/b3dhommatrix \
	CustomTarget/basegfx/source/matrix/b2dhommatrix \
	CustomTarget/basegfx/source/point/b3ipoint \
	CustomTarget/basegfx/source/point/b3dhompoint \
	CustomTarget/basegfx/source/point/b2dpoint \
	CustomTarget/basegfx/source/point/b3dpoint \
	CustomTarget/basegfx/source/point/b2dhompoint \
	CustomTarget/basegfx/source/point/b2ipoint \
	CustomTarget/basegfx/source/curve/b2dbeziertools \
	CustomTarget/basegfx/source/curve/b2dcubicbezier \
	CustomTarget/basegfx/source/curve/b2dquadraticbezier \
	CustomTarget/basegfx/source/color/bcolormodifier \
	CustomTarget/basegfx/source/color/bcolor \
	CustomTarget/basegfx/source/color/bcolortools \
	CustomTarget/basegfx/source/raster/bpixelraster \
	CustomTarget/basegfx/source/raster/bzpixelraster \
	CustomTarget/basegfx/source/raster/rasterconvert3d \
))

# vim: set noet sw=4 ts=4:
