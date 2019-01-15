#include "stdafx.h"
#include "trading.h"

#include <algorithm>

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
	transactions.clear ();
	calc_stats ();
}

#define step_2_price(s) inst.ticks_from(starting_price, nsteps - (s) - 1)

void rwt::price_ladder::calc_stats ()
{
	shares_bought = shares_sold = 0;
	avg_buy = avg_sell = 0.0;
	locked_in = 0.0;

	for (const auto &t : transactions)
	{
		const double price = step_2_price (t.step);
		if (t.amount >= 0)
		{
			shares_bought += t.amount;
			avg_buy += (t.amount * price);
		}
		else
		{
			shares_sold -= t.amount;
			avg_sell -= (t.amount * price);
		}
	}

	if(shares_bought > 0) avg_buy /= shares_bought;
	if(shares_sold > 0) avg_sell /= shares_sold;
	
	int lock_amt = min (shares_sold, shares_bought);
	locked_in = inst.value_of (lock_amt, avg_sell - avg_buy);
}

void rwt::price_ladder::describe (int step, price_description &pd) const
{
	pd.price = step_2_price (step);
	int position = shares_bought - shares_sold;
	if (position >= 0)
	{
		pd.profit = locked_in + inst.value_of (position, pd.price - avg_buy);
	}
	else
	{
		pd.profit = locked_in + inst.value_of (-position, avg_sell - pd.price);
	}

	// TODO... maybe improve this by not doing the find if the step is out of range of the min/max transaction
	auto trans = std::find_if (transactions.begin (), transactions.end (), [step](const auto &t) { return (t.step == step); });
	pd.shares = (trans == transactions.end ()) ? 0 : trans->amount;

	pd.risk_multiple = pd.profit / risk_size;
}

// adjust the open position ... +amount == buy, -amount == sell
void rwt::price_ladder::adjust (int step, int amount)
{
	auto trans = std::find_if (transactions.begin (), transactions.end (), [step](const auto &t) { return (t.step == step); });
	if (trans != transactions.end ())
	{
		trans->amount += amount;
		if (trans->amount == 0)
		{
			transactions.erase (trans);
		}
	}
	else
	{
		transactions.push_back (transaction{ step, amount });
	}
	calc_stats ();
}