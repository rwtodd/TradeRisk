package org.rwtodd.traderisk;

import java.awt.Component;
import java.awt.Font;
import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.event.ListDataListener;
import javax.swing.*;

class PricePoint {
  public double price;
  public int shares;
  public double pnl;
  public double riskMultiple;
}

class PriceLadderCellRenderer extends JLabel implements ListCellRenderer<PricePoint> {
   private final String rowFmt; // printf format for price

   PriceLadderCellRenderer(Instrument inst) {
      super();
      rowFmt = String.format("%% 4d  %%.%df  %%5.2f %%5.1f", 
                               inst.significantDigits());
      setFont(new Font(Font.MONOSPACED, Font.PLAIN, 14));
      setOpaque(true); // so that the background is drawn
   }
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


class ClickListener implements MouseListener {
  private final JList<PricePoint> parent;

  ClickListener(JList<PricePoint> l) {
      parent = l;
  } 

  public void	mouseClicked(MouseEvent e)	 {
      final int index = parent.locationToIndex(e.getPoint());
      final var pl = (PriceLadder)parent.getModel();
      if(SwingUtilities.isLeftMouseButton(e)) {
          pl.adjustTransaction(index, 1); 
      } else if(SwingUtilities.isRightMouseButton(e)) {
          pl.adjustTransaction(index, -1);
      } 
  }

  public void	mouseEntered(MouseEvent e)	{  }
  public void	mouseExited(MouseEvent e)  { }	
  public void	mousePressed(MouseEvent e)	{ }
  public void	mouseReleased(MouseEvent e)	 { }
}

public class Cmd extends JFrame {
  public Cmd() {
    super("Virtual List Example");
    setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    var lst = new JList<PricePoint>(new PriceLadder(Instrument.ES, 200, 2500.0, 100.0));
    lst.setCellRenderer(new PriceLadderCellRenderer(Instrument.ES));
    lst.setVisibleRowCount(25);    
    lst.addMouseListener(new ClickListener(lst)); 

    final var proto = new PricePoint();
    proto.shares = -99999;
    proto.pnl = -222009;
    proto.price = 11122009.25;
    proto.riskMultiple = 11122009.25;
    lst.setPrototypeCellValue(proto);
    setContentPane(new JScrollPane(lst));
    pack();
    setVisible(true);
  }

  public static void main(String[] args) {
    new Cmd();
  }
}
