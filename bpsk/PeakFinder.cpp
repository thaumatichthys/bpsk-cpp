#include "PeakFinder.hpp"
#include "constants.hpp"


PeakFinder::PeakFinder(int seq_len, float min_above_average) {
	seq_len_ = seq_len;
	min_above_average_ = min_above_average;
	max_index_ = -1;
	
	max_val_ = 0.0f;

	buffer_.assign(seq_len, 0.0f);
	write_ptr_ = 0;
	current_addr_ = 0;
}

bool PeakFinder::PushValue(float value) {
	buffer_[write_ptr_] = value;

	if (value > max_val_) {
		max_val_ = value;
		max_index_ = write_ptr_;
	}

	write_ptr_ = (write_ptr_ + 1) % seq_len_;

	if (current_addr_ >= seq_len_ - 1) {
		buffer_[max_index_] = 0;

		// get average
		float average = 0;
		for (auto& p : buffer_) {
			average += p;
		}
		average /= seq_len_;
		
		if (max_val_ > average * min_above_average_) {
			if (parameters::PRINT_STUFF) printf("Found peak, peak = %f, average = %f\n", max_val_, average);
			// get delta
			int delta = (max_index_ - write_ptr_) % seq_len_;

			// reset
			current_addr_ = 0;
			max_val_ = 0.0f;
			max_index_ = -1;
			output_ = delta;
			return true;
		}
		if (parameters::PRINT_STUFF) printf("Did not find a peak\n");
		max_val_ = 0;
		max_index_ = -1;
		current_addr_ = 0;
	}
	else {
		current_addr_++;
	}

	return false;
}

int PeakFinder::GetFoundIndex() {
	return output_;
}
