package org.rwtodd.traderisk;

import java.awt.event.MouseListener;
import java.awt.event.MouseEvent;
import javax.swing.*;

class ClickListener implements MouseListener {
  private final JList<PricePoint> parent;

  ClickListener(JList<PricePoint> l) {
      parent = l;
      scrollHalfway();
  } 

  /** Scroll to the middle of the list, unconditionally */
  void scrollHalfway() {
     try {
          final var vscroll = ((JScrollPane)parent.getParent().getParent()).getVerticalScrollBar();
          vscroll.setValue((vscroll.getMaximum() - 
                            vscroll.getMinimum() - 
                            vscroll.getVisibleAmount()) /2);
     } catch(Exception e) { 
        /* we might get an exception if the list is not
           yet embedded in a scrollpane, etc.  Just ignore
           the exception and move on. */
     }
  }

  public void mouseClicked(MouseEvent e)	 {
      final int index = parent.locationToIndex(e.getPoint());
      final var pl = (PriceLadder)parent.getModel();
      if(SwingUtilities.isLeftMouseButton(e)) {
          pl.adjustTransaction(index, 1); 
      } else if(SwingUtilities.isRightMouseButton(e)) {
          pl.adjustTransaction(index, -1);
      } else {
          // any other mouse button recenters
          final var centerPrice = pl.getElementAt(index).price;
          scrollHalfway();
          pl.recenter(centerPrice);
      }
  }

  public void mouseEntered(MouseEvent e)	{  }
  public void mouseExited(MouseEvent e)  { }	
  public void mousePressed(MouseEvent e)	{ }
  public void mouseReleased(MouseEvent e)	 { }
}

