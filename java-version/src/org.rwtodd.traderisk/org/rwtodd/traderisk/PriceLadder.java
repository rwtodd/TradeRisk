package org.rwtodd.traderisk;

import javax.swing.*;
import java.util.ArrayList;

/** PriceLadder is our model of a trade on the price ladder.  It
 * is also a model for a JList. 
 *
 * @author Richard Todd
 */
class PriceLadder extends AbstractListModel<PricePoint> {

  // these are set when the model is created, and a new PriceLadder
  // will have to be generated if the user switches them
  private final Instrument inst; // es, 6e, cl, etc.
  private final int nsteps; // how many prices we are tracking

  private class Transaction {
    int step;   // which step on the ladder 
    int amount; // how many shares?
  }

  // We track the components of the trade in play here:
  ArrayList<Transaction> transactions;
  int sharesBought;
  int sharesSold;
  double avgBuy;
  double avgSell;
  double lockedIn;
  int minStep;
  int maxStep;

  // these can be changed at will by the user without disrupting
  // a transaction.
  private double riskSize; // how much are we risking
  private double highPrice; // the highest price we can display

  // This is a token PricePoint object, used to tell a JList
  // what to display.
  PricePoint generated;

  PriceLadder(Instrument i, int steps, double center, double risk) {
      inst = i;
      nsteps = steps;
      riskSize = risk;
      generated = new PricePoint();
      transactions = new ArrayList<>();
      calcStats();

      recenter(center);
  }
  
  /** Recalculate our derived statistics about open transacitons. */
  private void calcStats() {
	sharesBought = 0;
    sharesSold = 0;
	avgBuy = 0.0; 
    avgSell = 0.0;
	lockedIn = 0.0;
	minStep = nsteps + 1;
	maxStep = -1;

	for (var t : transactions)
	{
		// track the biggest/smallest step level we've seen
		if (minStep > t.step) minStep = t.step;
		if (maxStep < t.step) maxStep = t.step;

		final double price = inst.ticksFrom(highPrice, t.step);
		if (t.amount >= 0)
		{
			sharesBought += t.amount;
			avgBuy += (t.amount * price);
		}
		else
		{
			sharesSold -= t.amount;
			avgSell -= (t.amount * price);
		}
	}

	if(sharesBought > 0) avgBuy /= sharesBought;
	if(sharesSold > 0) avgSell /= sharesSold;
	
	final int lockAmt = Math.min(sharesSold, sharesBought);
	lockedIn = inst.valueOf (lockAmt, avgSell - avgBuy);
  }

  /** Adjust the center of the price scale.  Take care to adjust
    * any open transactions so that they continue to fall on the
    * correct tick after the shift.
    *
    * @param price the new center price.
    */
  void recenter(double price) {
      double oldHP = highPrice;
      highPrice = inst.roundToTick(inst.ticksFrom(price, nsteps/2));
      if(!transactions.isEmpty()) {
         int ticksApart = inst.ticksApart(highPrice, oldHP);
         for(var t : transactions) {
            t.step += ticksApart;
         }
         minStep += ticksApart;
         maxStep += ticksApart;
      }
  }

  /** locate a transaction by the step it sits in.
   *
   * @param step the step where we want to find a transation.
   * @return the transaction, or {@code null} if one isn't found.
   */
  private Transaction locate(int step) {
      for(var t : transactions) {
         if(t.step == step) return t;
      }
      return null;
  }

  @Override
  public PricePoint getElementAt(int index) {
      generated.price = inst.ticksFrom(highPrice, index);
      final int position = sharesBought - sharesSold;
      if(position >= 0) {
         generated.pnl =  lockedIn + inst.valueOf(position, generated.price - avgBuy);
      } else {
         generated.pnl =  lockedIn + inst.valueOf(-position, avgSell - generated.price);
      }

      if( (index >= minStep) && (index <= maxStep) ) {
        final var transaction = locate(index); 
        generated.shares = (transaction == null) ? 0 : transaction.amount;
      } else {
        generated.shares = 0;
      }
 
      generated.riskMultiple = generated.pnl / riskSize;
      return generated;
  }

  public int getSize() { return nsteps; }

  void adjustTransaction(int step, int amount) {
     var transaction = locate(step); 
     if(transaction != null) {
        transaction.amount += amount;
        if(transaction.amount == 0) {
            transactions.remove(transaction);
        }
     } else {
        transaction = new Transaction();
        transaction.step = step;
        transaction.amount = amount;
        transactions.add(transaction);
     }
     calcStats();
  }

  double getRisk() { return riskSize; }
  void setRisk(double nrisk) { riskSize = nrisk; }
}

