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

class PIController {
public:
	PIController(float sample_rate, float max_deviation, float Kp, float Ki);
	float PushValue(float input);
	void SetKParams(float Kp, float Ki);
private:
	float max_deviation_;
	float Kp_, Ki_;
	float integral_term_ = 0.0f;
	float sample_rate_;
};

class FreqDivider {
public:
	FreqDivider(int denom);
	void SetDenom(int denom);
	bool PushValue(bool val);
private:
	int counter_ovf_ = 1;
	int counter_ = 0;
	bool prev_state = false;
	bool output_state_ = false;
};

/*
* PLL Block Diagram
* 
*       REF DIV                               
*       |----|                                            
* REF---| /N |-->|-----|    |-----------|    |-----|      |--------| 
*       |----|   | PFD |--->|Loop Filter|--->| NCO |--+-->| Output |
*             -->|-----|    |-----------|    |-----|  |   |--------| 
*            |                                        |
*            |                         |----|         |
*             -------------------------| /N |<--------
*                                      |----|
*                                      FB DIV
*/
class PLL {
public:
	PLL(float sample_rate, float initial_freq, float max_deviation);
	bool Update(bool ref_in);
	float GetNCOFreq();
	void SetRefDivider(int denom);
	void SetFBDivider(int denom);
	void SetLoopFilterCenterFreq(float initial_freq);
	void SetLoopFilterKParams(float Kp, float Ki);
	NCO nco_;
	PFD pfd_;
	PIController loop_filter_;
	FreqDivider fb_divider_;
	FreqDivider ref_divider_;
private:
	float sample_rate_;
	float initial_freq_;
};