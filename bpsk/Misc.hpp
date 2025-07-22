#pragma once


class Integrator {
public:
	Integrator();
	void Accumulate(float value);
	float DumpValue();
	float GetValue();
private:
	float accumulator_ = 0;
};
