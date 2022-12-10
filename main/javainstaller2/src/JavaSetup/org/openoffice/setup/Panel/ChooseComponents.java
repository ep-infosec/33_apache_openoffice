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

import org.openoffice.setup.PanelHelper.PanelLabel;
import org.openoffice.setup.PanelHelper.PanelTitle;
import org.openoffice.setup.PanelHelper.TreeNodeRenderer;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.DisplayPackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import java.awt.BorderLayout;
import java.awt.ComponentOrientation;
import java.awt.Insets;
import java.awt.event.KeyEvent;
import java.awt.event.KeyListener;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;
import javax.swing.BorderFactory;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTree;
import javax.swing.border.EmptyBorder;
import javax.swing.border.TitledBorder;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
import org.openoffice.setup.InstallData;

public class ChooseComponents extends JPanel implements MouseListener, KeyListener, TreeSelectionListener {

    private JTree componentTree;
    private PanelLabel descriptionLabel;    
    private PanelLabel sizeLabel;
    
    private String sizeString;
    private PanelTitle titleBox;
    
    public ChooseComponents() {

        InstallData data = InstallData.getInstance();

        setLayout(new BorderLayout());
        setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));
              
        String titleText    = ResourceManager.getString("String_ChooseComponents1");
        String subtitleText = ResourceManager.getString("String_ChooseComponents2");
        titleBox = new PanelTitle(titleText, subtitleText, 2, 40);
        titleBox.addVerticalStrut(20);
        add(titleBox, BorderLayout.NORTH);

        DefaultMutableTreeNode root = SetupDataProvider.createTree();

        componentTree = new JTree(root);
        componentTree.setShowsRootHandles(true);
        componentTree.setRootVisible(false);
        componentTree.setVisibleRowCount(3);
        componentTree.setCellRenderer(new TreeNodeRenderer());
        componentTree.addMouseListener( this );
        componentTree.addKeyListener( this );
        componentTree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        componentTree.addTreeSelectionListener(this);
        // if ( data.useRtl() ) { componentTree.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
       
        String BorderTitle = ResourceManager.getString("String_ChooseComponents3");
        TitledBorder PanelBorder = BorderFactory.createTitledBorder(BorderTitle);
        
        BorderLayout PanelLayout = new BorderLayout();
        PanelLayout.setHgap(20);
        JPanel DescriptionPanel = new JPanel();
        DescriptionPanel.setBorder(PanelBorder);
        DescriptionPanel.setLayout(PanelLayout);
        
        String DescriptionText = "";
        descriptionLabel = new PanelLabel(DescriptionText, 3, 20);        
        sizeString = ResourceManager.getString("String_ChooseComponents4");
        sizeLabel = new PanelLabel(sizeString, 1, 5);
        
        DescriptionPanel.add(descriptionLabel, BorderLayout.CENTER);
        DescriptionPanel.add(sizeLabel, BorderLayout.EAST);
        if ( data.useRtl() ) { DescriptionPanel.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT); }
        
        add(new JScrollPane(componentTree), BorderLayout.CENTER);
        add(DescriptionPanel, BorderLayout.SOUTH);
    }

    public void setTitleText(String s) {
        titleBox.setTitle(s);
    }

    private void updateNode(DefaultMutableTreeNode node) {
        if (node != null) {
            DisplayPackageDescription nodeInfo = (DisplayPackageDescription)node.getUserObject();
            nodeInfo.toggleState(node);

            DefaultTreeModel model = (DefaultTreeModel)componentTree.getModel();
            // model.nodeChanged(node);
            
            // The following line was included because of task i78481. 
            // In Java 1.6 nodeChanged does not work correctly.
            model.nodeStructureChanged(node);
            
            descriptionLabel.setText(nodeInfo.getDescription());
            sizeLabel.setText(sizeString + nodeInfo.getSize());
        }
    }
 
    /**
     * Implement the MouseListener Interface
     */
    public void mouseClicked(MouseEvent event)  {
    }
    public void mouseEntered(MouseEvent event)  {
    }
    public void mouseExited(MouseEvent event)   {
    }
    public void mousePressed(MouseEvent event)  {
        TreePath selPath = componentTree.getPathForLocation( event.getX(), event.getY() );
        if ((selPath != null) && (componentTree.getPathBounds(selPath).getX() + 20 >= event.getX())) {
            updateNode((DefaultMutableTreeNode)selPath.getLastPathComponent());
        }
    } 
    public void mouseReleased(MouseEvent e) {
    }
    
    /**
     * Implement the KeyListener Interface
     */
    public void keyPressed(KeyEvent event)  {   
    }
    public void keyReleased(KeyEvent event) {
    }
    public void keyTyped(KeyEvent event)    {
        if ( event.getKeyChar() == ' ' ) {
            TreePath selPath = componentTree.getAnchorSelectionPath();
            if ( selPath != null ) {
                updateNode((DefaultMutableTreeNode)selPath.getLastPathComponent());
            }
        }
    }
    
    /** 
     * Implement the TreeSelectionListener Interface. 
     */
    public void valueChanged(TreeSelectionEvent event) {
        DefaultMutableTreeNode node = (DefaultMutableTreeNode)componentTree.getLastSelectedPathComponent();
        if (node == null) {
            descriptionLabel.setText("");
            sizeLabel.setText("");
        } else {
            DisplayPackageDescription nodeInfo = (DisplayPackageDescription)node.getUserObject();

            nodeInfo.updateSize(node); // important to set default values for nodes
            DefaultTreeModel model = (DefaultTreeModel)componentTree.getModel();
            model.nodeChanged(node);

            descriptionLabel.setText(nodeInfo.getDescription());
            sizeLabel.setText(sizeString + nodeInfo.getSize());
        }
    }

}
