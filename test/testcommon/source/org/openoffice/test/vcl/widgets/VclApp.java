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

package org.openoffice.test.vcl.widgets;

import java.io.IOException;

import org.openoffice.test.OpenOffice;
import org.openoffice.test.common.Condition;
import org.openoffice.test.vcl.Tester;
import org.openoffice.test.vcl.client.CommandCaller;
import org.openoffice.test.vcl.client.CommunicationManager;
import org.openoffice.test.vcl.client.Constant;
import org.openoffice.test.vcl.client.Handshaker;
import org.openoffice.test.vcl.client.WinInfoReceiver;

/**
 * This class provides a proxy to interact with OpenOffice application.
 * 
 */
public class VclApp {
	
	protected CommunicationManager communicationManager = null;

	protected CommandCaller caller = null;
	
	protected OpenOffice openOffice = null;
	
	/**
	 * 
	 * @param openOffice
	 */
	public VclApp(OpenOffice openOffice) {
		this.openOffice = openOffice;
		this.communicationManager = new CommunicationManager("127.0.0.1", openOffice.getAutomationPort());
		this.caller = new CommandCaller(communicationManager);
		new Handshaker(communicationManager);
	}

	public void setWinInfoReceiver(WinInfoReceiver receiver) {
		caller.setWinInfoReceiver(receiver);
	}
	
	public void clean() {
		openOffice.kill();
		openOffice.cleanUserInstallation();
	}
	
	public void start() {
		openOffice.start();
		//wait for UI to be ready
		new Condition() {

			@Override
			public boolean value() {
				try {
					String tree = (String) caller.callCommand(Constant.RC_WinTree);
					return tree.length() > 0;
				} catch (Exception e) {
					return false;
				}
				
			}
			
		}.waitForTrue("Can't connect to automation server!", 20, 1);
	}
	
	public void start(boolean clean) { 
		if (clean) 
			clean();
		start();
	}
	
	/**
	 * Quit the application, if failed, one exception will be thrown
	 */
	public void quit() {
		dispatch(".uno:Quit");
		if (openOffice != null) {
			new Condition() {
				@Override
				public boolean value() {
					return !openOffice.isRunning();
				}

			}.waitForTrue("OpenOffice can't quit!", 5, 1);
		}
	}
	
	public void stop() {
		if (openOffice != null) {
			if (!openOffice.isRunning()) 
				return;
		}
		
		try {
			quit();
		} catch(Exception e) {
			openOffice.kill();
		}
		
		communicationManager.stop();
	}
	
	/**
	 * @deprecated use stop
	 */
	public void close() {
		stop();
	}
	
	
	/**
	 * Activate the document window at the given index
	 * Note: this method requires automation enabled.
	 * @param i
	 * @return
	 */
	public void activateDoc(int i) {
		caller.callCommand(Constant.RC_ActivateDocument, new Object[] { i + 1 });
	}

	/**
	 * Try to close all OpenOffice windows, until startcenter window appears.
	 * The method does not always succeed.
	 * Note: this method requires automation enabled.
	 */
	public void reset() {
		caller.callCommand(Constant.RC_ResetApplication);
	}

	/**
	 * Note: this method requires automation enabled.
	 * @return
	 */
	public boolean existsSysDialog() {
		return (Boolean) caller.callCommand(Constant.RC_ExistsSysDialog);
	}

	/**
	 * Note: this method requires automation enabled.
	 */
	public void closeSysDialog() {
		caller.callCommand(Constant.RC_CloseSysDialog);
	}

	/**
	 * Note: this method requires automation enabled.
	 * @return
	 */
	public String getClipboard() {
		return (String) caller.callCommand(Constant.RC_GetClipboard);
	}

	/**
	 * Note: this method requires automation enabled.
	 * @param content
	 */
	public void setClipboard(String content) {
		caller.callCommand(Constant.RC_SetClipboard, content);
	}

	/**
	 * Check if OpenOffice exists.
	 * Note: this method requires automation enabled.
	 * @return
	 */
	public boolean exists() {
		try {
			communicationManager.connect();
			return true;
		} catch (IOException e) {
			return false;
		}
	}

	/**
	 * Check if the control exists in a period of time
	 * Note: this method requires automation enabled.
	 */
	public boolean exists(double iTimeout) {
		return exists(iTimeout, 1);
	}

	/**
	 * Check if the control exists in a period of time
	 * Note: this method requires automation enabled.
	 */
	public boolean exists(double iTimeout, double interval) {
		long startTime = System.currentTimeMillis();
		while (System.currentTimeMillis() - startTime < iTimeout * 1000) {
			if (exists())
				return true;
			Tester.sleep(interval);
		}

		return exists();
	}

	/**
	 * Wait OpenOffice for existence in the given period.
	 * When time is out, an runtime exception will be thrown.
	 * Note: this method requires automation enabled.
	 * @param iTimeout
	 * @param interval
	 */
	public void waitForExistence(double iTimeout, double interval) {
		if (!exists(iTimeout, interval))
			throw new RuntimeException("OpenOffice is not found!");
	}

	/**
	 * Get document window count
	 * Note: this method requires automation enabled.
	 * @return
	 */
	public int getDocCount() {
		return (Integer) caller.callCommand(Constant.RC_GetDocumentCount);
	}

	/**
	 * Run a dispatch
	 * Note: this method requires automation enabled.
	 * @param url
	 */
	public void dispatch(String url) {
		caller.callUNOSlot(url);
	}

	private static final int CONST_WSTimeout = 701;

	// private static final int CONST_WSAborted = 702; // Not used now!
	// private static final int CONST_WSFinished = 703; //

	/**
	 * Run a dispatch and then wait some time.
	 * If time is out, an runtime exception will be thrown
	 * Note: this method requires automation enabled.
	 * @param url
	 * @param time timeout
	 */
	public void dispatch(String url, double time) {
		caller.callUNOSlot(url);
		waitSlot(time);
	}
	
	public void waitSlot(double time) {
		int result = (Integer) caller.callCommand(Constant.RC_WaitSlot, (int) time * 1000);
		if (result == CONST_WSTimeout)
			throw new RuntimeException("Timeout to execute the dispatch!");
	}
}
