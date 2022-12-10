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



package org.openoffice.setup.Installer;

import org.openoffice.setup.InstallData;
import org.openoffice.setup.InstallerHelper.SolarisHelper;
import org.openoffice.setup.ResourceManager;
import org.openoffice.setup.SetupData.PackageDescription;
import org.openoffice.setup.Util.Converter;
import org.openoffice.setup.Util.ExecuteProcess;
import org.openoffice.setup.Util.Informer;
import org.openoffice.setup.Util.LogManager;
import org.openoffice.setup.Util.SystemManager;
import java.io.File;
import java.util.HashMap;
import java.util.Vector;

public class SolarisInstaller extends Installer {
    
    SolarisHelper helper = new SolarisHelper();
    
    public SolarisInstaller() {
        super();
    }
 
    public void defineDatabasePath() {

        InstallData data = InstallData.getInstance();
        String oldDatabasePath = data.getDatabasePath();
        data.setDatabasePath(null);

        // Determining the database path (only for user installation).
        // Important if a user installation is done into an existing
        // user installation -> this methode can only be called after
        // determination of installation directory.

        if ( data.isUserInstallation() ) {
            String databasePath = helper.getSolarisDatabasePath(data); // the file does not need to exist!
            data.setDatabasePath(databasePath);

            // If this is a new path to the database, then this database was
            // not analyzed before (when going back in installation wizard)
            if ( ! databasePath.equals(oldDatabasePath) ) {
                data.setDatabaseAnalyzed(false);
                data.setDatabaseQueried(false);
            }        
        }
    }

    public void preInstallationOngoing() {
        // an admin file has to be created for user and for root installation
        InstallData data = InstallData.getInstance();
        
        if ( data.isInstallationMode()) {
            boolean makeRelocatableAdminFile = true;
            boolean removeDepends = true;
            helper.createAdminFile(makeRelocatableAdminFile, removeDepends);
            helper.createAdminFile(makeRelocatableAdminFile, ! removeDepends);
            helper.createAdminFile(! makeRelocatableAdminFile, removeDepends);
            helper.createAdminFile(! makeRelocatableAdminFile, ! removeDepends);
        }

        if ( data.isUserInstallation() ) {
            // LD_PRELOAD_32 needs to be set for user installation
            helper.setEnvironmentForUserInstall();
            // Solaris 10 needs to have local temp directory for pkgadd
            // createLocalTempDir();
        }
    }

    public void postInstallationOngoing() {
        InstallData data = InstallData.getInstance();
        // if ( data.isUserInstallation() ) {
            // Solaris 10 needs to have local temp directory for pkgadd
            // removeLocalTempDir();
        // }
        
        helper.removeSolarisLockFile();
    }

    public void postInstall(PackageDescription packageData) {
        InstallData data = InstallData.getInstance();

        if ( ! data.isAbortedInstallation() ) {
            data.setStillRunning(true);
            // Collecting information about installed packages
            // Creating a list containing pairs of package names and rpm file names
            // that has to be used during uninstallation.
            helper.saveModulesLogFile(data);
            data.setStillRunning(false);
        }
    }
    
    public void postUninstallationOngoing() {
        helper.removeSolarisLockFile();        
    }
    
    public String getChangeInstallDir(PackageDescription packageData) {
        String installDir = null;
        String packageName = packageData.getPackageName(); 
       
        if ( packageName != null ) {
            String pkgCommand = "pkginfo -r" + " " + packageName;
            String[] pkgCommandArray = new String[3];
            pkgCommandArray[0] = "pkginfo";
            pkgCommandArray[1] = "-r";
            pkgCommandArray[2] = packageName;
            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();
            int returnValue = ExecuteProcess.executeProcessReturnVector(pkgCommandArray, returnVector, returnErrorVector);
            String returnString = (String) returnVector.get(0);
            
            String log = pkgCommand + "<br><b>Returns: " + returnString + "</b><br>";
            LogManager.addCommandsLogfileComment(log);

            installDir = returnString;            
        }
        
        return installDir;
    }
        
    public void installPackage(PackageDescription packageData) {
        InstallData data = InstallData.getInstance();

        String log = "<br><b>Package: " + packageData.getName() + "</b>";
        LogManager.addCommandsLogfileComment(log);

        String rootDir = data.getInstallDir();
        String packagePath = data.getPackagePath();

        if (( packageData.getPkgSubdir() != null ) && ( ! packageData.getPkgSubdir().equals("") )) {
            File completePackageFile = new File(packagePath, packageData.getPkgSubdir());
            packagePath = completePackageFile.getPath();
        }

        String packageName = packageData.getPackageName();
        
        if (( packageName.equals("")) || ( packageName == null )) {
            log = "<b>No package name specified. Nothing to do</b>";
            LogManager.addCommandsLogfileComment(log);
        } else {
            log = "<b>Package Name: " + packageName + "</b>";
            LogManager.addCommandsLogfileComment(log);

            File completePackage = new File(packagePath, packageName);

            if ( completePackage.exists() ) {

                String pkgCommand = "";
                String[] pkgCommandArray;
                String adminFileName = "";
                Vector returnVector = new Vector();
                Vector returnErrorVector = new Vector();
                int returnValue;

                // is package relocatable or not?
                if ( packageData.isRelocatable() ) {
                    adminFileName = data.getAdminFileNameReloc();
                } else {
                    adminFileName = data.getAdminFileNameNoReloc();
                }

                if ( data.isUserInstallation() ) {
                    HashMap env = data.getShellEnvironment();
                    String[] envStringArray = Converter.convertHashmapToStringArray(env);
                    pkgCommand = "/usr/sbin/pkgadd -n -d " + packagePath + " -R " +
                            rootDir + " -a " + adminFileName + " " + packageName;
                    pkgCommandArray = new String[9];
                    pkgCommandArray[0] = "/usr/sbin/pkgadd";
                    pkgCommandArray[1] = "-n";
                    pkgCommandArray[2] = "-d";
                    pkgCommandArray[3] = packagePath;
                    pkgCommandArray[4] = "-R";
                    pkgCommandArray[5] = rootDir;
                    pkgCommandArray[6] = "-a";
                    pkgCommandArray[7] = adminFileName;
                    pkgCommandArray[8] = packageName;
                    returnValue = ExecuteProcess.executeProcessReturnVectorEnv(pkgCommandArray, envStringArray, returnVector, returnErrorVector);
                } else {
                    // shifting of the installation root (-R) is only wanted for user installation
                    pkgCommand = "/usr/sbin/pkgadd -n -d " + packagePath +
                            " -a " + adminFileName + " " + packageName;
                    pkgCommandArray = new String[7];
                    pkgCommandArray[0] = "/usr/sbin/pkgadd";
                    pkgCommandArray[1] = "-n";
                    pkgCommandArray[2] = "-d";
                    pkgCommandArray[3] = packagePath;
                    pkgCommandArray[4] = "-a";
                    pkgCommandArray[5] = adminFileName;
                    pkgCommandArray[6] = packageName;
                    returnValue = ExecuteProcess.executeProcessReturnVector(pkgCommandArray, returnVector, returnErrorVector);
                }
                
                if ( returnValue == 0 ) {
                    log = pkgCommand + "<br><b>Returns: " + returnValue + " Successful installation</b><br>";
                    LogManager.addCommandsLogfileComment(log);                        
                } else {    // an error occurred during installation
                    if ( packageData.installCanFail() ) {
                        log = pkgCommand + "<br><b>Returns: " + returnValue + " Problem during installation. Can be ignored.</b><br>";
                        LogManager.addCommandsLogfileComment(log);
                    } else {
                        log = pkgCommand + "<br><b>Returns: " + returnValue + " Error during installation</b><br>";
                        LogManager.addCommandsLogfileComment(log);
                        System.err.println("Error during installation:");
                        for (int i = 0; i < returnErrorVector.size(); i++) {
                            LogManager.addCommandsLogfileComment((String)returnErrorVector.get(i));
                            System.err.println(returnErrorVector.get(i));
                        }
                        data.setIsErrorInstallation(true);
                    }
                }
            }
            else {
                log = "<b>Error: Did not find package " + packageName + "</b><br>";
                System.err.println(log);
                String message = ResourceManager.getString("String_File_Not_Found") + ": " + packageName;
                String title = ResourceManager.getString("String_Error");
                Informer.showErrorMessage(message, title);
                LogManager.addCommandsLogfileComment(log);
                data.setIsErrorInstallation(true);
            }
        }
    }

    public void uninstallPackage(PackageDescription packageData) {
        InstallData data = InstallData.getInstance();

        String log = "<br><b>Package: " + packageData.getName() + "</b>";
        LogManager.addCommandsLogfileComment(log);
       
        String installRoot = data.getInstallDir();
        String packageName = packageData.getPackageName();
        String adminFileName = "";

        // is package relocatable or not?
        if ( packageData.isRelocatable() ) {
        	if ( packageData.ignoreDependsForUninstall() ) {    // Force removal of older packages during installation
                adminFileName = data.getAdminFileNameRelocNoDepends();
            } else {
                adminFileName = data.getAdminFileNameReloc();
            }
        } else {
        	if ( packageData.ignoreDependsForUninstall() ) {    // Force removal of older packages during installation
                adminFileName = data.getAdminFileNameNoRelocNoDepends();
            } else {
                adminFileName = data.getAdminFileNameNoReloc();
            }
        }
        
        String pkgCommand = "";
        String[] pkgCommandArray;
        int returnValue;
        Vector returnVector = new Vector();
        Vector returnErrorVector = new Vector();

        if ( data.isUserInstallation() ) {
            pkgCommand = "/usr/sbin/pkgrm -n " + "-R " + installRoot + " -a " + adminFileName + " " + packageName;
            pkgCommandArray = new String[7];
            pkgCommandArray[0] = "/usr/sbin/pkgrm";
            pkgCommandArray[1] = "-n";
            pkgCommandArray[2] = "-R";
            pkgCommandArray[3] = installRoot;
            pkgCommandArray[4] = "-a";
            pkgCommandArray[5] = adminFileName;
            pkgCommandArray[6] = packageName;
            HashMap env = data.getShellEnvironment();
            String[] envStringArray = Converter.convertHashmapToStringArray(env);
            // SystemManager.dumpStringArray(envStringArray);
            returnValue = ExecuteProcess.executeProcessReturnVectorEnv(pkgCommandArray, envStringArray, returnVector, returnErrorVector);
        } else {
            // shifting of the installation root (-R) is only wanted for user installation
            pkgCommand = "/usr/sbin/pkgrm -n " + "-a " + adminFileName + " " + packageName;
            pkgCommandArray = new String[5];
            pkgCommandArray[0] = "/usr/sbin/pkgrm";
            pkgCommandArray[1] = "-n";
            pkgCommandArray[2] = "-a";
            pkgCommandArray[3] = adminFileName;
            pkgCommandArray[4] = packageName;
            returnValue = ExecuteProcess.executeProcessReturnVector(pkgCommandArray, returnVector, returnErrorVector);
        }

        if ( returnValue == 0 ) {
            log = pkgCommand + "<br><b>Returns: " + returnValue + " Successful uninstallation</b><br>";
            LogManager.addCommandsLogfileComment(log);                        
        } else {    // an error occurred during installation                    
            if ( packageData.uninstallCanFail() ) {
                log = pkgCommand + "<br><b>Returns: " + returnValue + " Problem during uninstallation. Can be ignored.</b><br>";
                LogManager.addCommandsLogfileComment(log);
            } else {
                log = pkgCommand + "<br><b>Returns: " + returnValue + " Error during uninstallation</b><br>";
                LogManager.addCommandsLogfileComment(log);
                System.err.println("Error during uninstallation:");
                for (int i = 0; i < returnErrorVector.size(); i++) {
                    LogManager.addCommandsLogfileComment((String)returnErrorVector.get(i));
                    System.err.println(returnErrorVector.get(i));
                }
                data.setIsErrorInstallation(true);
            }
        }
    }

    public boolean isPackageNameInstalledClassic(String packageName, InstallData installData) {
        String rootString = "";
        String rootPath = null;
        String pkgCommand;
        String[] pkgCommandArray;
        boolean useLocalRoot = false;
        boolean isInstalled = false;

        if (installData.isUserInstallation()) {
            rootPath = installData.getDatabasePath();
        }
    
        if (( rootPath != null ) && (! rootPath.equals("null"))) {
            rootString = "-R";
            useLocalRoot = true;
        }
        
        if (useLocalRoot) {
            pkgCommand = "pkginfo " + rootString + " " + rootPath + " " + packageName;
            pkgCommandArray = new String[4];
            pkgCommandArray[0] = "pkginfo";
            pkgCommandArray[1] = rootString;
            pkgCommandArray[2] = rootPath;
            pkgCommandArray[3] = packageName;
        } else {
            pkgCommand = "pkginfo " + packageName;
            pkgCommandArray = new String[2];
            pkgCommandArray[0] = "pkginfo";
            pkgCommandArray[1] = packageName;
        }

        // Vector returnVector = new Vector();
        int returnValue = ExecuteProcess.executeProcessReturnValue(pkgCommandArray);
        
        if ( returnValue == 0 ) {
            isInstalled = true;
            String log = pkgCommand + "<br><b>Returns: " + returnValue + " Package is installed" + "</b><br>";
            LogManager.addCommandsLogfileComment(log);
        } else {
            String log = pkgCommand + "<br><b>Returns: " + returnValue + " Package is not installed" + "</b><br>";
            LogManager.addCommandsLogfileComment(log);                
        }
    	
        return isInstalled;
    }

    private void queryAllDatabase(InstallData installData) {

        String rootString = "";
        String rootPath = null;
        String pkgCommand;
        String[] pkgCommandArray;
        boolean useLocalRoot = false;
        HashMap map = new HashMap();;

        if (installData.isUserInstallation()) {
            rootPath = installData.getDatabasePath();
        }
    
        if (( rootPath != null ) && (! rootPath.equals("null"))) {
            rootString = "-R";
            useLocalRoot = true;
        }
        
        if (useLocalRoot) {
            pkgCommand = "pkginfo " + rootString + " " + rootPath;
            pkgCommandArray = new String[4];
            pkgCommandArray[0] = "pkginfo";
            pkgCommandArray[1] = "-x";
            pkgCommandArray[2] = rootString;
            pkgCommandArray[3] = rootPath;
        } else {
            pkgCommand = "pkginfo -x";
            pkgCommandArray = new String[2];
            pkgCommandArray[0] = "pkginfo";
            pkgCommandArray[1] = "-x";
        }

        Vector returnVector = new Vector();
        Vector returnErrorVector = new Vector();
        int returnValue = ExecuteProcess.executeProcessReturnVector(pkgCommandArray, returnVector, returnErrorVector);
    
        String log = pkgCommand + "<br><b>Returns: " + returnValue + "</b><br>";
        LogManager.addCommandsLogfileComment(log);
        String value = "1";
        
        if ( ! returnVector.isEmpty()) {
            for (int i = 0; i < returnVector.size(); i++) {
                String onePackage = (String)returnVector.get(i);
                int pos1 = onePackage.indexOf(" ");
                String key = onePackage.substring(0, pos1);
                map.put(key, value);
            }
        }

        installData.setDatabaseQueried(true);
        installData.setDatabaseMap(map);

    }

    public boolean isPackageNameInstalled(String packageName, InstallData installData) {

        boolean isInstalled = false;

        HashMap map = null;
        if ( ! installData.databaseQueried() ) {
            queryAllDatabase(installData);
        }

        map = installData.getDatabaseMap();
                
        if ( map.containsKey(packageName)) {
            isInstalled = true;
        }                

        return isInstalled;
    }

    public boolean isPackageInstalled(PackageDescription packageData, InstallData installData) {
        boolean isInstalled = false;
        
        String packageName = packageData.getPackageName();

        if ( packageName.equals("")) {
            packageName = null;
        }
                
        if ( packageName != null ) {
        	isInstalled = isPackageNameInstalled(packageName, installData);            
        }

        return isInstalled;
    }    

    public boolean isInstallSetPackageOlder(PackageDescription packageData, InstallData installData) {
        boolean installSetPackageIsOlder = false;
        boolean checkIfInstalledIsOlder = false;
        installSetPackageIsOlder = findOlderPackage(packageData, installData, checkIfInstalledIsOlder);
        return installSetPackageIsOlder;    
    }

    public boolean isInstalledPackageOlder(PackageDescription packageData, InstallData installData) {
        boolean installedPackageIsOlder = false;
        boolean checkIfInstalledIsOlder = true;
        installedPackageIsOlder = findOlderPackage(packageData, installData, checkIfInstalledIsOlder);
        return installedPackageIsOlder;
    }

    private boolean findOlderPackage(PackageDescription packageData, InstallData installData, boolean checkIfInstalledIsOlder) {

        boolean firstPackageIsOlder = false;
        String packageName = packageData.getPackageName();
        String log;

        if ( packageName.equals("")) {
            packageName = null;
        }

        if ( packageName != null ) {
            String rootString = "";
            String rootPath = null;
            String pkgCommand;
            String[] pkgCommandArray;
            boolean useLocalRoot = false;

            if (installData.isUserInstallation()) {
                rootPath = installData.getDatabasePath();
            }

            if (( rootPath != null ) && (! rootPath.equals("null"))) {
                rootString = "-R";
                useLocalRoot = true;
            }

            if (useLocalRoot) {
                // String pkgCommand = "pkginfo -l" + rootString + " " + rootPath + " " + packageName + " | grep VERSION:";
                pkgCommand = "pkginfo -x " + rootString + " " + rootPath + " " + packageName;
                pkgCommandArray = new String[5];
                pkgCommandArray[0] = "pkginfo";
                pkgCommandArray[1] = "-x";
                pkgCommandArray[2] = rootString;
                pkgCommandArray[3] = rootPath;
                pkgCommandArray[4] = packageName;
                
            } else {
                // String pkgCommand = "pkginfo -l" + rootString + " " + rootPath + " " + packageName + " | grep VERSION:";
                pkgCommand = "pkginfo -x " + packageName;
                pkgCommandArray = new String[3];
                pkgCommandArray[0] = "pkginfo";
                pkgCommandArray[1] = "-x";
                pkgCommandArray[2] = packageName;
            }

            Vector returnVector = new Vector();
            Vector returnErrorVector = new Vector();

            int returnValue = ExecuteProcess.executeProcessReturnVector(pkgCommandArray, returnVector, returnErrorVector);

            log = pkgCommand + "<br><b>Returns:</b>";
            LogManager.addCommandsLogfileComment(log);
            for (int i = 0; i < returnVector.size(); i++) {
                log = "<b>" + returnVector.get(i) + "</b>";
                LogManager.addCommandsLogfileComment(log);
            }

            // log = "<br>";
            // LogManager.addCommandsLogfileComment(log);

            String installedPackageVersion = helper.getVersionString(returnVector);
            String newPackageVersion = packageData.getPkgVersion();

            if ( ! installData.installedProductMinorSet() ) {
                int productMinor = helper.getInstalledMinor(installedPackageVersion);
                installData.setInstalledProductMinor(productMinor);
                installData.setInstalledProductMinorSet(true);
            }
                        
            if (( installedPackageVersion != null ) && ( newPackageVersion != null )) {
            	if ( checkIfInstalledIsOlder ) {
                    firstPackageIsOlder = helper.comparePackageVersions(installedPackageVersion, newPackageVersion);
                } else {
                    firstPackageIsOlder = helper.comparePackageVersions(newPackageVersion, installedPackageVersion);
                }
            }
        }        

        if ( checkIfInstalledIsOlder ) {
            if ( firstPackageIsOlder ) {
                log = "<b>-> Installed package is older</b><br>";
                LogManager.addCommandsLogfileComment(log);
            } else {
                log = "<b>-> Installed package is not older</b><br>";
                LogManager.addCommandsLogfileComment(log);                
            }
        } else {
            if ( firstPackageIsOlder ) {
                log = "<b>-> Package in installation set is older</b><br>";
                LogManager.addCommandsLogfileComment(log);
            } else {
                log = "<b>-> Package in installation set is not older</b><br>";
                LogManager.addCommandsLogfileComment(log);                
            }        
        }       

        return firstPackageIsOlder;
    }

}
