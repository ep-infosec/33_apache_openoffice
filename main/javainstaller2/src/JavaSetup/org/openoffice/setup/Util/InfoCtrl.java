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



package org.openoffice.setup.Util;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.ProductDescription;
import java.util.Enumeration;
import java.util.Vector;

public class InfoCtrl {
    
    private InfoCtrl() {
    }

    static public String setHtmlFrame(String position, String htmlInfoText) {
        if ( position.equals("header") ) {
            htmlInfoText = "<HTML><BODY><FONT FACE=\"sans-serif\" SIZE=3>";
        }
        else if ( position.equals("end")) {
            htmlInfoText = htmlInfoText + "</FONT></BODY></HTML>";
        }
        
        return htmlInfoText;
    }

    static public String setReadyToInstallInfoText(ProductDescription productData, String htmlInfoText) {
        // String oneline = "-------------------------------------------------------------------";
        String oneline = "-------------------------------------------------------------";
        htmlInfoText = htmlInfoText + "<b>Product</b>: " + productData.get("product_fullname") + "<br>";
        InstallData data = InstallData.getInstance();
        htmlInfoText = htmlInfoText + "<b>Location</b>: " + data.getInstallDefaultDir() + "<br>";
        htmlInfoText = htmlInfoText + oneline + "<br>";

        return htmlInfoText;
    }    
    
    static public String setReadyToInstallInfoText(PackageDescription packageData, String htmlInfoText) {
        // setHtmlInfoText(packageData, 0);
        InstallData data = InstallData.getInstance();
        if ( data.isInstallationMode() ) {            
            htmlInfoText = setReadyToInstallInfoText(packageData, "", htmlInfoText);
        } else {
            htmlInfoText = setReadyToUninstallInfoText(packageData, "", htmlInfoText);
        }
        return htmlInfoText;
    }

    // private void setReadyToInstallInfoText(PackageDescription packageData, Integer indent) {
    static private String setReadyToInstallInfoText(PackageDescription packageData, String indent, String htmlInfoText) {
        // String spacer = "<spacer type=horizontal size=" + indent.toString() + ">";
        // System.out.println(ind);
        if (( packageData.isLeaf() ) || ( packageData.isAllChildrenHidden() )) {
            if ( ! packageData.isHidden() ) {
                if ( packageData.getSelectionState() == packageData.INSTALL ) {
                    // htmlInfoText = htmlInfoText + spacer + packageData.getName() + "<br>";
                    htmlInfoText = htmlInfoText + indent + packageData.getName() + "<br>";
                }
            }
        }

        if (( ! packageData.isLeaf() ) && ( ! packageData.isAllChildrenHidden() )) {
            if ( ! packageData.isHidden() ) {
                if (( packageData.getSelectionState() == packageData.INSTALL ) ||
                    ( packageData.getSelectionState() == packageData.INSTALL_SOME )) {
                    // htmlInfoText = htmlInfoText + spacer + "<b>" + packageData.getName() + "</b>" + "<br>";
                    // htmlInfoText = htmlInfoText + indent + "<b>" + packageData.getName() + "</b>" + "<br>";
                    htmlInfoText = htmlInfoText + indent + packageData.getName() + "<br>";
                }
            }

            indent = indent + "..";

            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                htmlInfoText = setReadyToInstallInfoText(child, indent, htmlInfoText);
            }
        }
        
        return htmlInfoText;
    }
    
    // private void setReadyToUninstallInfoText(PackageDescription packageData, Integer indent, String htmlInfoText) {
    static private String setReadyToUninstallInfoText(PackageDescription packageData, String indent, String htmlInfoText) {
        // String spacer = "<spacer type=horizontal size=" + indent.toString() + ">";
        // System.out.println(ind);
        if (( packageData.isLeaf() ) || ( packageData.isAllChildrenHidden() )) {
            if ( ! packageData.isHidden() ) {
                if ( packageData.getSelectionState() == packageData.REMOVE ) {
                    // htmlInfoText = htmlInfoText + spacer + packageData.getName() + "<br>";
                    htmlInfoText = htmlInfoText + indent + packageData.getName() + "<br>";
                }
            }
        }

        if (( ! packageData.isLeaf() ) && ( ! packageData.isAllChildrenHidden() )) {
            if ( ! packageData.isHidden() ) {
                if (( packageData.getSelectionState() == packageData.REMOVE ) ||
                    ( packageData.getSelectionState() == packageData.REMOVE_SOME )) {
                    // htmlInfoText = htmlInfoText + spacer + "<b>" + packageData.getName() + "</b>" + "<br>";
                    // htmlInfoText = htmlInfoText + indent + "<b>" + packageData.getName() + "</b>" + "<br>";
                    htmlInfoText = htmlInfoText + indent + packageData.getName() + "<br>";
                }
            }

            indent = indent + "..";

            for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
                PackageDescription child = (PackageDescription) e.nextElement();
                htmlInfoText = setReadyToUninstallInfoText(child, indent, htmlInfoText);
            }
        }

        return htmlInfoText;
    }

    static public String  setInstallLogInfoText(ProductDescription productData, String htmlInfoText) {
        // String separatorline = "-------------------------------------------------------------------";
        String separatorline = "-------------------------------------------------------------";
        htmlInfoText = htmlInfoText + "<b>Product</b>: " + productData.get("product_fullname") + "<br>";
        InstallData data = InstallData.getInstance();
        htmlInfoText = htmlInfoText + "<b>Location</b>: " + data.getInstallDir() + "<br>";
        htmlInfoText = htmlInfoText + "<b>Operating system</b>: " + data.getOSType() + "<br>";
        if ( data.isUserInstallation() ) {
            htmlInfoText = htmlInfoText + "<b>Installation type</b>: " + "User installation" + "<br>";
        } else {
            htmlInfoText = htmlInfoText + "<b>Installation type</b>: " + "Root installation" + "<br>";
        }

        htmlInfoText = htmlInfoText + separatorline + "<br>";
        
        htmlInfoText = LogManager.publishLogfileContent(htmlInfoText, separatorline);
        htmlInfoText = LogManager.publishCommandsLogfileContent(htmlInfoText);

        return htmlInfoText;
    }

}
