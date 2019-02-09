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

int rwt::instrument::significant_digits () const
{
	int tmpDigits = 0;
	double tmp = tick_size;
	while (tmp - int (tmp) > 0.000001)
	{
		tmp = tmp * 10;
		tmpDigits = tmpDigits + 1;
	}
	return tmpDigits;
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
	return floor (value / tick_size + 0.5) * tick_size;
}

int rwt::instrument::ticks_apart (double p1, double p2) const
{
	return static_cast<int>(floor((p1 - p2) / tick_size + 0.5));
}

rwt::price_ladder::price_ladder () : inst(0.25, 12.5)
{
	reset (instrument{ 0.25, 12.5 }, 2500.00, 200);
	set_risk (100.0);
}

void rwt::price_ladder::recenter (double center_price)
{
	double old_sp = starting_price;
	starting_price = inst.round_to_tick (inst.ticks_from (center_price, -nsteps / 2));
	if (!transactions.empty ())
	{
		auto ticks_apart = inst.ticks_apart (starting_price, old_sp);
		std::for_each (transactions.begin (), transactions.end (), [=](rwt::transaction &t) { t.step += ticks_apart; });
		min_step += ticks_apart;
		max_step += ticks_apart;
	}
}

double rwt::price_ladder::center () const
{
	return inst.ticks_from (starting_price, nsteps / 2);
}

void rwt::price_ladder::reset (const instrument & i, double center_price, int steps)
{
	clear_transactions ();
	inst = i;
	nsteps = steps;
	recenter (center_price);
	swprintf_s (price_fmt_string, 7, L"%%.%dlf", inst.significant_digits ());
}

int rwt::price_ladder::format_price (wchar_t *const buff, size_t buffsz, double price) const
{
	return swprintf_s (buff, buffsz, price_fmt_string, price);
}


void rwt::price_ladder::clear_transactions ()
{
	transactions.clear ();
	calc_stats ();
}

#define step_2_price(s) inst.ticks_from(starting_price, nsteps - (s) - 1)

void rwt::price_ladder::calc_stats ()
{
	shares_bought = shares_sold = 0;
	avg_buy = avg_sell = 0.0;
	locked_in = 0.0;
	min_step = nsteps + 1;
	max_step = -1;

	for (const auto &t : transactions)
	{
		// track the biggest/smallest step level we've seen
		if (min_step > t.step) min_step = t.step;
		if (max_step < t.step) max_step = t.step;

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

	if( (step >= min_step) && (step <= max_step) ) 
	{
		auto trans = std::find_if (transactions.begin (), transactions.end (), [step](const auto &t) { return (t.step == step); });
		pd.shares = (trans == transactions.end ()) ? 0 : trans->amount;
	}
	else
	{
		pd.shares = 0;
	}

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

double rwt::price_ladder::risk () const { return risk_size; }
void rwt::price_ladder::set_risk (double r) { risk_size = r; }
