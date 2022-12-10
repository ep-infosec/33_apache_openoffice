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



#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Intrinsic.h>

#include "jni.h"

// Workaround for problematic IBM JDK 1.6.0 on ppc
#ifndef _JNI_IMPORT_OR_EXPORT_
#define _JNI_IMPORT_OR_EXPORT_
#endif

#include "jawt_md.h"
#include "jawt.h"

/*#include "../inc/com_sun_star_comp_beans_LocalOfficeWindow.h"*/

#if defined assert
#undef assert
#endif

#define assert(X) if (!X) { (*env)->ThrowNew(env, (*env)->FindClass(env, "java/lang/RuntimeException"), "assertion failed"); return;}


#define SYSTEM_WIN32   1
#define SYSTEM_WIN16   2
#define SYSTEM_JAVA    3
#define SYSTEM_OS2     4
#define SYSTEM_MAC     5
#define SYSTEM_XWINDOW 6

JNIEXPORT jlong JNICALL Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this);

/*****************************************************************************/
/*
 * Class:     com_sun_star_comp_beans_LocalOfficeWindow
 * Method:    getNativeWindowSystemType
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindowSystemType
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* avoid warning about unused parameter */ 
    (void) obj_this; /* avoid warning about unused parameter */ 
    return (SYSTEM_XWINDOW);
}


/*****************************************************************************/
/*
 * Class:     com_sun_star_beans_LocalOfficeWindow
 * Method:    getNativeWindow
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_com_sun_star_beans_LocalOfficeWindow_getNativeWindow
  (JNIEnv * env, jobject obj_this)
{
    (void) env; /* avoid warning about unused parameter */ 
    (void) obj_this; /* avoid warning about unused parameter */ 
	return Java_com_sun_star_comp_beans_LocalOfficeWindow_getNativeWindow(env, obj_this);
}











