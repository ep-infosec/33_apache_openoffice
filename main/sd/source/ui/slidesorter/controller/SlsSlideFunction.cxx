/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"

#include "controller/SlsSlideFunction.hxx"

#include "SlideSorter.hxx"
#include "controller/SlideSorterController.hxx"
#include "view/SlideSorterView.hxx"
#include "model/SlideSorterModel.hxx"


namespace sd { namespace slidesorter { namespace controller {

TYPEINIT1(SlideFunction, FuPoor);


SlideFunction::SlideFunction (
    SlideSorter& rSlideSorter,
    SfxRequest& rRequest)
    : FuPoor (
        rSlideSorter.GetViewShell(),
        rSlideSorter.GetContentWindow().get(), 
        &rSlideSorter.GetView(),
        rSlideSorter.GetModel().GetDocument(),
        rRequest)
{
}

void SlideFunction::ScrollStart (void)
{
}

void SlideFunction::ScrollEnd (void)
{
}

sal_Bool SlideFunction::MouseMove(const MouseEvent& )
{
	return sal_False;
}

sal_Bool SlideFunction::MouseButtonUp(const MouseEvent& )
{
	return sal_False;

}

sal_Bool SlideFunction::MouseButtonDown(const MouseEvent& )
{
	return sal_False;
}

} } } // end of namespace ::sd::slidesorter::controller
