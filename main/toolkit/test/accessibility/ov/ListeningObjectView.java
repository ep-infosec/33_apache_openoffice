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

package ov;

import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleEventBroadcaster;
import com.sun.star.accessibility.XAccessibleEventListener;
import com.sun.star.lang.EventObject;
import com.sun.star.uno.UnoRuntime;

/** Base class for object views that regsiters as accessibility event
    listener.
*/
abstract class ListeningObjectView
    extends ObjectView
    implements XAccessibleEventListener
{
    public ListeningObjectView (ObjectViewContainer aContainer)
    {
        super (aContainer);
    }

    /** Add this object as event listener at the broadcasting
        accessible object.
    */
    public void SetObject (XAccessibleContext xContext)
    {
        super.SetObject (xContext);
        XAccessibleEventBroadcaster xBroadcaster = 
            (XAccessibleEventBroadcaster)UnoRuntime.queryInterface(
                XAccessibleEventBroadcaster.class, xContext);
        if (xBroadcaster != null)
            xBroadcaster.addEventListener (this);
    }


    /** Remove this object as event listener from the broadcasting
        accessible object.
    */
    public void Destroy ()
    {
        super.Destroy ();
        XAccessibleEventBroadcaster xBroadcaster = 
            (XAccessibleEventBroadcaster)UnoRuntime.queryInterface(
                XAccessibleEventBroadcaster.class, mxContext);
        if (xBroadcaster != null)
            xBroadcaster.removeEventListener (this);
    }

    /** Derived classes have to implement this method to handle incoming
        events.
    */
    abstract public void notifyEvent (AccessibleEventObject aEvent);

    /** The disposing event is ignored per default.  If a derived class is
        interested it can overwrite this method.
    */
    public void disposing (EventObject aEvent)
    {
    }
}
