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


package com.sun.star.wizards.fax;
import com.sun.star.wizards.common.*;

public interface FaxWizardDialogConst
{

    public static final String OPTBUSINESSFAX_ITEM_CHANGED = "optBusinessFaxItemChanged";
    public static final String LSTBUSINESSSTYLE_ACTION_PERFORMED = null; // "lstBusinessStyleActionPerformed";
    public static final String LSTBUSINESSSTYLE_ITEM_CHANGED = "lstBusinessStyleItemChanged";
    public static final String OPTPRIVATEFAX_ITEM_CHANGED = "optPrivateFaxItemChanged";
    public static final String LSTPRIVATESTYLE_ACTION_PERFORMED = null; // "lstPrivateStyleActionPerformed";
    public static final String LSTPRIVATESTYLE_ITEM_CHANGED = "lstPrivateStyleItemChanged";
    public static final String CHKUSELOGO_ITEM_CHANGED = "chkUseLogoItemChanged";
    public static final String CHKUSEDATE_ITEM_CHANGED = "chkUseDateItemChanged";
    public static final String CHKUSECOMMUNICATIONTYPE_ITEM_CHANGED = "chkUseCommunicationItemChanged";
    public static final String LSTCOMMUNICATIONTYPE_ACTION_PERFORMED = null; // "lstCommunicationActionPerformed";
    public static final String LSTCOMMUNICATIONTYPE_ITEM_CHANGED = "lstCommunicationItemChanged";
    public static final String LSTCOMMUNICATIONTYPE_TEXT_CHANGED = "lstCommunicationTextChanged";
    public static final String CHKUSESUBJECT_ITEM_CHANGED = "chkUseSubjectItemChanged";
    public static final String CHKUSESALUTATION_ITEM_CHANGED = "chkUseSalutationItemChanged";
    public static final String LSTSALUTATION_ACTION_PERFORMED = null; // "lstSalutationActionPerformed";
    public static final String LSTSALUTATION_ITEM_CHANGED = "lstSalutationItemChanged";
    public static final String LSTSALUTATION_TEXT_CHANGED = "lstSalutationTextChanged";
    public static final String CHKUSEGREETING_ITEM_CHANGED = "chkUseGreetingItemChanged";
    public static final String LSTGREETING_ACTION_PERFORMED = null; // "lstGreetingActionPerformed";
    public static final String LSTGREETING_ITEM_CHANGED = "lstGreetingItemChanged";
    public static final String LSTGREETING_TEXT_CHANGED = "lstGreetingTextChanged";
    public static final String CHKUSEFOOTER_ITEM_CHANGED = "chkUseFooterItemChanged";
    public static final String OPTSENDERPLACEHOLDER_ITEM_CHANGED = "optSenderPlaceholderItemChanged";
    public static final String OPTSENDERDEFINE_ITEM_CHANGED = "optSenderDefineItemChanged";
    public static final String TXTSENDERNAME_TEXT_CHANGED = "txtSenderNameTextChanged";
    public static final String TXTSENDERSTREET_TEXT_CHANGED = "txtSenderStreetTextChanged";
    public static final String TXTSENDERPOSTCODE_TEXT_CHANGED = "txtSenderPostCodeTextChanged";
    public static final String TXTSENDERSTATE_TEXT_CHANGED = "txtSenderStateTextChanged";
    public static final String TXTSENDERCITY_TEXT_CHANGED = "txtSenderCityTextChanged";
    public static final String TXTSENDERFAX_TEXT_CHANGED = "txtSenderFaxTextChanged";
    public static final String OPTRECEIVERPLACEHOLDER_ITEM_CHANGED = "optReceiverPlaceholderItemChanged";
    public static final String OPTRECEIVERDATABASE_ITEM_CHANGED = "optReceiverDatabaseItemChanged";
    public static final String TXTFOOTER_TEXT_CHANGED = "txtFooterTextChanged";
    public static final String CHKFOOTERNEXTPAGES_ITEM_CHANGED = "chkFooterNextPagesItemChanged";
    public static final String CHKFOOTERPAGENUMBERS_ITEM_CHANGED = "chkFooterPageNumbersItemChanged";
    public static final String TXTTEMPLATENAME_TEXT_CHANGED = "txtTemplateNameTextChanged";
    public static final String FILETEMPLATEPATH_TEXT_CHANGED = null; // "fileTemplatePathTextChanged";
    public static final String OPTCREATEFAX_ITEM_CHANGED = "optCreateFaxItemChanged";
    public static final String OPTMAKECHANGES_ITEM_CHANGED = "optMakeChangesItemChanged";
    public static final String imageURLImageControl2 = null; //"images/ImageControl2";
    public static final String imageURLImageControl3 = null; //"images/ImageControl3";

    /* ====================================
     *                 Help IDs
     * ====================================
     */
    public final static int HID = 41119; //TODO enter first hid here 
    public static final int HIDMAIN = 41180;
    public final String OPTBUSINESSFAX_HID = HelpIds.getHelpIdString(HID + 1);
    public final String LSTBUSINESSSTYLE_HID = HelpIds.getHelpIdString(HID + 2);
    public final String OPTPRIVATEFAX_HID = HelpIds.getHelpIdString(HID + 3);
    public final String LSTPRIVATESTYLE_HID = HelpIds.getHelpIdString(HID + 4);
    public final String IMAGECONTROL3_HID = HelpIds.getHelpIdString(HID + 5);
    public final String CHKUSELOGO_HID = HelpIds.getHelpIdString(HID + 6);
    public final String CHKUSEDATE_HID = HelpIds.getHelpIdString(HID + 7);
    public final String CHKUSECOMMUNICATIONTYPE_HID = HelpIds.getHelpIdString(HID + 8);
    public final String LSTCOMMUNICATIONTYPE_HID = HelpIds.getHelpIdString(HID + 9);
    public final String CHKUSESUBJECT_HID = HelpIds.getHelpIdString(HID + 10);
    public final String CHKUSESALUTATION_HID = HelpIds.getHelpIdString(HID + 11);
    public final String LSTSALUTATION_HID = HelpIds.getHelpIdString(HID + 12);
    public final String CHKUSEGREETING_HID = HelpIds.getHelpIdString(HID + 13);
    public final String LSTGREETING_HID = HelpIds.getHelpIdString(HID + 14);
    public final String CHKUSEFOOTER_HID = HelpIds.getHelpIdString(HID + 15);
    public final String OPTSENDERPLACEHOLDER_HID = HelpIds.getHelpIdString(HID + 16);
    public final String OPTSENDERDEFINE_HID = HelpIds.getHelpIdString(HID + 17);
    public final String TXTSENDERNAME_HID = HelpIds.getHelpIdString(HID + 18);
    public final String TXTSENDERSTREET_HID = HelpIds.getHelpIdString(HID + 19);
    public final String TXTSENDERPOSTCODE_HID = HelpIds.getHelpIdString(HID + 20);
    public final String TXTSENDERSTATE_HID = HelpIds.getHelpIdString(HID + 21);
    public final String TXTSENDERCITY_HID = HelpIds.getHelpIdString(HID + 22);
    public final String TXTSENDERFAX_HID = HelpIds.getHelpIdString(HID + 23);
    public final String OPTRECEIVERPLACEHOLDER_HID = HelpIds.getHelpIdString(HID + 24);
    public final String OPTRECEIVERDATABASE_HID = HelpIds.getHelpIdString(HID + 25);
    public final String TXTFOOTER_HID = HelpIds.getHelpIdString(HID + 26);
    public final String CHKFOOTERNEXTPAGES_HID = HelpIds.getHelpIdString(HID + 27);
    public final String CHKFOOTERPAGENUMBERS_HID = HelpIds.getHelpIdString(HID + 28);
    public final String TXTTEMPLATENAME_HID = HelpIds.getHelpIdString(HID + 29);
    public final String FILETEMPLATEPATH_HID = HelpIds.getHelpIdString(HID + 30);
    public final String OPTCREATEFAX_HID = HelpIds.getHelpIdString(HID + 31);
    public final String OPTMAKECHANGES_HID = HelpIds.getHelpIdString(HID + 32);
    public final String IMAGECONTROL2_HID = HelpIds.getHelpIdString(HID + 33);
}
  
