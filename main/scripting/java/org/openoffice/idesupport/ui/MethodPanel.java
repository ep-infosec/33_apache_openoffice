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



package org.openoffice.idesupport.ui;

import java.io.File;
import java.util.Vector;
import java.util.ArrayList;

import java.lang.reflect.Method;
import java.lang.reflect.Modifier;

import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JList;
import javax.swing.JTable;
import javax.swing.table.AbstractTableModel;
import javax.swing.JLabel;
import java.awt.BorderLayout;
import java.net.URL;
import java.net.URLClassLoader;
import java.net.MalformedURLException;

import com.sun.star.script.framework.container.ScriptEntry;
import org.openoffice.idesupport.MethodFinder;
import org.openoffice.idesupport.ExtensionFinder;
import org.openoffice.idesupport.JavaFinder;

public class MethodPanel extends JPanel {

    private File basedir;
    private Vector classpath;
    private final static String FIRST_PARAM =
        "drafts.com.sun.star.script.framework.runtime.XScriptContext";

    // private JTable table;
    // private MethodTableModel model;
    private JList list;
    private ScriptEntry[] values;

    public MethodPanel(File basedir, Vector classpath, String language) {
        this.basedir = basedir;
        this.classpath = classpath;

        initValues(language);
        initUI();
    }

    public void reload(File basedir, Vector classpath, String language) {
        this.basedir = basedir;
        this.classpath = classpath;

        initValues(language);
        list.setListData(values);
    }

    public ScriptEntry[] getSelectedEntries() {
        Object[] selections = list.getSelectedValues();
        ScriptEntry[] entries = new ScriptEntry[selections.length];

        for (int i = 0; i < selections.length; i++) {
            entries[i] = (ScriptEntry)selections[i];
        }

        return entries;
    }

    private void initUI() {
        JLabel label = new JLabel("Available Methods:");
        // table = new JTable(model);
        list = new JList(values);

        JScrollPane pane = new JScrollPane(list);
        label.setLabelFor(pane);
        
        BorderLayout layout = new BorderLayout();
        setLayout(layout);
        layout.setVgap(5);
        
        add(label, BorderLayout.NORTH);
        add(pane, BorderLayout.CENTER);
    }

    private void initValues(String language) {
        MethodFinder finder;

        if (language == null)
            finder = JavaFinder.getInstance(classpath);
        else if (language.toLowerCase().equals("beanshell"))
            finder = new ExtensionFinder(language, new String[] {".bsh"});
        else
            finder = JavaFinder.getInstance(classpath);

        values = finder.findMethods(basedir);
    }

    /*
    private class MethodTableModel extends AbstractTableModel {
        final String[] columnNames = {"Method", 
                                      "Language"};

        private Vector methods;
        private int nextRow;

        public MethodTableModel() {
            methods = new Vector(11);
        }

        public int getColumnCount() {
            return columnNames.length;
        }
        
        public int getRowCount() {
            return methods.size();
        }

        public String getColumnName(int col) {
            return columnNames[col];
        }

        public void add(ScriptEntry entry) {
            methods.addElement(entry);
            fireTableRowsInserted(nextRow, nextRow);
            nextRow++;
        }

        public void remove(int row) {
            methods.removeElementAt(row);
            fireTableRowsDeleted(row, row);
            nextRow--;
        }
        
        public void removeAll() {
            methods.removeAllElements();
            fireTableRowsDeleted(0, nextRow);
            nextRow = 0;
        }

        public Object getValueAt(int row) {
            return methods.elementAt(row);
        }

        public Object getValueAt(int row, int col) {
            String result = "";
            ScriptEntry entry;

            entry = (ScriptEntry)methods.elementAt(row);

            if (col == 0)
                result = entry.getLanguageName();
            else if (col == 1)
                result = entry.getLanguage();

            return result;
        }

        public boolean isCellEditable(int row, int col) {
            return false;
        }
    }
    */
}
