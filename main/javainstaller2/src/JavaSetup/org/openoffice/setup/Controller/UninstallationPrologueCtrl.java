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



package org.openoffice.setup.Controller;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.UninstallationPrologue;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.Converter;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.ModuleCtrl;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.HashMap;
import java.util.Vector;
import org.openoffice.setup.ResourceManager;

public class UninstallationPrologueCtrl extends PanelController {

    private String helpFile;
    
    public UninstallationPrologueCtrl() {
        super("UninstallationPrologue", new UninstallationPrologue());
        helpFile = "String_Helpfile_UninstallationPrologue";
    }

    public String getNext() {
        return new String("ChooseUninstallationType");
    }
    
    public String getPrevious() {
        return null;
    }
    
    public final String getHelpFileName () {
        return this.helpFile;
    }

    private HashMap getInfoFileData() {
        InstallData data = InstallData.getInstance();
        File infoRootDir = data.getInfoRoot();
        String infoFilename = "infoFile";
        File infoFile = new File(infoRootDir, infoFilename);
        Vector infoFileContent = SystemManager.readCharFileVector(infoFile.getPath());
        HashMap map = Converter.convertVectorToHashmap(infoFileContent);

        // for (int i = 0; i < infoFileContent.size(); i++) {
        //    System.out.println(infoFileContent.get(i));
        // }
        
        // Iterator m = map.entrySet().iterator();
        // while ( m.hasNext() ) {
        //     Map.Entry entry = (Map.Entry) m.next();
        //     System.out.println( "MAP:" + entry.getKey() + ":" + entry.getValue() );
        // }

        return map;
    }
    
    private void setNewInstallData(HashMap map) {
        InstallData data = InstallData.getInstance();
        // adding information to installData
        data.setPackagePath((String)map.get("PackagePath"));
        data.setAdminFileNameReloc((String)map.get("AdminFileReloc"));
        data.setAdminFileNameRelocNoDepends((String)map.get("AdminFileRelocNoDepends"));
        data.setAdminFileNameNoReloc((String)map.get("AdminFileNoReloc"));
        data.setAdminFileNameNoRelocNoDepends((String)map.get("AdminFileNoRelocNoDepends"));
        data.setDatabasePath((String)map.get("DatabasePath"));
        data.setInstallDir((String)map.get("InstallationDir"));
        data.setStoredInstallationPrivileges((String)map.get("InstallationPrivileges"));        
        data.setGetUidPath((String)map.get("GetUidFile"));
    }
    
    private void readInfoFile() {
        HashMap map = getInfoFileData();
        setNewInstallData(map);
        Dumper.dumpNewInstallData();
    }
    
    private void checkUninstallPrivileges() {
        InstallData data = InstallData.getInstance();
        // data.setStoredInstallationPrivileges((String)map.get("InstallationPrivileges"));
        String originalPrivileges = data.getStoredInstallationPrivileges();
        String currentPrivileges = data.getInstallationPrivileges();
        if ( ! currentPrivileges.equalsIgnoreCase(originalPrivileges) ) {
            // aborting installation with error message
            if ( currentPrivileges.equalsIgnoreCase("root")) {
                String message = ResourceManager.getString("String_UninstallationPrologue_Wrong_Privileges_Current_Root");
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                String log = "<b>Error: Wrong uninstallation privileges (currently Root)!</b><br>";
                System.err.println(log);
                // LogManager.addLogfileComment(log);                
            } else {
                String message = ResourceManager.getString("String_UninstallationPrologue_Wrong_Privileges_Current_User");
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                String log = "<b>Error: Wrong uninstallation privileges (currently User)!</b><br>";
                System.err.println(log);
                // LogManager.addLogfileComment(log);
            }
            System.exit(1);            
        }

    }
    
    public void beforeShow() {
        getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_PREVIOUS);
        // System.err.println("\nUninstallation module state dump 1:");
        // PackageDescription packageData = SetupDataProvider.getPackageDescription();                        
        // ModuleCtrl.dumpModuleStates(packageData);
        getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_NEXT);
    }

    public void duringShow() {

        Thread t = new Thread() {
            public void run() {
                InstallData installData = InstallData.getInstance();
                if ( ! installData.databaseAnalyzed() ) {
                    getSetupFrame().setButtonEnabled(false, getSetupFrame().BUTTON_NEXT);
                
                    // now it is time to read the infoFile in directory "installData.getInfoRoot()"
                    readInfoFile();
                    
                    // controlling the installation privileges. Are the original installation privileges
                    // identical with the current deinstallation privileges?
                    checkUninstallPrivileges();

                    PackageDescription packageData = SetupDataProvider.getPackageDescription();                
                    Installer installer = InstallerFactory.getInstance();
                    installer.preUninstall(packageData);

                    // searching in the database for already installed packages
                    LogManager.setCommandsHeaderLine("Analyzing system database");
                    ModuleCtrl.setDatabaseSettings(packageData, installData, installer);
                    installData.setDatabaseAnalyzed(true);
                    ModuleCtrl.setDontUninstallFlags(packageData);
                    if ( installData.isRootInstallation() ) { ModuleCtrl.setDontUninstallUserInstallOnylFlags(packageData); }
                    ModuleCtrl.setParentDefaultModuleSettings(packageData);
                    getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_NEXT);
                    getSetupFrame().setButtonSelected(getSetupFrame().BUTTON_NEXT);
                }
            }
        };
        
        t.start(); 
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;
        getSetupFrame().setButtonEnabled(true, getSetupFrame().BUTTON_PREVIOUS);
        return repeatDialog;
    }
    
}
