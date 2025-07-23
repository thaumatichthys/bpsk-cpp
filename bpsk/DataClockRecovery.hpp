#pragma once 
#include "PLL.hpp"
#include "Misc.hpp"
#include "IIRFilter.hpp"


class DataClockRecovery {
public:
	DataClockRecovery(float sample_rate, float initial_baudrate, float max_dev_baud);
	void Update(float input);
private:
	PLL pll_;
	Derivator derivator_;
	IIRFilter input_filter_;
};