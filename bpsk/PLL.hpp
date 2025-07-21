#pragma once
#include "stdint.h"
#include "IIRFilter.hpp"


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
	float GetFreq();
	float GetSineValue();
	float GetCosValue();
private:
	void UpdateFreq_();
	bool output_ = false;
	float sample_rate_;
	float frequency_ = 0;
	uint32_t phase_accum_ = 0;
	uint32_t tuning_word_ = 0;
};

class LoopFilter {
public:
	LoopFilter(float sample_rate, float max_deviation, float Kp, float Ki);
	float PushValue(float input);
	void SetKParams(float Kp, float Ki);
private:
	float max_deviation_;
	float Kp_, Ki_;
	float integral_term_ = 0.0f;
	float sample_rate_;
};

/*
* REF--->|-----|    |-----------|    |-----|   |----|      |---------|
*        | PFD |--->|Loop Filter|--->| NCO |---| x2 |----->| Out Div |
*     -->|-----|    |-----------|    |-----|   |----|  |   |---------|
*    |                                                 |
*    |              |--------|                         |
*     --------------| FB Div |<------------------------
*                   |--------|
*/
class PLL {
public:
	PLL(float sample_rate, float initial_freq, float max_deviation);
	bool Update(bool ref_in);
	float GetNCOFreq();
	NCO nco_;
	PFD pfd_;
	LoopFilter loop_filter_;
private:
	float sample_rate_;
	float initial_freq_;
};