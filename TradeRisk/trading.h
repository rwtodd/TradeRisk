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

	double ticks_from (double start, int ticks) const;
	double value_of (int shares, double diff) const;
	double round_to_tick (double value) const;
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

// price_ladder remembers all the relevant information about
// each price point.  step 0 == highest price, step n == starting price
class price_ladder
{
private:
	instrument inst;
	int nsteps;
	double starting_price;
	double risk_size;
public:
	price_ladder ();
	void reset (const instrument &i, double start_at, int steps, double risk);
	inline int steps () const { return nsteps; }
	void describe (int step, price_description &pd) const;
	void adjust (int step, int amt);
};

}
