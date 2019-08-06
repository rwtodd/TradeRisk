package org.rwtodd.traderisk;

import javax.swing.*;

class PricePoint {
  public double price;
  public int shares;
  public double pnl;
  public double riskMultiple;
}

public class Cmd extends JFrame {
  private JList<PricePoint> lst;
  
  private void changeInstrument(Instrument i, int rows, double center, double risk) {
     // FIXME: set the model to an empty ListModel first?
     var cells = new PriceLadderCellRenderer(i);
     lst.setCellRenderer(cells);
     var model = new PriceLadder(i, rows, center, risk);
     lst.setModel(model);

     final var proto = new PricePoint();
     proto.shares = 999999999;
     proto.pnl =    999999999;
     proto.price =  11122009.25;
     proto.riskMultiple = 11122009.25;
     lst.setPrototypeCellValue(proto);
  }

  public Cmd() {
    super("Trade Risk");
    setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    lst = new JList<PricePoint>();
    changeInstrument(Instrument.ES, 200, 2500.0, 100.0);

    lst.setVisibleRowCount(25);    
    final var click = new ClickListener(lst);
    lst.addMouseListener(click); 
    setContentPane(new JScrollPane(lst));

    pack();
    click.scrollHalfway();
    setVisible(true);
  }

  public static void main(String[] args) {
    new Cmd();
  }
}
