#pragma once
#include "stdint.h"
//#include "IIRFilter.hpp"


class PFD {
public:
	PFD();
	int Update(bool reference, bool input);
private:
	bool prev_ref_ = 0;
	bool prev_inp_ = 0;
	int up_ = 0;
	int down_ = 0;
};

class NCO {
public:
	NCO(float sample_rate);
	void SetFreq(float freq);
	void ChangeFreq(float increment);
	bool Update();
	bool GetOutput();
private:
	bool output_ = false;
	float sample_rate_;
	float frequency_ = 0;
	uint32_t phase_accum_ = 0;
	uint32_t tuning_word_ = 0;
};

class LoopFilter {
public:
	LoopFilter(float sample_rate);
	float PushValue(float input);
private:
	float sample_rate_;
};

class PLL {
public:
	PLL(float sample_rate, float initial_freq, float max_deviation);
	bool Update(bool ref_in);
private:
	NCO nco_;
	PFD pfd_;
	LoopFilter loop_filter_;
	float max_deviation_;
	float sample_rate_;
	float freq_correction_ = 0;
	float initial_freq_;

};