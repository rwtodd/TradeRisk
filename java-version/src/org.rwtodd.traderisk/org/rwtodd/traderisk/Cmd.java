package org.rwtodd.traderisk;

import javax.swing.*;

class PricePoint {
  public double price;
  public int shares;
  public double pnl;
  public double riskMultiple;
}

public class Cmd extends JFrame {
  private final JList<PricePoint> lst;
  private final ClickListener click;

  /** a helper method to dig our PriceLadder out of the list.
   * @return the price ladder.
   */
  private PriceLadder getLadder() {
      return (PriceLadder)lst.getModel();
  }
  
  private void changeInstrument(Instrument i, int rows, double center, double risk) {
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

  private void buildMenus() {
     final var bar = new JMenuBar();
     final var act = new JMenu("Actions");
     act.setMnemonic('A');

     /* ----------------------------------------------------- */
     final var clr = new JMenuItem("Clear");
     clr.setMnemonic('l');
     clr.addActionListener( (e) -> {
        getLadder().clearTransactions(); 
     });

     /* ----------------------------------------------------- */
     final var rsk = new JMenuItem("Set Risk");
     rsk.setMnemonic('R');
     rsk.addActionListener( (e) -> {
        final var ladder = getLadder();
        final var result = JOptionPane.showInputDialog(this, "Set risk amount", ladder.getRisk());
        try {
          ladder.setRisk(Double.parseDouble(result));
        } catch(Exception problem) { /* ignore problems with user input */ }
     });

     /* ----------------------------------------------------- */
     final var cent = new JMenuItem("Recenter");
     cent.setMnemonic('c');
     cent.addActionListener( (e) -> {
        final var ladder = getLadder();
        final double init = ladder.getElementAt(lst.getFirstVisibleIndex()).price;
        final var result = JOptionPane.showInputDialog(this, "Set center price", init);
        try {
           ladder.recenter(Double.parseDouble(result));
           click.scrollHalfway();
        } catch(Exception problem) { /* ignore problems with bad input */ }
     });

     /* ----------------------------------------------------- */
     final var inst = new JMenuItem("Instrument");
     inst.setMnemonic('I');
     inst.addActionListener( (e) -> {
        final var dlg = new InstrumentDialog(this, getLadder());   
        dlg.setVisible(true);
        if(dlg.gotResponse()) {
           changeInstrument(dlg.getInstrument(),
                            dlg.getRows(),
                            dlg.getCenter(),
                            dlg.getRisk());
           click.scrollHalfway();
        }
     });

     /* ----------------------------------------------------- */
     bar.add(act);
     act.add(clr);
     act.add(rsk);
     act.add(cent);
     act.add(inst);
     setJMenuBar(bar);
  }

  public Cmd() {
    super("Trade Risk");
    setDefaultCloseOperation(WindowConstants.EXIT_ON_CLOSE);
    try {
        setIconImage(javax.imageio.ImageIO.read(this.getClass().getResource("/icon.gif")));
     } catch (java.io.IOException exc) {
        exc.printStackTrace();
    }              

    lst = new JList<PricePoint>();
    changeInstrument(Instrument.ES, 200, 2500.0, 100.0);

    lst.setVisibleRowCount(25);    
    click = new ClickListener(lst);
    lst.addMouseListener(click); 
    setContentPane(new JScrollPane(lst));

    buildMenus();
    pack();
    click.scrollHalfway();
    setVisible(true);
  }

  public static void main(String[] args) {
    new Cmd();
  }
}
