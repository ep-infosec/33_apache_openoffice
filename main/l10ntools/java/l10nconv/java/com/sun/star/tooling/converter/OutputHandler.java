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


/*
 * Creates logfile and file for debug information
 * 
 */
package com.sun.star.tooling.converter;

import java.io.BufferedWriter;
import java.io.IOException;

/**
 * Handle the whole output during converting process except the converted files. 
 * Write output to logfiles and to screen depending on switches given 
 * at command line.
 * @author Christian Schmidt 2005
 *  
 */
public class OutputHandler {

    private static final String  EMPTY   = "";

    /**
     * The logfile to write
     */
    public static BufferedWriter logFile;

    /**
     * Indicates whether excisting files should be overwritten
     */
    public static boolean        doLog   = false;

    /**
     * Indicate whether the debug information should be written
     */
    public static boolean        doDebug = false;

    /**
     * The File to write the debug information to
     */
    public static BufferedWriter dbgFile;

    /**
     * Write the data to screen and if the switches were set to
     * logfile and debugfile
     * 
     * @param out the String to write
     */
    final static void out(String out) {
        try {
            if (doDebug)
                OutputHandler.dbg(out);

            if (!doLog || logFile == null) {
                System.out.println(out);
            } else {
                System.out.println(out);
                if (EMPTY.equals(out)) {
                    OutputHandler.log(EMPTY);
                } else {
                    OutputHandler.log(out);
                }
            }
        } catch (IOException e) {

            OutputHandler.log(e.toString());
        }
    }

    /**
     * make a dot on the screen to show the user that it is going on
     */
    final static void printDot() {
        System.out.print(".");

    }

    /**
     * write the data to log file and if the switch is set 
     * to the debug file
     * 
     * @param out the String to write
     */
    final static void log(String out) {
        try {
            if(out==null) return;
            if (doDebug)

                OutputHandler.dbg(out);

            if (doLog) {
                if (!EMPTY.equals(out)) {
                    logFile.write(out);
                }
                logFile.newLine();
                logFile.flush();
            }
        } catch (IOException e) {

            System.out.println(e.getMessage());
        }
    }

    /**
     * write the string to the debug file
     * 
     * @param out the String to write
     * @throws IOException
     */
    final static void dbg(String out) throws IOException {
        if(out==null) return;
        if (doDebug) {
            if (!EMPTY.equals(out)) {
                dbgFile.write(out);

            }
            dbgFile.newLine();
            dbgFile.flush();
        }
    }

    /**
     * Set a new value to indicate whether a debug file should be written or not
     * 
     * @param doDebug
     *            The debug value to set.
     */
    private static void setDebug(boolean doDebug) {
        OutputHandler.doDebug = doDebug;
    }

    //	/**
    //	 * @return Returns the doDebug.
    //	 */
    //	private static boolean isDebug() {
    //		return doDebug;
    //	}

    /**
     * Set the debug file to write to. 
     * 
     * @param dbgFile
     *            The debug File to use.
     */
    private static void setDbgFile(BufferedWriter dbgFile) {
        OutputHandler.dbgFile = dbgFile;
    }

    /**
     * @return Returns the debug File.
     */
    private static BufferedWriter getDbgFile() {
        return dbgFile;
    }

    /**
     * @param doLog
     *            The log value to set.
     */
    private static void setLog(boolean doLog) {
        OutputHandler.doLog = doLog;
    }

    /**
     * @return Returns the log value.
     */
    private static boolean isLog() {
        return doLog;
    }

    /**
     * @param logFile
     *            The logFile to set.
     */
    private static void setLogFile(BufferedWriter logFile) {
        OutputHandler.logFile = logFile;
    }

    /**
     * @return Returns the logFile.
     */
    private static BufferedWriter getLogFile() {
        return logFile;
    }

    /**
     * @throws IOException
     *             close log and dbg
     */
    public static void closeAll() throws IOException {
        if (OutputHandler.logFile != null) {
            OutputHandler.logFile.flush();
            OutputHandler.logFile.close();
        }
        if (OutputHandler.dbgFile != null) {
            OutputHandler.dbgFile.flush();
            OutputHandler.dbgFile.close();
        }
    }

}