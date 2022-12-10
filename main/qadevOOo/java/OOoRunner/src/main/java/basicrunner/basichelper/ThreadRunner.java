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



package basicrunner.basichelper;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.util.XURLTransformer;
import com.sun.star.frame.XController;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.URL;
import com.sun.star.lang.XMultiServiceFactory;
import util.utils;
import com.sun.star.lang.XComponent;
import lib.StatusException;
import lib.Status;
import util.SOfficeFactory;
import com.sun.star.ui.dialogs.XExecutableDialog;


public class ThreadRunner implements XServiceInfo, XSingleServiceFactory {
    static final String __serviceName = "basichelper.ThreadRunner";
    static ThreadRunnerImpl oThreadRunner = null;

    public ThreadRunner(XMultiServiceFactory xMSF) {
	oThreadRunner = new ThreadRunnerImpl(xMSF);
    }

    public Object createInstanceWithArguments(Object[] args) {
	return oThreadRunner;
    }

    public Object createInstance() {
        return createInstanceWithArguments(null);
    }

    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        return types;
    }

    public boolean supportsService(String name) {
        return __serviceName.equals(name);
    }

    public String[] getSupportedServiceNames() {
        return new String[] {__serviceName};
    }

    public String getImplementationName() {
        return getClass().getName();
    }
}


class ThreadRunnerImpl implements XInitialization, XTypeProvider {
    Object oDoc = null;
    String actionType = null;
    String actionParm = null;
    XMultiServiceFactory oMSF = null;

    public ThreadRunnerImpl(XMultiServiceFactory MSF) {
	oMSF = MSF;
    }

    public void initialize(Object[] params) throws com.sun.star.uno.Exception {
	boolean parmsOK = false;

        if (!(params[0] instanceof String)) {
            throw new StatusException(Status.failed(
                "Wrong first parameter for ThreadRunner, allowed values:" +
                "'OpenToolkitDialog', 'OpenDialogFromFile', 'ExecuteDialog'"));
        }

        actionType = (String) params[0];

        if (actionType.equals("OpenToolkitDialog")) {
            if (params.length != 3 || !(params[2] instanceof Object) ||
                !(params[1] instanceof String) ) {
                throw new StatusException(Status.failed("Expected the " +
                    "following type of parameters for 'OpenToolkitDialog': " +
                    "String, Object"));
            }
            actionParm = (String)params[1];
            oDoc = (Object)params[2];
            ToolkitDialogThread aThread =
                new ToolkitDialogThread(oMSF, oDoc, actionParm);
            aThread.start();
        } else if (actionType.equals("OpenDialogFromFile")) {
            if (params.length != 2 || !(params[1] instanceof String) ) {
                throw new StatusException(Status.failed("Expected the " +
                    "following type of parameters for 'OpenDialogFromFile': " +
                    "String"));
            }
            actionParm = (String)params[1];
            DialogFromFileThread bThread =
                new DialogFromFileThread(oMSF, actionParm);
            bThread.start();
        } else if ( actionType.equals("ExecuteDialog")) {
            if (params.length != 2 || !(params[1] instanceof String)) {
                throw new StatusException(Status.failed("Expected the " +
                    "following type of parameters for 'ExecuteDialog': " +
                    "String"));
            }
            ExecuteDialogThread cThread =
                new ExecuteDialogThread(oMSF, (String)params[1]);
            cThread.start();
        } else {
            System.out.println("Error! ThreadRunnerImpl.initialize(): " +
            "Incorrect parameters!");
        }
    }

    public byte[] getImplementationId() {
        return toString().getBytes();
    }

    public Type[] getTypes() {
        Class interfaces[] = getClass().getInterfaces();
        Type types[] = new Type[interfaces.length];
        for(int i = 0; i < interfaces.length; ++ i)
            types[i] = new Type(interfaces[i]);
        return types;
    }
}


class ToolkitDialogThread extends Thread {
    Object oDoc = null;
    String url = null;
    XMultiServiceFactory msf = null;

    public ToolkitDialogThread(XMultiServiceFactory xMSF, Object doc, String sUrl) {
	oDoc = doc;
	url = sUrl;
	msf = xMSF;
    }

    public void run() {
	XModel aModel = (XModel) UnoRuntime.queryInterface(XModel.class, oDoc);
	XController xController = aModel.getCurrentController();
	try {
	    XDispatchProvider xDispProv = (XDispatchProvider)
		UnoRuntime.queryInterface( XDispatchProvider.class, xController );
	    XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
		UnoRuntime.queryInterface(XURLTransformer.class,
					  msf.createInstance("com.sun.star.util.URLTransformer"));
	    URL[] aParseURL = new URL[1];
	    aParseURL[0] = new URL();
	    aParseURL[0].Complete = url;
	    xParser.parseStrict(aParseURL);
	    URL aURL = aParseURL[0];
	    XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
	    if( xDispatcher != null )
		xDispatcher.dispatch( aURL, null );
	} catch (com.sun.star.uno.Exception e) {
	    System.out.println("Couldn't open dialog!!!");
	    throw new StatusException( "Couldn't open dialog!!!", e );
	}
    }
}


class DialogFromFileThread extends Thread {
    String url = null;
    SOfficeFactory SOF = null;
    XMultiServiceFactory myMSF = null;

    public DialogFromFileThread(XMultiServiceFactory xMSF, String sUrl) {
	url = sUrl;
	SOF = SOfficeFactory.getFactory(xMSF);
        myMSF = xMSF;
    }

    public void run() {
	try {
            PropertyValue[] args = new PropertyValue[1];
            args[0] = new PropertyValue();
            args[0].Name = "InteractionHandler";
            args[0].Value = myMSF.createInstance(
                "com.sun.star.comp.uui.UUIInteractionHandler");

	    String testUrl= utils.getFullTestURL(url);
	    System.out.println("loading "+testUrl);
	    XComponent xDoc = SOF.loadDocument(testUrl, args);
	} catch (com.sun.star.uno.Exception e) {
	    System.out.println("Couldn't create document!!!");
	    throw new StatusException( "Couldn't create document!!!", e );
	}
    }

}

class ExecuteDialogThread extends Thread {
    XMultiServiceFactory xMSF = null;
    String serviceName = null;

    public ExecuteDialogThread(XMultiServiceFactory xMSF, String serviceName) {
        this.xMSF = xMSF;
        this.serviceName = serviceName;
    }

    public void run() {
        Object dlg = null;
        try {
            dlg = xMSF.createInstance(serviceName);
        } catch(com.sun.star.uno.Exception e) {
            throw new StatusException(Status.failed("Couldn't create service"));
        }
        XExecutableDialog execDlg = (XExecutableDialog)UnoRuntime.queryInterface
            (XExecutableDialog.class, dlg);
        execDlg.execute();
    }
}


