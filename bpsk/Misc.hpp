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

class Derivator {
public:
	Derivator();
	int PushValue(bool input);
private:
	int prev_;
};