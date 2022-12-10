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
import java.util.Enumeration;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;
import java.util.Vector;


public class Dumper {
    
    private Dumper() {
    }

    static public void dumpPackageSettings(PackageDescription packageData) {
 
        if ( packageData.isLeaf() ) {
            System.out.println("Name: " + packageData.getName() +
                               " State: " + packageData.getSelectionState()  +
                               " " + packageData.getPackageName());
        } else {
            System.out.println("Nod-Name: " + packageData.getName() +
                               " State: " + packageData.getSelectionState());
        }
        
        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpPackageSettings(child);
        }      

    }

    static public void dumpModuleStates(PackageDescription packageData) {

        System.err.println("Name: " + packageData.getName() + 
                           " State: " + packageData.getSelectionState() +
                           " " + packageData.getPackageName());

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpModuleStates(child);
        }        
    }
    
    static private String getStateString(int state) {
        String stateString = null;
        
        if ( state == 0 ) {
            stateString = "DONT_INSTALL";
        } else if ( state == 1 ) {
            stateString = "INSTALL";
        } else if ( state == 2 ) {
            stateString = "INSTALL_SOME";
        } else if ( state == 3 ) {
            stateString = "REMOVE";
        } else if ( state == 4 ) {
            stateString = "DONT_REMOVE";
        } else if ( state == 5 ) {
            stateString = "REMOVE_SOME";
        } else if ( state == 6 ) {
            stateString = "IGNORE";
        } else if ( state == 7 ) {
            stateString = "DONT_KNOW";
        } else {
            stateString = null;
        }

        return stateString;
    }

    static private void logModuleStatesHelper(PackageDescription packageData) {
        int state = packageData.getSelectionState();
        String stateStr = getStateString(state);
        
        LogManager.addModulesLogfileComment("Name: " + packageData.getName() + 
                           " State: " + stateStr +
                           " " + packageData.getPackageName());

        // System.err.println("Name: " + packageData.getName() + 
        //                    " State: " + stateStr +
        //                    " " + packageData.getPackageName());
        
        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            logModuleStatesHelper(child);
        }        
    }

    static public void logModuleStates(PackageDescription packageData, String text) {
        LogManager.setModulesLogFileHeaderLine(text);
        logModuleStatesHelper(packageData);
    }
    
    static public void logPackagesToInstall(Vector packages, String text) {
        PackageDescription packageData = null;
        LogManager.setModulesLogFileHeaderLine(text);
        for (int i = 0; i < packages.size(); i++) {
            packageData = (PackageDescription)packages.get(i);
            LogManager.addModulesLogfileComment("Name: " + packageData.getName() + 
                                                " " + packageData.getPackageName());
        }    
    }

    static public void dumpInstallPackages(PackageDescription packageData) {
 
        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.INSTALL )) {
            System.out.println("Now installing: " + packageData.getPackageName());
        }
        
        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpInstallPackages(child);
        }
    }
    
    static public void dumpUninstallPackages(PackageDescription packageData) {
 
        if (( packageData.isLeaf() ) && ( packageData.getSelectionState() == packageData.REMOVE )) {
            System.out.println("Now uninstalling: " + packageData.getPackageName());
        }
        
        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpUninstallPackages(child);
        }
    }
    
    static public void dumpAllRpmInfo(PackageDescription packageData) {

        if (( packageData.getPackageName() != null ) && ( ! packageData.getPackageName().equals(""))) {
            if ( packageData.pkgExists() ) {
                System.err.println("RPM data: " + packageData.getPkgRealName() + " : " +
                                                  packageData.getPkgVersion() + " : " +
                                                  packageData.getPkgRelease() + " : " + 
                                                  packageData.getPackageName() );            
            }
        }

        for (Enumeration e = packageData.children(); e.hasMoreElements(); ) {
            PackageDescription child = (PackageDescription) e.nextElement();
            dumpAllRpmInfo(child);
        }        
        
    }

    static public void dumpNewInstallData() {
        InstallData data = InstallData.getInstance();
        System.err.println("PackagePath: " + data.getPackagePath());
        System.err.println("AdminFileReloc: " + data.getAdminFileNameReloc());
        System.err.println("AdminFileRelocNoDepends: " + data.getAdminFileNameRelocNoDepends());
        System.err.println("AdminFileNoReloc: " + data.getAdminFileNameNoReloc());
        System.err.println("AdminFileNoRelocNoDepends: " + data.getAdminFileNameNoRelocNoDepends());
        System.err.println("DatabasePath: " + data.getDatabasePath());
        System.err.println("InstallDir: " + data.getInstallDir());
        System.err.println("Original privileges: " + data.getStoredInstallationPrivileges());        
        System.err.println("getuid.so File: " + data.getGetUidPath());
    }

    static public void dumpAllProperties() {
        Properties properties = System.getProperties();

        int size = properties.size();
        Iterator m = properties.entrySet().iterator();
        int counter = 0;
        
        while ( m.hasNext() ) {
            Map.Entry entry = (Map.Entry) m.next();
            String env = entry.getKey() + "=" + entry.getValue();
            System.err.println(env);
        }        
    }

}
