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



package org.openoffice.setup.PanelHelper;

import org.openoffice.setup.SetupData.DisplayPackageDescription;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.ResourceManager;
import java.awt.Component;
import javax.swing.ImageIcon;
import javax.swing.JTree;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;

public class TreeNodeRenderer extends DefaultTreeCellRenderer {
    
    ImageIcon    InstallIcon;
    ImageIcon    InstalledIcon;
    ImageIcon    DontInstallIcon;
    ImageIcon    DontKnowIcon;
    ImageIcon    RemoveIcon;

    public TreeNodeRenderer() {
        InstallIcon     = ResourceManager.getIcon("Icon_Install");
        InstalledIcon   = ResourceManager.getIcon("Icon_Installed");
        DontInstallIcon = ResourceManager.getIcon("Icon_DontInstall");
        DontKnowIcon    = ResourceManager.getIcon("Icon_DontKnow");
        RemoveIcon      = ResourceManager.getIcon("Icon_Remove");
    }

    public Component getTreeCellRendererComponent(
                        JTree tree, Object value, boolean sel, boolean expanded,
                        boolean leaf, int row, boolean hasFocus)    {
        
        super.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);

        DefaultMutableTreeNode node = (DefaultMutableTreeNode) value;
        Object nodeObject = node.getUserObject();
        
        if (DisplayPackageDescription.is(nodeObject)) {
            DisplayPackageDescription nodeInfo = (DisplayPackageDescription)nodeObject;

            switch (nodeInfo.getState()) {
                case PackageDescription.INSTALL:      setIcon(InstallIcon);      break;
                case PackageDescription.DONT_REMOVE:  setIcon(InstallIcon);      break;
                case PackageDescription.IGNORE:       setIcon(InstalledIcon);    break;
                case PackageDescription.INSTALL_SOME: setIcon(DontKnowIcon);     break;
                case PackageDescription.REMOVE_SOME:  setIcon(DontKnowIcon);     break;
                case PackageDescription.DONT_INSTALL: setIcon(DontInstallIcon);  break;
                case PackageDescription.REMOVE:       setIcon(RemoveIcon);       break;
                default: setIcon(InstalledIcon); break;
            }
        }
 
        if (sel) {
            setBackground(super.getBackgroundSelectionColor());
            setForeground(textSelectionColor);
        } else {
            setBackground(super.getBackgroundNonSelectionColor());
            setForeground(textSelectionColor);
        }
        
        return this;
    }
}
