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



#ifndef _SV_NATIVEWIDGETS_H
#define _SV_NATIVEWIDGETS_H

// since 10.4 ... no metrics are returned, and we have to fix the values
#define BUTTON_WIDTH 16
#define BUTTON_HEIGHT 17

//standard height of the AHIG
//tabs
#define TAB_HEIGHT_NORMAL 20
#define TAB_HEIGHT_SMALL  17
#define TAB_HEIGHT_MINI   15 

#define TAB_TEXT_OFFSET     12
#define VCL_TAB_TEXT_OFFSET 2

//listboxes, comboboxes (they have the same dimensions)
#define COMBOBOX_HEIGHT_NORMAL 20
#define DROPDOWN_BUTTON_WIDTH 20

//text edit
#define TEXT_EDIT_HEIGHT_NORMAL 22

//spin box
#define SPIN_BUTTON_SPACE           2
#define SPIN_BUTTON_WIDTH           13
#define SPIN_UPPER_BUTTON_HEIGHT    11
#define SPIN_LOWER_BUTTON_HEIGHT    10
#define SPIN_TWO_BUTTONS_HEIGHT     21

// progress bar
#define INTRO_PROGRESS_HEIGHT 9

// for some controls, like spinbuttons + spinboxes, or listboxes
// we need it to adjust text position beside radio and check buttons

#define TEXT_SEPARATOR 3

// extra border for focus ring
#define FOCUS_RING_WIDTH 4

#define CLIP_FUZZ 1

#endif	// _SV_NATIVEWIDGETS_H
