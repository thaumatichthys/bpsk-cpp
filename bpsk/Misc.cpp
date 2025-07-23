#include "Misc.hpp"

Integrator::Integrator() {
	accumulator_ = 0;
}

void Integrator::Accumulate(float value) {
	accumulator_ += value;
}

float Integrator::DumpValue() {
	float out = accumulator_;
	accumulator_ = 0;
	return out;
}

float Integrator::GetValue() {
	return accumulator_;
}

Derivator::Derivator() {
	prev_ = 0;
}

int Derivator::PushValue(bool input) {
	int out = (int)input - prev_;
	prev_ = (int)input;
	return out;
}
