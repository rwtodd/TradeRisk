package org.rwtodd.traderisk;

import java.awt.Component;
import java.awt.Font;
import javax.swing.*;

class PriceLadderCellRenderer extends JLabel implements ListCellRenderer<PricePoint> {
   private final String rowFmt; // printf format for price

   PriceLadderCellRenderer(Instrument inst) {
      super();
      rowFmt = String.format("%% 4d  %%.%df  %%5.2f  %%5.1f", 
                               inst.significantDigits());
      setFont(new Font(Font.MONOSPACED, Font.PLAIN, 14));
      setOpaque(true); // so that the background is drawn
   }

   @Override
   public Component getListCellRendererComponent(
      JList<? extends PricePoint> list,
      PricePoint value, 
      int index,
      boolean isSelected,
      boolean cellHasFocus) {
       final String repr = String.format(rowFmt, value.shares, 
                                                 value.price, 
                                                 value.pnl,
                                                 value.riskMultiple);
       setText(repr);
       if(value.riskMultiple < 0) {
          setBackground(java.awt.Color.RED);
       } else {
          setBackground(java.awt.Color.GREEN);
       }
       return this;
   }
}

