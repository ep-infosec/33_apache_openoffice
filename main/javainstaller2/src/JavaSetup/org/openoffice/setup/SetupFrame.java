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



package org.openoffice.setup;
import java.awt.BorderLayout;
import java.awt.CardLayout;
import java.awt.ComponentOrientation;
import java.awt.Dimension;
import java.awt.Insets;
import java.awt.event.WindowAdapter;
import java.io.File;
import javax.swing.Box;
import javax.swing.BoxLayout;
import javax.swing.Icon;
import javax.swing.JButton;
import javax.swing.JDialog;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JSeparator;
import javax.swing.border.EmptyBorder;

public class SetupFrame extends WindowAdapter {

    String StringPrevious;
    String StringNext;
    String StringCancel;
    String StringFinish;
    String StringHelp;
    String StringAppTitle;
    
    Icon   IconStarOffice;

    public static final String ACTION_NEXT      = "ActionNext";
    public static final String ACTION_PREVIOUS  = "ActionPrevious";
    public static final String ACTION_CANCEL    = "ActionCancel";
    public static final String ACTION_HELP      = "ActionHelp";
    public static final String ACTION_DETAILS   = "ActionDetails";
    public static final String ACTION_STOP      = "ActionStop";

    public static final int CODE_OK         = 0;
    public static final int CODE_CANCEL     = 1;
    public static final int CODE_ERROR      = 2;
     
    public static final int BUTTON_NEXT     = 1;
    public static final int BUTTON_PREVIOUS = 2;
    public static final int BUTTON_CANCEL   = 3;
    public static final int BUTTON_HELP     = 4;

    private JButton     mNextButton;
    private JButton     mPreviousButton;
    private JButton     mCancelButton;
    private JButton     mHelpButton;

    private JDialog     mDialog;

    private JPanel      mCardPanel;
    private CardLayout  mCardLayout;
    
    private SetupActionListener mActionListener;
    private DeckOfPanels        mDeck; 

    public SetupFrame() {

        StringPrevious   = ResourceManager.getString("String_Previous");
        StringNext       = ResourceManager.getString("String_Next");
        StringCancel     = ResourceManager.getString("String_Cancel");
        StringFinish     = ResourceManager.getString("String_Finish");
        StringHelp       = ResourceManager.getString("String_Help");
        
        InstallData data = InstallData.getInstance();
        if ( data.isInstallationMode() ) {
            StringAppTitle   = ResourceManager.getString("String_ApplicationTitle");        
        } else {
            StringAppTitle   = ResourceManager.getString("String_ApplicationTitleUninstallation");            
        }
        
        // The setup icon has to be flexible for customization, not included into the jar file
        File iconFile = data.getInfoRoot("images");
        iconFile = new File(iconFile, "Setup.gif");
        IconStarOffice   = ResourceManager.getIconFromPath(iconFile);
           
        mActionListener = new SetupActionListener(this);
        mDeck           = new DeckOfPanels(); 
        
        mDialog = new JDialog();
        mDialog.setTitle(StringAppTitle);
        initFrame();
    }
    
    public void addPanel(PanelController panel, String name) {
        mCardPanel.add(panel.getPanel(), name);
        panel.setSetupFrame(this);
        mDeck.addPanel(panel, name);
    }  
   
    public PanelController getCurrentPanel() {
        return mDeck.getCurrentPanel();
    }
 
    public void setCurrentPanel(String name, boolean ignoreRepeat, boolean isNext) {
        if (name == null)
            close(CODE_ERROR);
        
        PanelController panel = mDeck.getCurrentPanel();
        boolean repeatDialog = false;
        if (panel != null) {
            repeatDialog = panel.afterShow(isNext);
            if ( isNext ) {
                name = panel.getNext();   // afterShow() could have changed the "next" dialog
            }
            if ( ignoreRepeat ) {
                repeatDialog = false;
            }
        }
            
        if ( repeatDialog ) {
            name = panel.getName();
        }
        
        panel = mDeck.setCurrentPanel(name);
        if (panel != null)
        {
            setButtonsForPanel(panel);
            panel.beforeShow();
            mCardLayout.show(mCardPanel, name);
            panel.duringShow();
        }
    }

    void setButtonsForPanel(PanelController panel) {

        setButtonText(StringCancel,     BUTTON_CANCEL);
        setButtonText(StringHelp,       BUTTON_HELP);
        setButtonText(StringPrevious,   BUTTON_PREVIOUS);
        // setButtonEnabled((panel.getPrevious() != null), BUTTON_PREVIOUS);
        // setButtonEnabled((panel.getNext() != null),     BUTTON_CANCEL);
        if (panel.getNext() == null) {   
            setButtonText(StringFinish, BUTTON_NEXT);
        } else {
            setButtonText(StringNext,   BUTTON_NEXT);
        }
    }

    public void setButtonText(String text, int button) {
        switch (button) {
            case BUTTON_NEXT:       mNextButton.setText(text);        break;
            case BUTTON_PREVIOUS:   mPreviousButton.setText(text);    break;
            case BUTTON_CANCEL:     mCancelButton.setText(text);      break;
            case BUTTON_HELP:       mHelpButton.setText(text);        break;
        }
    }

    public void setButtonSelected(int button) {
        switch (button) {
            case BUTTON_NEXT:       mNextButton.grabFocus();     break;
            case BUTTON_PREVIOUS:   mPreviousButton.grabFocus(); break;
            case BUTTON_CANCEL:     mCancelButton.grabFocus();   break;
            case BUTTON_HELP:       mHelpButton.grabFocus();     break;
        }
    }

    public void setButtonEnabled(boolean enabled, int button) {
        switch (button) {
            case BUTTON_NEXT:       mNextButton.setEnabled(enabled);     break;
            case BUTTON_PREVIOUS:   mPreviousButton.setEnabled(enabled); break;
            case BUTTON_CANCEL:     mCancelButton.setEnabled(enabled);   break;
            case BUTTON_HELP:       mHelpButton.setEnabled(enabled);     break;
        }
    }

    public void removeButtonIcon(int button) {
        switch (button) {
            case BUTTON_NEXT:       mNextButton.setIcon(null);           break;
            case BUTTON_PREVIOUS:   mPreviousButton.setIcon(null);       break;
            case BUTTON_CANCEL:     mCancelButton.setIcon(null);         break;
            case BUTTON_HELP:       mHelpButton.setIcon(null);           break;
        }
    }
    
    public SetupActionListener getSetupActionListener() {
        return mActionListener;
    }
            
    void close(int code) {
        mDialog.dispose();
    }
    
    public JDialog getDialog() {
        return mDialog;
    }
    
    public int showFrame() {
        mDialog.pack();
        mDialog.setLocationRelativeTo(null);
        mDialog.setModal(true);    
        mDialog.setResizable(false);
        // mDialog.setMinimumSize(new Dimension(679, 459));
        mDialog.setVisible(true);
        // System.err.println("Width: " + mDialog.getWidth() + ", Height: " + mDialog.getHeight());

        return 0;
    }
    
    private void initFrame() {

        mDialog.getContentPane().setLayout(new BorderLayout());
        
        mCardLayout = new CardLayout(); 
        mCardPanel  = new JPanel();
        mCardPanel.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));       
        mCardPanel.setLayout(mCardLayout);
        
        mPreviousButton = new JButton();
        mNextButton     = new JButton();
        mCancelButton   = new JButton();
        mHelpButton     = new JButton();       
 
        mPreviousButton.setHorizontalTextPosition(JButton.RIGHT);
        mNextButton.setHorizontalTextPosition(JButton.LEFT);
        
        mPreviousButton.setIcon(ResourceManager.getIcon("Icon_Previous"));
        mNextButton.setIcon(ResourceManager.getIcon("Icon_Next"));
        
        mPreviousButton.setActionCommand(ACTION_PREVIOUS);
        mNextButton.setActionCommand(ACTION_NEXT);
        mCancelButton.setActionCommand(ACTION_CANCEL);
        mHelpButton.setActionCommand(ACTION_HELP);

        mPreviousButton.addActionListener(mActionListener);
        mNextButton.addActionListener(mActionListener);
        mCancelButton.addActionListener(mActionListener);
        mHelpButton.addActionListener(mActionListener);
        
        InstallData data = InstallData.getInstance();

        if (data.useRtl()) {
            mPreviousButton.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
            mNextButton.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
            mCancelButton.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
            mHelpButton.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
        }
        
        Box ButtonBox   = new Box(BoxLayout.X_AXIS);
        ButtonBox.setBorder(new EmptyBorder(new Insets(5, 10, 5, 10)));       
        ButtonBox.add(mPreviousButton);
        ButtonBox.add(Box.createHorizontalStrut(10));
        ButtonBox.add(mNextButton);
        ButtonBox.add(Box.createHorizontalStrut(30));
        ButtonBox.add(mCancelButton);
        ButtonBox.add(Box.createHorizontalStrut(10));
        ButtonBox.add(mHelpButton);
        if (data.useRtl()) {
            ButtonBox.setComponentOrientation(ComponentOrientation.RIGHT_TO_LEFT);
        }
        
        JPanel ButtonPanel = new JPanel();
        JSeparator Separator = new JSeparator();
        ButtonPanel.setLayout(new BorderLayout());
        ButtonPanel.setPreferredSize(new Dimension(612, 44));
        ButtonPanel.add(Separator, BorderLayout.NORTH);
        ButtonPanel.add(ButtonBox, java.awt.BorderLayout.EAST);
 
        JPanel IconPanel = new JPanel();
        JLabel Icon = new JLabel();
        Icon.setIcon(IconStarOffice);
//        IconPanel.setPreferredSize(new Dimension(142, 372));
//        IconPanel.setBorder(new EmptyBorder(new Insets(10, 10, 10, 10)));
        IconPanel.setLayout(new BorderLayout());
        IconPanel.add(Icon);
        
        mDialog.getContentPane().add(ButtonPanel, java.awt.BorderLayout.SOUTH);
        mDialog.getContentPane().add(mCardPanel, java.awt.BorderLayout.CENTER);
        mDialog.getContentPane().add(IconPanel, java.awt.BorderLayout.WEST); 
    }

}
