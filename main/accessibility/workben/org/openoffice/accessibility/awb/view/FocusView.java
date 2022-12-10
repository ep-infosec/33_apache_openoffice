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



package org.openoffice.accessibility.awb.view;

import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;

import javax.swing.JButton;
import javax.swing.JLabel;

import com.sun.star.accessibility.AccessibleEventId;
import com.sun.star.accessibility.AccessibleEventObject;
import com.sun.star.accessibility.AccessibleStateType;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleStateSet;
import com.sun.star.uno.UnoRuntime;

public class FocusView
    extends ObjectView
    implements ActionListener
{
    /** Create a FocusView when the given object supports the
        XAccessibleComponent interface.
    */
    static public ObjectView Create (
        ObjectViewContainer aContainer,
        XAccessibleContext xContext)
    {
        XAccessibleComponent xComponent = (XAccessibleComponent)UnoRuntime.queryInterface(
                XAccessibleComponent.class, xContext);
        if (xComponent != null)
            return new FocusView (aContainer);
        else
            return null;
    }

    public FocusView (ObjectViewContainer aContainer)
    {
        super (aContainer);

        setLayout (new GridBagLayout());
        GridBagConstraints aConstraints = new GridBagConstraints ();

        maFocused = new JLabel ();
        maFocused.setFont (GetContainer().GetViewFont());
        aConstraints.gridy = 0;
        aConstraints.weightx = 1;
        aConstraints.fill = GridBagConstraints.HORIZONTAL;
        add (maFocused, aConstraints);

        maGrabFocus = new JButton ("grabFocus");
        maGrabFocus.setFont (GetContainer().GetViewFont());
        aConstraints.gridy = 1;
        aConstraints.fill = GridBagConstraints.NONE;
        aConstraints.anchor = GridBagConstraints.WEST;
        add (maGrabFocus, aConstraints);

        maGrabFocus.addActionListener (this);
    }

    /** Additionally to the context store a reference to the
        XAccessibleComponent interface.
    */
    public void SetObject (XAccessibleContext xObject)
    {
        mxComponent = (XAccessibleComponent)UnoRuntime.queryInterface(
                XAccessibleComponent.class, xObject);
        super.SetObject (xObject);
    }

    synchronized public void Destroy ()
    {
        super.Destroy();
        maGrabFocus.removeActionListener (this);
    }

    synchronized public void Update ()
    {
        if (mxContext == null)
        {
            maFocused.setText ("<null object>");
            maGrabFocus.setEnabled (false);
        }
        else
        {
            XAccessibleStateSet aStateSet = mxContext.getAccessibleStateSet();
            if (aStateSet.contains(AccessibleStateType.FOCUSED))
                maFocused.setText ("focused");
            else
                maFocused.setText ("not focused");
            if (maGrabFocus != null)
                maGrabFocus.setEnabled (true);
        }
    }

    public String GetTitle ()
    {
        return ("Focus");
    }

    synchronized public void actionPerformed (ActionEvent aEvent)
    {
        if (aEvent.getActionCommand().equals("grabFocus"))
        {
            mxComponent.grabFocus();
        }
    }

    public void notifyEvent (AccessibleEventObject aEvent)
    {
        System.out.println (aEvent);
        if (aEvent.EventId == AccessibleEventId.STATE_CHANGED)
            Update ();
    }

    private JLabel maFocused;
    private JButton maGrabFocus;
    private XAccessibleComponent mxComponent;
}
