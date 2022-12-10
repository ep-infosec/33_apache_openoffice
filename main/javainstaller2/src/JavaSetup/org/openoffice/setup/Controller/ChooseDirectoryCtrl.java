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

import java.awt.Dimension;
import org.openoffice.setup.InstallData;
import org.openoffice.setup.Installer.Installer;
import org.openoffice.setup.Installer.InstallerFactory;
import org.openoffice.setup.PanelController;
import org.openoffice.setup.Panel.ChooseDirectory;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.SetupDataProvider;
import org.openoffice.setup.Util.InstallChangeCtrl;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.Vector;
import javax.swing.JDialog;
import javax.swing.JOptionPane;
import org.openoffice.setup.Dialogs.DatabaseDialog;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.Util.Controller;
import org.openoffice.setup.Util.Dumper;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.ModuleCtrl;
import org.openoffice.setup.Util.PackageCollector;

public class ChooseDirectoryCtrl extends PanelController {

    private String helpFile;
    
    public ChooseDirectoryCtrl() {
        super("ChooseDirectory", new ChooseDirectory());
        helpFile = "String_Helpfile_ChooseDirectory";
    }

    public String getNext() {

        InstallData data = InstallData.getInstance();

        if ( data.olderVersionExists() ) {
            return new String("InstallationImminent");
        } else if ( data.sameVersionExists() ) {
            return new String("ChooseComponents");            
        } else {
            return new String("ChooseInstallationType");
        }
    }
    
    public String getPrevious() {
        InstallData data = InstallData.getInstance();

        if ( data.hideEula() ) {
            return new String("Prologue");            
        } else {
            return new String("AcceptLicense");
        }
    }

    public final String getHelpFileName () {
        return this.helpFile;
    }

    private String removeEndingDelimiter(String s) {
        s.trim();
        if (( s.endsWith("/")) || ( s.endsWith("\\"))) {
            // removing the last character
            s = s.substring(0, s.length() - 1);
        }
        // mask spaces
        // s.replaceAll(" ", "\\ ");
        
        return s;
    }

    public void beforeShow() {
        ChooseDirectory panel = (ChooseDirectory)getPanel();
        InstallData data = InstallData.getInstance();
        
        if ( data.getInstallDir() == null ) {
            String installDir = data.getDefaultDir();

            if ( data.isUserInstallation() ) {
                // System.getenv only supported in Java 1.5, property set in shell script
                // if (( System.getenv("HOME") != null ) && ( ! System.getenv("HOME").equals(""))) {
                //     rootDir = System.getenv("HOME");
                // }
                if (( System.getProperty("HOME") != null ) && ( ! System.getProperty("HOME").equals("") )) {
                    installDir = System.getProperty("user.home");
                }
            }
            data.setInstallDir(installDir);
        }

        panel.setDirectory(data.getInstallDir());
    }

    public boolean afterShow(boolean nextButtonPressed) {
        boolean repeatDialog = false;
        ChooseDirectory panel = (ChooseDirectory)getPanel();
        String dir = panel.getDirectory();
        dir = removeEndingDelimiter(dir);
        InstallData data = InstallData.getInstance();
        data.setInstallDir(dir);
        
        File installDefaultDir = new File(dir, data.getDefaultDir());
        data.setInstallDefaultDir(installDefaultDir.getPath());

        SetupDataProvider.setNewMacro("DIR", installDefaultDir.getPath()); // important for string replacement
        // SetupDataProvider.dumpMacros();

        // Check existence of directory. Try to create, if it does not exist.
        // If successufully created, calculate available disc space

        if ( nextButtonPressed ) {

            // If the directory exists, is has to be tested, whether the user has write access

            if ( SystemManager.exists_directory(dir) ) {
            	if ( ! Controller.createdSubDirectory(dir) ) {
            	    repeatDialog= true;
            	}
            }

            // If the directory does not exist, is has to be tested, whether the user can create it

            if ( ! SystemManager.exists_directory(dir)) {
                String title = ResourceManager.getString("String_ChooseDirectory4_Question_Title");
                String message = null;
                message = ResourceManager.getString("String_ChooseDirectory5_Question_Message");

                int returnValue = JOptionPane.showConfirmDialog(null, message, title, JOptionPane.YES_NO_OPTION, JOptionPane.QUESTION_MESSAGE);

                if (  returnValue == JOptionPane.YES_OPTION ) {
                    if ( ! Controller.createdDirectory(dir) ) {
            	        repeatDialog= true;
            	    }
                }
                else if (returnValue == JOptionPane.NO_OPTION) {
                    repeatDialog = true;
                }
            }

            // Additional tasks, if the directory is okay
            
            if ( ! repeatDialog ) {
                // Calculate available disc space
                int discSpace = SystemManager.calculateDiscSpace(dir);
                data.setAvailableDiscSpace(discSpace);

                Installer installer = InstallerFactory.getInstance();                

                // Is this a new directory, or one that was already set before.
                // In this case, the database does not need to be evaluated again.
                // -> Testing, whether the database path has changed. This can
                // only happen in user installations.
                if ( data.isUserInstallation() ) {
                    String oldDatabasePath = data.getDatabasePath();
                    // Setting the database path
                    installer.defineDatabasePath();
                    
                    if (( oldDatabasePath == null ) || ( ! oldDatabasePath.equals(data.getDatabasePath()))) {
                        data.setDatabaseAnalyzed(false);
                        data.setDatabaseQueried(false);
                    } else {
                        data.setDatabaseAnalyzed(true);                        
                    }
                }

                // In installations with user privileges, now it can be controlled,
                // if there are products installed in the selected directory.
                // Therefore the directory selection dialog has to be shown before.
                // In installations with root privileges, this can only be checked,
                // before the destination directory can be set, because it is fix.
                if ( data.isUserInstallation() ) {
                    LogManager.setCommandsHeaderLine("Checking change installation");
                    InstallChangeCtrl.checkInstallChange(data);
                    // InstallChangeCtrl.checkInstallChange(data, panel);
                }
                                
                // At this point it is clear, whether the update-Package is installed or not.
                // If it is installed, it is also clear, in which version it is installed.
                // Therefore the following dialog is also defined.                
                
                if ( data.newerVersionExists() ) {
                    // This can happen only in installation with user privileges.
                    // Installations with root privileges have cancelled installation 
                    // already at first dialog.
                    // Possibility to select a new directory, in which no 
                    // newer product exists
                    String message = ResourceManager.getString("String_Newer_Version_Installed_Found") + "\n" + data.getInstallDir() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Database") + ": " + data.getDatabasePath() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Tip");
                    String title = ResourceManager.getString("String_Error");
                    Informer.showErrorMessage(message, title);

                    repeatDialog = true;
                }
                
                // If an older version is found, and the update is forbidden, the user
                // can select another installation directory.

                if ( data.olderVersionExists() && data.dontUpdate() ) {
                    // This can happen only in installation with user privileges.
                    // Installations with root privileges have cancelled installation 
                    // already at first dialog (InstallChangeCtrl.java).
                    // Possibility to select a new directory, in which no 
                    // older product exists
                    String message = ResourceManager.getString("String_Older_Version_Installed_Found") + "\n" + data.getInstallDir() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Database") + ": " + data.getDatabasePath() + "\n" +
                                     ResourceManager.getString("String_Newer_Version_Tip");
                    String title = ResourceManager.getString("String_Error");
                    Informer.showErrorMessage(message, title);

                    repeatDialog = true;
                }

                if ( ! repeatDialog ) {

                    if ( ! data.databaseAnalyzed()) {
                        ModuleCtrl.defaultDatabaseAnalysis(data);
                        data.setDatabaseAnalyzed(true);
                    }
                }
            }
        }
        
        return repeatDialog;
    }

}
