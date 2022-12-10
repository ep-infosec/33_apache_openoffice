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



package org.openoffice.setup.SetupData;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Util.ModuleCtrl;
import java.util.Enumeration;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.TreeNode;

public class DisplayPackageDescription {

    private PackageDescription data;
    private int size  = 0;
    
    public DisplayPackageDescription(PackageDescription data) {
        this.data = data;
        // Setting default module settings for modules, that are not hidden
        ModuleCtrl.setDefaultModuleSettings(data);
        // The size is set dynamically, when a module is selected.
        // This is better for the nodes, which have a changing size.
    }

    public String getDescription() {
        return data.getDescription();
    }
    
    public int getSize() {
        return size;     // using local size
    }
    
    public int getState() {
        return data.getSelectionState();
    }
    
    private int getSize(TreeNode peerNode) {
        // return peerNode.isLeaf() ? data.getAccumulatedSize() : data.getSize();
        // using size from PackageDescription, which is pkgSize (and that is defined in xpd file)
        return data.getSize();
    }
    
    public void setState(TreeNode node, int newState) {
        if ((data.getSelectionState() != PackageDescription.IGNORE) && data.isOptional()) {
            data.setSelectionState(newState);
        }

        if (!node.isLeaf()) {
            size = data.getSize();  // -> that is the value defined in xpd file

            for (Enumeration e = node.children(); e.hasMoreElements();) {               
                TreeNode child = (TreeNode)e.nextElement();
                DisplayPackageDescription childInfo = getInfo(child);
                
                childInfo.setState(child, newState);
                
                if (childInfo.isSelected())   {
                    size += childInfo.getSize(child);
                }
            }
        } else if (isSelected()) {
            size = data.getSize();  // -> that is the value defined in xpd file
        }
    }

    public void toggleState(TreeNode node) {

        int state = data.getSelectionState();
        
        if (state != PackageDescription.IGNORE) {
            
            if (state == PackageDescription.REMOVE) {
                setState(node, PackageDescription.DONT_REMOVE);
            } else if ((state == PackageDescription.DONT_REMOVE) || (state == PackageDescription.REMOVE_SOME)) {
                setState(node, PackageDescription.REMOVE );
            } else if ((state == PackageDescription.DONT_INSTALL) || (state == PackageDescription.INSTALL_SOME)) {
                setState(node, PackageDescription.INSTALL);
            } else {
                setState(node, PackageDescription.DONT_INSTALL);
            }
            
            if (!node.isLeaf()) {
                updateState(node);
            }

            try {
                TreeNode parent = node.getParent();
                DisplayPackageDescription parentInfo = getInfo(parent);
                
                parentInfo.updateState(parent);
                try {
                    TreeNode grandpa = parent.getParent();
                    DisplayPackageDescription grandpaInfo = getInfo(grandpa);
                
                    grandpaInfo.updateState(grandpa);
                } catch (java.lang.IllegalArgumentException e) {
                    /* ignore */
                }           

            } catch (java.lang.IllegalArgumentException e) {
                /* ignore */
            }
        }
    }

    public void updateState(TreeNode node)
    {
        int state = PackageDescription.DONT_KNOW;
        InstallData installdata = InstallData.getInstance();
        size = data.getSize();  // -> that is the value defined in xpd file

        for (Enumeration e = node.children(); e.hasMoreElements();) {            
            TreeNode child = (TreeNode) e.nextElement();
            DisplayPackageDescription childInfo = getInfo(child);
            
            int childState = childInfo.getState();

            if ((state == PackageDescription.DONT_KNOW) || (state == PackageDescription.IGNORE)) {
                state = childState;
            } else if ((state != childState) && (childState != PackageDescription.IGNORE)) {
                if ( installdata.isUninstallationMode() ) {
                    state = PackageDescription.REMOVE_SOME;
                } else {
                    state = PackageDescription.INSTALL_SOME;
                }
            }
            if (childInfo.isSelected()) {
                size += childInfo.getSize(child);
            }
        }
        
        data.setSelectionState(state);

    }

    public void updateSize(TreeNode node)
    {
        size = data.getSize();  // -> that is the value defined in xpd file
        
        for (Enumeration e = node.children(); e.hasMoreElements();) {            
            TreeNode child = (TreeNode) e.nextElement();
            DisplayPackageDescription childInfo = getInfo(child);
            if (childInfo.isSelected()) {
                size += childInfo.getSize(child);
            }
        }
    }

    public String toString() {
        return data.getName();
    }
    
    static public boolean is(Object o) {
        return (o != null) 
            && (o.getClass().getName().equals("org.openoffice.setup.SetupData.DisplayPackageDescription"));
    }
    
    public boolean isSelected() {
        int state = data.getSelectionState();
        return     (state == PackageDescription.INSTALL) || (state == PackageDescription.INSTALL_SOME) 
                || (state == PackageDescription.REMOVE)  || (state == PackageDescription.REMOVE_SOME);
    }
    
    private DisplayPackageDescription getInfo(TreeNode node) throws java.lang.IllegalArgumentException {
        if (node == null) {
            throw new java.lang.IllegalArgumentException();
        }
        
        DisplayPackageDescription info = (DisplayPackageDescription)((DefaultMutableTreeNode)node).getUserObject();
        if ((info != null) && is(info)) {
            return info;
        } else {
            throw new java.lang.IllegalArgumentException();
        }
    }
}
