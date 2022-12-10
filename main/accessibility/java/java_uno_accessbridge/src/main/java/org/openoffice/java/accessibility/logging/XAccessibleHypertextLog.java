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



package org.openoffice.java.accessibility.logging;

import com.sun.star.accessibility.*;
import com.sun.star.uno.*;

/** The AccessibleHypertextImpl mapps all calls to the java AccessibleHypertext
 *  interface to the corresponding methods of the UNO XAccessibleHypertext 
 *  interface.
 */
public class XAccessibleHypertextLog extends XAccessibleTextLog
    implements com.sun.star.accessibility.XAccessibleHypertext {
    
    private com.sun.star.accessibility.XAccessibleHypertext unoObject;

    /** Creates a new instance of XAccessibleTextLog */
    public XAccessibleHypertextLog(XAccessibleHypertext xAccessibleHypertext) {
        super(xAccessibleHypertext);
        unoObject = xAccessibleHypertext;
    }

    public XAccessibleHyperlink getHyperLink(int param) 
            throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getHyperLink(param);
    }
    
    public int getHyperLinkCount() {
        return unoObject.getHyperLinkCount();
    }
    
    public int getHyperLinkIndex(int param)
            throws com.sun.star.lang.IndexOutOfBoundsException {
        return unoObject.getHyperLinkIndex(param);
    }
}
