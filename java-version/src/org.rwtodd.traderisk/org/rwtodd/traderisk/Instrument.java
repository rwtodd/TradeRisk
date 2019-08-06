package org.rwtodd.traderisk;

enum Instrument {
  ES(0.25,12.5),
  E6(0.00005, 6.25),
  CL(0.01, 10.0),
  STOCK(0.01, 0.01);

  /** The price-ladder increment of a tick */
  private final double tickSize;

  /** The dollar-value of a tick */
  private final double tickValue;

  /** the value / size of a tick */
  private final double incrValue;

  Instrument(final double ts, final double tv) {
     tickSize = ts;
     tickValue = tv;
     incrValue = tv / ts;
  }

  /** Calculate the price {@code ticks} ticks from {@code start}. 
   *
   *  @param start the starting price.
   *  @param ticks how many ticks away to move.
   *  @return the price {@code ticks} ticks from {@code start}. 
   */
  double ticksFrom(final double start, final int ticks) {
     return start + (ticks * tickSize); 
  }

  /** Calculate the money-value of a price move.
   *
   * @param shares the size of the position.
   * @param diff the size of the price move.
   * @return the money-value of the move.
   */
  double valueOf(int shares, double diff) {
     return shares * diff * incrValue;
  }

  /** Round a price to the nearest tick for this instrument.
   * 
   * @param price the rough price.
   * @return the rounded price.
   */
  double roundToTick(double price) {
      return Math.floor(price / tickSize + 0.5) * tickSize;
  }

  /** Determine the number of significant digits in prices.
   * Different instruments will have different numbers of
   * relevant digits to report.
   *
   * @param The computed number of relevant digits.
   */
  int significantDigits() {
     int tmpDigits = 0;
     double tmp = tickSize;
     while( (tmp - (int)tmp) > 0.000001 ) {
        tmp *= 10;
        ++tmpDigits;
     }
     return tmpDigits;
  }

  /** Determines how many ticks apart two prices are.
   *
   * @param p1 the first price
   * @param p2 the second price
   * @return the price difference, in ticks.
   */
   int ticksApart(double p1, double p2) {
      return (int)Math.floor( (p1 - p2) / tickSize + 0.5 );
   }
}
