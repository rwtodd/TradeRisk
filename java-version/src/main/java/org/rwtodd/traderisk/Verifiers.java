package org.rwtodd.traderisk;

import javax.swing.*;
import javax.swing.text.JTextComponent;
import javax.swing.border.CompoundBorder;
import java.awt.Color;

/** wraps a failing component in a red border */
abstract class ReddeningInputVerifier extends InputVerifier {
  private boolean red;
  private JComponent toDisable;

  ReddeningInputVerifier(JComponent disable) {
     red = false;
     toDisable = disable;
  }

  @Override
  public boolean shouldYieldFocus(JComponent source, JComponent target) {
     if(verify(source)) {
        if (red) {
          red = false;
          toDisable.setEnabled(true); 
          source.setBorder(
            ((CompoundBorder)source.getBorder()).getInsideBorder());
        }
        return true;
     } else {
        if(!red) {
          red = true;
          toDisable.setEnabled(false); 
          source.setBorder(
            BorderFactory.createCompoundBorder(
                 BorderFactory.createLineBorder(Color.RED,2),
                 source.getBorder()));
        }
        return false;
     }
  }
}

class PositiveIntegerVerifier extends ReddeningInputVerifier {
  PositiveIntegerVerifier(JComponent disable) {
    super(disable);
  }

  @Override
  public boolean verify(JComponent input) {
      var txt = ((JTextComponent)input).getText();     
      try {
         return (Integer.parseInt(txt) > 0);
      } catch(NumberFormatException nfe) {
         return false;
      }
  }
}

class PositiveDoubleVerifier extends ReddeningInputVerifier {
  PositiveDoubleVerifier(JComponent disable) {
    super(disable);
  }

  @Override
  public boolean verify(JComponent input) {
      var txt = ((JTextComponent)input).getText();     
      try {
         return (Double.parseDouble(txt) > 0.0);
      } catch(NumberFormatException nfe) {
         return false;
      }
  }

}
