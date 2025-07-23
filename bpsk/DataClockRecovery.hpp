#pragma once 
#include "Misc.hpp"
#include "IIRFilter.hpp"


class DataClockRecovery {
public:
	DataClockRecovery(float sample_rate, float initial_baudrate, float max_dev_baud);
	float Update(float input);
private:
	Derivator derivator_;
	Derivator float_derivator_;
	Derivator out_derivator_;
	IIRFilter input_filter_;
};