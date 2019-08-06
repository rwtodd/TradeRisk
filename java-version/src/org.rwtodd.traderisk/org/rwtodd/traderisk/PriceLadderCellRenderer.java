package org.rwtodd.traderisk;

import java.awt.Color;
import java.awt.Component;
import java.awt.Font;
import javax.swing.*;

class PriceLadderCellRenderer extends JLabel implements ListCellRenderer<PricePoint> {
   private final String rowFmt; // printf format for price
   private final static Color[] riskColors =  // different shades of pnl
     { 
       new Color(128,64,50),
       new Color(221,126,107),
       new Color(255,229,153),
       new Color(217,234,211),
       new Color(182,215,168),
       new Color(147,196,125),
       new Color(106,168,79),
       new Color(68,145,35)
     };

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

       int idx = 7;
       if(value.riskMultiple < -1.0) {
           idx = 0;
       } else if(value.riskMultiple < 0) {
           idx = 1;
       } else if(value.riskMultiple < 0.5) {
           idx = 2;
       } else if(value.riskMultiple < 2) {
           idx = 3;
       } else if(value.riskMultiple < 4) {
           idx = 4;
       } else if(value.riskMultiple < 6) {
           idx = 5;
       } else if(value.riskMultiple < 8) {
           idx = 6;
       }
       setBackground(PriceLadderCellRenderer.riskColors[idx]);
       return this;
   }
}

