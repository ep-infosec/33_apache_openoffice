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
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.SetupData.PackageDescription;
import java.util.Enumeration;
import java.util.Vector;

public class PackageCollector {
    
    private PackageCollector() {
    }

    static public void collectInstallPackages(PackageDescription packageData, Vector allPackages) {
            	
        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.INSTALL )) {
            boolean doAdd = true;
            // Special handling for jre package, because this is not necessarily older, if an older product is updated.
            if ( packageData.isJavaPackage() ) {
                Installer installer = InstallerFactory.getInstance();
                InstallData data = InstallData.getInstance();
                if ( installer.isPackageInstalled(packageData, data) ) {
                    if ( ! installer.isInstalledPackageOlder(packageData, data) ) {
                        doAdd = false;
                    }
                }
            }

            if ( doAdd ) {
                allPackages.add(packageData);
                System.err.println("Adding to collector 1: " + packageData.getPackageName());
            }
        }
        
        // also allowing packages at nodes! 
        if (( ! packageData.isLeaf() ) &&
                ( packageData.getPackageName() != null ) &&
                ( ! packageData.getPackageName().equals("")) &&
                (( packageData.getSelectionState() == packageData.INSTALL ) ||
                ( packageData.getSelectionState() == packageData.INSTALL_SOME ))) {
            boolean doAdd = true;
            // Special handling for jre package, because this is not necessarily older, if an older product is updated.
            if ( packageData.isJavaPackage() ) {
                Installer installer = InstallerFactory.getInstance();
                InstallData data = InstallData.getInstance();
                if ( installer.isPackageInstalled(packageData, data) ) {
                    if ( ! installer.isInstalledPackageOlder(packageData, data) ) {
                        doAdd = false;
                    }
                }
            }

            if ( doAdd ) {
                allPackages.add(packageData);
                // System.err.println("Adding to collector 2: " + packageData.getPackageName());
            }
        }
        
        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            collectInstallPackages(child, allPackages);
        }
        
    }

    static public void collectUninstallPackages(PackageDescription packageData, Vector allPackages) {
        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.REMOVE )) {
            allPackages.add(0, packageData);
        }
 
        // also allowing packages at nodes! 
        if (( ! packageData.isLeaf() ) &&
                ( packageData.getPackageName() != null ) &&
                ( ! packageData.getPackageName().equals("")) &&
                ( packageData.getSelectionState() == packageData.REMOVE )) {
            allPackages.add(0, packageData);
        }
                
        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            collectUninstallPackages(child, allPackages);
        }   
    }

    // Special handling for packages, that change their name, and therefore need to be uninstalled
    
    // static public void findOldPackages( InstallData installData ) {
    //
    //    String basis = "ooobasis3";
    //    if ( installData.getOSType().equalsIgnoreCase("Linux") ) { basis = basis + "."; }
    //    String search = basis + installData.getProductMinor();

    //    Vector allPackages = installData.getInstallPackages();
    //    Vector oldPackages = new Vector();

    //    for (int i = 0; i < allPackages.size(); i++) {
    //        PackageDescription packageData = (PackageDescription) allPackages.get(i);
    //        int pos = packageData.getPackageName().indexOf(search);

    //        if ( pos > -1 ) {
    //            String substring = packageData.getPackageName().substring(pos, pos + 1);
    //            for (int j = 0; j < installData.getProductMinor(); j++) {
    //                String replace = basis + j;
    //                // Creating new package for removal, very simple PackageDescription
    //                PackageDescription localPackage = new PackageDescription();
    //                localPackage.setUninstallCanFail(true);
    //                localPackage.setIsRelocatable(packageData.isRelocatable());
    //                String localName = packageData.getPackageName();
    //                localName = localName.replace(search, replace);
    //                localPackage.setPackageName(localName);

    //                if ( ( packageData.getPkgRealName() != null ) && ( ! packageData.getPkgRealName().equals("") )) {
    //                    localName = packageData.getPkgRealName();
    //                    localName = localName.replace(search, replace);
    //                    localPackage.setPkgRealName(localName);
    //                }

    //                if (( packageData.getName() != null ) && ( ! packageData.getName().equals("") )) {
    //                    localName = packageData.getName();
    //                    localName = localName.replace(search, replace);
    //                    localPackage.setName(localName);
    //                }
                    
    //                oldPackages.add(localPackage);
    //            }
    //        }
    //    }
        
    //    // reverse order for uninstallation
    //    int number = oldPackages.size();
    //    for (int i = 0; i < number; i++) {
    //        if ( i > 0 ) {
    //            PackageDescription oldPackageData = (PackageDescription) oldPackages.remove(i);
    //            oldPackages.add(0,oldPackageData);
    //        }
    //    }       

    //    installData.setOldPackages(oldPackages);
    // }

    static public void sortPackages(Vector allPackages, Vector sortedPackages, String mode) {
        for (int i = 0; i < allPackages.size(); i++) {
            boolean integrated = false;
            PackageDescription packageData = (PackageDescription) allPackages.get(i);
            
            if ( i == 0 ) {
                sortedPackages.add(packageData);
                integrated = true;
            } else {
                int position = packageData.getOrder();
                for (int j = 0; j < sortedPackages.size(); j++) {
                    PackageDescription sortedPackageData = (PackageDescription) sortedPackages.get(j);
                    int compare = sortedPackageData.getOrder();
                
                    if ( position < compare ) {
                        sortedPackages.add(j, packageData);
                        integrated = true;
                        break;
                    }
                }
                
                // no break used -> adding at the end
                if ( ! integrated ) {
                    sortedPackages.add(packageData);
                }
            }
        }
        
        // reverse order for uninstallation
        if ( mode.equalsIgnoreCase("uninstall")) {
            int number = sortedPackages.size();
            for (int i = 0; i < number; i++) {
                if ( i > 0 ) {
                    PackageDescription sortPackageData = (PackageDescription) sortedPackages.remove(i);
                    sortedPackages.add(0,sortPackageData);
                }
            }
        } 
    }

}
