#include <vector>


class PeakFinder {
public:
	PeakFinder(int seq_len, float min_above_average);
	bool PushValue(float value);
	int GetFoundIndex(); // this is not valid until pushvalue returns true
private:
	int seq_len_;
	float max_val_;
	int max_index_;
	int write_ptr_;
	int current_addr_;
	float min_above_average_;
	int output_;
	std::vector<float> buffer_;
};