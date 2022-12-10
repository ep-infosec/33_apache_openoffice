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



package org.openoffice.setup.Panel;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.ResourceManager;
import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Dimension;
import java.awt.Insets;
import java.io.File;
import javax.swing.JPanel;
import javax.swing.JEditorPane;
import javax.swing.JScrollPane;
import javax.swing.border.EmptyBorder;
public class AcceptLicense extends JPanel {

    public AcceptLicense() {

        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));
  
        String titletext = ResourceManager.getString("String_AcceptLicense1");
        PanelTitle titlebox = new PanelTitle(titletext);
        add(titlebox, BorderLayout.NORTH);

        JPanel contentpanel = new JPanel();
        contentpanel.setLayout(new java.awt.BorderLayout());
       
        String text1 = ResourceManager.getString("String_AcceptLicense2");
        PanelLabel label1 = new PanelLabel(text1);
        
        String text2 = ResourceManager.getString("String_AcceptLicense3");
        PanelLabel label2 = new PanelLabel(text2, true);

        JEditorPane editorPane = createEditorPane();
        JScrollPane editorScrollPane = new JScrollPane(editorPane);
        
        editorScrollPane.setPreferredSize(new Dimension(250, 145));
        editorScrollPane.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));
        
        contentpanel.add(label1, BorderLayout.NORTH);
        contentpanel.add(editorScrollPane, BorderLayout.CENTER);
        contentpanel.add(label2, BorderLayout.SOUTH);                
        
        add(contentpanel, BorderLayout.CENTER);
    }
    
    private JEditorPane createEditorPane() {
        JEditorPane editorPane = new JEditorPane();
        editorPane.setEditable(false);

        InstallData data = InstallData.getInstance();
        File htmlDirectory = data.getInfoRoot("html");
        String licenseFile = ResourceManager.getFileName("String_License_Filename");

        if ( htmlDirectory != null) {
            File htmlFile = new File(htmlDirectory, licenseFile);
            if (! htmlFile.exists()) {
                System.err.println("Couldn't find file: " + htmlFile.toString());
            }

            try {
                // System.err.println("URLPath: " + htmlFile.toURL());
                editorPane.setContentType("text/html;charset=utf-8");
                editorPane.setPage(htmlFile.toURL());
            } catch (Exception e) {
                e.printStackTrace();
                System.err.println("Attempted to read a bad URL");
            }
        } else {
            System.err.println("Did not find html directory");
        }

        return editorPane;
    }
}
