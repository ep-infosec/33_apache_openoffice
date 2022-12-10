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



package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.lang.EventObject;
import com.sun.star.util.XModifyBroadcaster;
import com.sun.star.util.XModifyListener;

/**
* Testing <code>com.sun.star.util.XModifyBroadcaster</code>
* interface methods :
* <ul>
*  <li><code>addModifyListener()</code></li>
*  <li><code>removeModifyListener()</code></li>
* </ul> <p>
*
* Result checking is not performed. Modify
* listeners are called only in case of
* interaction with UI. <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.util.XModifyBroadcaster
*/
public class _XModifyBroadcaster extends MultiMethodTest {
    public XModifyBroadcaster oObj = null;

    boolean modified = false;

    protected class TestModifyListener implements XModifyListener {
        public void disposing ( EventObject oEvent ) {}
        public void modified (EventObject aEvent ) {
            modified = true;
        }
    }

    private TestModifyListener listener = new TestModifyListener();

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _addModifyListener() {
        log.println("'Modified' events are called only in case"+
            " of user interaction.");
        oObj.addModifyListener(listener);
        tRes.tested("addModifyListener()", true);
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    */
    public void _removeModifyListener() {
        requiredMethod("addModifyListener()");
        oObj.removeModifyListener(listener);
        tRes.tested("removeModifyListener()", true);
    }
}// finish class _XModifyBroadcaster

