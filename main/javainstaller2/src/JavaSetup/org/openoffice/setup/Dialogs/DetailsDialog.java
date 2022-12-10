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



package org.openoffice.setup.Dialogs;

import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupFrame;
import org.openoffice.setup.Util.DialogFocusTraversalPolicy;
import java.awt.BorderLayout;
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.ActionListener;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JEditorPane;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JSeparator;
import javax.swing.JViewport;
import javax.swing.ScrollPaneConstants;
import javax.swing.border.EmptyBorder;
import org.openoffice.setup.InstallData;

public class DetailsDialog extends JDialog implements ActionListener {

    private JButton okButton;
    private String helpFileName;
    private String helpFileString;
    
    public DetailsDialog(SetupFrame setupFrame) {

        super(setupFrame.getDialog());

        InstallData data = InstallData.getInstance();

        String dialogTitle = ResourceManager.getString("String_InstallationCompleted_Button");
        String dialogText = setupFrame.getCurrentPanel().getDialogText();
                
        setTitle(dialogTitle);
        this.getContentPane().setLayout(new java.awt.BorderLayout());
        
        JPanel toppanel = new JPanel();
        toppanel.setLayout(new java.awt.BorderLayout());
        toppanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        
        JPanel buttonpanel = new JPanel();
        buttonpanel.setLayout(new java.awt.FlowLayout());
        buttonpanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        if ( data.useRtl() ) { buttonpanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        //Create an editor pane.
        JEditorPane editorPane = createEditorPane(dialogText);
        editorPane.setCaretPosition(0);
        if ( data.useRtl() ) { editorPane.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        JScrollPane editorScrollPane = new JScrollPane(editorPane,
                ScrollPaneConstants.VERTICAL_SCROLLBAR_AS_NEEDED,
                ScrollPaneConstants.HORIZONTAL_SCROLLBAR_AS_NEEDED);
        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        if ( data.useRtl() ) { editorScrollPane.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }

        JViewport port = editorScrollPane.getViewport();
        port.getVisibleRect().setLocation(0,0);
        if ( data.useRtl() ) { port.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        editorScrollPane.setViewport(port);

        // String helpTitle1 = ResourceManager.getString("String_Details_Title_1");
        // PanelLabel label1 = new PanelLabel(helpTitle1, true);
        // String helpTitle2 = ResourceManager.getString("String_Details_Title_2");
        // PanelLabel label2 = new PanelLabel(helpTitle2);

        String okString = ResourceManager.getString("String_OK");
        okButton = new JButton(okString);
        okButton.setEnabled(true);
        okButton.addActionListener(this);

        JSeparator separator = new JSeparator();
        
        // toppanel.add(label1, BorderLayout.NORTH);
        // toppanel.add(label2, BorderLayout.CENTER);  
        buttonpanel.add(okButton);  

        this.getContentPane().add(toppanel, BorderLayout.NORTH);
        this.getContentPane().add(editorScrollPane, BorderLayout.CENTER);
        this.getContentPane().add(buttonpanel, BorderLayout.SOUTH);
        
        // JScrollBar ScrollBar = editorScrollPane.getVerticalScrollBar();
        // if ( ScrollBar.isShowing() ) {
        //     editorPane.setFocusable(false);
        // } else {
        //     editorPane.setFocusable(true);            
        // }
        
        // Setting tab-order and focus on okButton 
        DialogFocusTraversalPolicy policy = new DialogFocusTraversalPolicy(new JComponent[] {okButton, editorScrollPane});
        this.setFocusTraversalPolicy(policy);  // set policy
        this.setFocusCycleRoot(true); // enable policy
    }
 
     private JEditorPane createEditorPane(String dialogText) {
        JEditorPane editorPane = new JEditorPane();
        editorPane.setEditable(false);
        editorPane.setContentType("text/html");
        editorPane.setText(dialogText);        
        return editorPane;
    }     
     
    public void actionPerformed (java.awt.event.ActionEvent evt) {
        setVisible(false);
        dispose(); 
    }

}
