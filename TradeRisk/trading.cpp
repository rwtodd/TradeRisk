#include "stdafx.h"
#include "trading.h"

rwt::instrument::instrument (double tsz, double tv)
	: tick_size (tsz), tick_value (tv), incr_value (tv / tsz)
{
}


rwt::instrument::~instrument ()
{
}

double rwt::instrument::ticks_from (double start, int ticks) const
{
	return start + (ticks * tick_size);
}

double
rwt::instrument::value_of (int shares, double diff) const
{
	return (shares * diff * incr_value);
}

double rwt::instrument::round_to_tick (double value) const
{
	return floor (value / tick_size) * tick_size;
}

rwt::price_ladder::price_ladder () : inst(0.25, 12.5)
{
	reset (instrument{ 0.25, 12.5 }, 2500.00, 200, 25.00);
}

void rwt::price_ladder::reset (const instrument & i, double start_at, int steps, double risk)
{
	inst = i;
	risk_size = risk;
	nsteps = steps;
	starting_price = inst.round_to_tick (start_at);
}

#define step_2_price(s) inst.ticks_from(starting_price, nsteps - (s) - 1)

void rwt::price_ladder::describe (int step, price_description &pd) const
{
	pd.price = step_2_price (step);
	pd.profit = inst.value_of (1, pd.price - step_2_price(20));
	pd.shares = 1;
	pd.risk_multiple = pd.profit / risk_size;
}

// adjust the open position ... +amount == buy, -amount == sell
void rwt::price_ladder::adjust (int step, int amount)
{
	/* do nothing for now. */
}
