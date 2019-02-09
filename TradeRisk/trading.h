#pragma once

#include<vector>

namespace rwt
{

class instrument
{
private:
	double tick_size;
	double tick_value;
	double incr_value; // value / size
public:
	instrument (double tsz, double tv);
	~instrument ();

	// return the price TICKS ticks from START
	double ticks_from (double start, int ticks) const;

	// return the money-value of DIFF ticks for SHARES shares
	double value_of (int shares, double diff) const;

	// round a price to a tick
	double round_to_tick (double value) const;

	// determine the number of significant digits in reporting for these ticks
	int significant_digits () const;

	// determine how many ticks apart two prices are
	int ticks_apart (double p1, double p2) const;
};

// this is the description that price_ladder can
// generate on demand
struct price_description
{
	int shares;
	double price;
	double profit;
	double risk_multiple;
};

// this is a transaction at a price, which price_ladder will
// track.
struct transaction
{
	int step;
	int amount;
};

// price_ladder remembers all the relevant information about
// each price point.  step 0 == highest price, step n == starting price
class price_ladder
{
private:
	instrument inst;
	int nsteps;
	double starting_price;
	double risk_size;
	wchar_t price_fmt_string[7]; // "%.99lf"

	// tracking the trade in play...
	std::vector<transaction> transactions;

	// these are derives stats from the transactions
	int shares_bought;
	int shares_sold;
	double locked_in;
	double avg_buy;
	double avg_sell;
	int max_step;
	int min_step;

	void calc_stats ();
public:
	price_ladder ();
	void reset (const instrument &i, double center_price, int steps);
	void recenter (double center_price);
	double risk () const;
	double center () const;
	void set_risk (double r);
	void clear_transactions ();
	inline int steps () const { return nsteps; }
	void describe (int step, price_description &pd) const;
	int format_price (wchar_t *const buff, size_t buffsz, double price) const;
	void adjust (int step, int amt);
};

}
