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
import java.awt.FlowLayout;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.io.File;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JTextField;
import javax.swing.border.EmptyBorder;

public class ChooseDirectory extends JPanel implements ActionListener {
    
    private JFileChooser directoryChooser;
    private JFileChooser directoryChooserRootdir;
    private JButton      directoryButton;
    private JButton      directoryButtonRootdir;
    private JTextField   directoryField;
    private JTextField   directoryFieldRootdir;
    private PanelLabel   databaseProgress;    
    private PanelTitle   titleBox;
    
    public ChooseDirectory() {
        
        setLayout(new java.awt.BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));
        
        String titleText    = ResourceManager.getString("String_ChooseDirectory1");
        String subtitleText = ResourceManager.getString("String_ChooseDirectory2");

        titleBox = new PanelTitle(titleText, subtitleText);
        titleBox.addVerticalStrut(10);
        add(titleBox, BorderLayout.NORTH);
        
        Box contentBox   = new Box(BoxLayout.Y_AXIS);
        
        JPanel contentPanel = new JPanel();
        contentPanel.setLayout(new GridBagLayout());
        
        directoryChooser = new JFileChooser();
        directoryChooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        
        String browseText = ResourceManager.getString("String_ChooseDirectory3");
        directoryButton = new JButton(browseText);
        directoryButton.addActionListener(this);
        
        directoryField = new JTextField();
        
        GridBagConstraints constraints = new GridBagConstraints();
        
        constraints.gridx = 0;
        constraints.gridy = 0;
        constraints.weightx = 1;
        constraints.weighty = 0;
        constraints.fill = GridBagConstraints.HORIZONTAL;
        
        contentPanel.add(directoryField, constraints);
        
        constraints.gridx = 1;
        constraints.gridy = 0;
        constraints.weightx = 0;
        constraints.weighty = 0;
        constraints.fill = GridBagConstraints.HORIZONTAL;
        
        contentPanel.add(directoryButton, constraints);
        
        constraints.gridx = 0;
        constraints.gridy = 1;
        constraints.weightx = 0;
        constraints.weighty = 1;
        constraints.fill = GridBagConstraints.VERTICAL;
        
        contentPanel.add(new JPanel(), constraints);
        
        contentBox.add(contentPanel);
        
        // defining a place for text output
        databaseProgress = new PanelLabel(""); // planned for database progress
        contentBox.add(databaseProgress);
        
        add(contentBox, BorderLayout.CENTER);
    }

    public void setDatabaseText(String s) {
        databaseProgress.setText(s);
    }

    public void setTitleText(String s) {
        titleBox.setTitle(s);
    }
    
    public void setDirectory(String dir) {
        directoryField.setText(dir);
    }

    public void disableDirectoryField() {
        directoryField.setEditable(false) ;
    }

    public void disableBrowseButton() {
        directoryButton.setEnabled(false);
    }

    public void enableDirectoryField() {
        directoryField.setEditable(true) ;
    }

    public String getDirectory() {
        return directoryField.getText();
    }
    
    public void setRootDirectory(String dir) {
        directoryFieldRootdir.setText(dir);
    }
    
    public String getRootDirectory() {
        return directoryFieldRootdir.getText();
    }
    
    public void actionPerformed(ActionEvent e) {
        
        //Handle open button action.
        if (e.getSource() == directoryButton) {
            int ReturnValue = directoryChooser.showOpenDialog(ChooseDirectory.this);
            
            if (ReturnValue == JFileChooser.APPROVE_OPTION) {
                File file = directoryChooser.getSelectedFile();
                directoryField.setText(file.getAbsolutePath());
            } else {
                // do nothing for now
            }
        }
        
        if (e.getSource() == directoryButtonRootdir) {
            int ReturnValue = directoryChooserRootdir.showOpenDialog(ChooseDirectory.this);
            
            if (ReturnValue == JFileChooser.APPROVE_OPTION) {
                File file = directoryChooserRootdir.getSelectedFile();
                directoryFieldRootdir.setText(file.getAbsolutePath());
            } else {
                // do nothing for now
            }
        }
        
    }
    
}
