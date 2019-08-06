package org.rwtodd.traderisk;

import javax.swing.*;
import java.awt.GridLayout;
import java.awt.BorderLayout;

class InstrumentDialog extends JDialog {
  private boolean canceled;

  private JTextField uiRows;
  private int nRows;

  private JTextField uiRisk;
  private double risk;

  private JTextField uiCenter;
  private double center;

  private JComboBox<Instrument> uiInst;
  private Instrument inst;

  InstrumentDialog(JFrame parent, PriceLadder ladder) {
      super(parent, "Instrument Setup", true);
      canceled = true;

      // set up the buttons at the bottom of the dialog
      final var btns = new JPanel();
      final var okBtn = new JButton("Ok");
      okBtn.addActionListener( (e) -> {
           readValues();
           setVisible(false);
      });
      btns.add(okBtn);

      final var cBtn = new JButton("Cancel");
      cBtn.addActionListener( (e) -> { setVisible(false); } );
      btns.add(cBtn);

      add(btns, BorderLayout.SOUTH);

      // fetch initial values from the price ladder
      nRows = ladder.getSize();
      risk = ladder.getRisk(); 
      center = ladder.getCenter();
      inst = ladder.getInstrument();

      // set up verifiers for input
      var posDbl = new PositiveDoubleVerifier(okBtn);
      var posInt = new PositiveIntegerVerifier(okBtn);

      final var grid = new JPanel();
      grid.setLayout(new GridLayout(4,2));

      grid.add(new JLabel("Instrument:"));
      uiInst = new JComboBox<>(Instrument.values());
      uiInst.setSelectedItem(inst);
      grid.add(uiInst); 

      grid.add(new JLabel("Center Price:"));
      uiCenter = new JTextField(String.valueOf(center));
      uiCenter.setInputVerifier(posDbl);
      grid.add(uiCenter);

      grid.add(new JLabel("Rows:"));
      uiRows = new JTextField(String.valueOf(nRows));
      uiRows.setInputVerifier(posInt);
      grid.add(uiRows);

      grid.add(new JLabel("Risk $:"));
      uiRisk = new JTextField(String.valueOf(risk));
      uiRisk.setInputVerifier(posDbl);
      grid.add(uiRisk);

      grid.setBorder(BorderFactory.createEmptyBorder(10, 10, 10, 10));
      add(grid, BorderLayout.CENTER);

      pack();
  }

  private void readValues() {
      try {
         inst = uiInst.getItemAt(uiInst.getSelectedIndex());
         nRows = Integer.parseInt(uiRows.getText());
         center = Double.parseDouble(uiCenter.getText());
         risk = Double.parseDouble(uiRisk.getText());
         canceled = false;
      } catch(Exception problem) { 
         /* if there is a problem, at least don't 
            screw up the ladder */
         canceled = true; 
      }
  }

  boolean gotResponse() { return !canceled; }
  int getRows() { return nRows; }
  Instrument getInstrument() { return inst; }
  double getRisk() { return risk; }
  double getCenter() { return center; }
}
