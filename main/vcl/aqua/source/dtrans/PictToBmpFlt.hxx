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

#ifndef INCLUDED_PICTTOBMPFLT_HXX
#define INCLUDED_PICTTOBMPFLT_HXX

#include <com/sun/star/uno/Sequence.hxx>

#include <premac.h>
#include <Cocoa/Cocoa.h>
#include <postmac.h>

/** Transform an image from PICT to PNG format

   Returns true if the conversion was successful false 
   otherwise.
 */
bool PICTtoPNG(com::sun::star::uno::Sequence<sal_Int8>& rPictData, 
			   com::sun::star::uno::Sequence<sal_Int8>& rPngData);

/** Transform an image from PNG to a PICT format

   Returns true if the conversion was successful false
   otherwise.
 */
bool PNGtoPICT(com::sun::star::uno::Sequence<sal_Int8>& rPngData,
			   com::sun::star::uno::Sequence<sal_Int8>& rPictData);

#define PICTImageFileType ((NSBitmapImageFileType)~0)

bool ImageToPNG( com::sun::star::uno::Sequence<sal_Int8>& rImgData,
			     com::sun::star::uno::Sequence<sal_Int8>& rPngData,
			     NSBitmapImageFileType eInFormat);

bool PNGToImage( com::sun::star::uno::Sequence<sal_Int8>& rPngData,
			     com::sun::star::uno::Sequence<sal_Int8>& rImgData,
			     NSBitmapImageFileType eOutFormat);

#endif

