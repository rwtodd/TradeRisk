package org.rwtodd.traderisk;

import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.*;

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

