// bpsk.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <complex>
#include <vector>
#include <fstream>
#include <string>
#include <algorithm>

#include "IIRFilter.hpp"
#include "PLL.hpp"
#include "SquaringLoop.hpp"
#include "DSSSDemodulator.hpp"


void write_vector_to_file(const std::vector<float>& vec, const std::string& path) {
	std::ofstream file(path);
	if (!file) {
		std::cerr << "Failed to open file: " << path << "\n";
		return;
	}

	for (float val : vec) {
		file << val << "\n";
	}

	file.close();
}


std::vector<float> load_float_array(const std::string& filename) {
	std::ifstream file(filename, std::ios::binary | std::ios::ate);
	if (!file) {
		std::cerr << "Failed to open file: " << filename << "\n";
		return {};
	}

	std::streamsize size = file.tellg();
	if (size % sizeof(float) != 0) {
		std::cerr << "File size is not a multiple of float size!\n";
		return {};
	}

	file.seekg(0, std::ios::beg);

	std::vector<float> data(size / sizeof(float));
	if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
		std::cerr << "Failed to read file contents.\n";
		return {};
	}

	return data;
}

// Converts bits to bytes starting from a given offset
std::string bitsToAscii(const std::vector<bool>& bits, int offset, bool invert) {
	std::string result;
	for (size_t i = offset; i + 7 < bits.size(); i += 8) {
		uint8_t byte = 0;
		for (int b = 0; b < 8; ++b) {
			bool bit = bits[i + b];
			if (invert) bit = !bit;
			byte = (byte << 1) | bit;
		}
		result += static_cast<char>(byte);
	}
	return result;
}

// Counts how many chars are printable ASCII (32 to 126)
int countPrintable(const std::string& s) {
	return std::count_if(s.begin(), s.end(), [](char c) {
		return c >= 32 && c <= 126;
		});
}

// Main function: finds best offset and inversion to maximize ASCII chars
void findBestAsciiDecode(const std::vector<bool>& bitstream) {
	int bestScore = -1;
	std::string bestString;
	int bestOffset = 0;
	bool bestInverted = false;

	for (int offset = 0; offset < 8; ++offset) {
		for (bool invert : {false, true}) {
			std::string decoded = bitsToAscii(bitstream, offset, invert);
			int score = countPrintable(decoded);

			if (score > bestScore) {
				bestScore = score;
				bestString = decoded;
				bestOffset = offset;
				bestInverted = invert;
			}
		}
	}

	std::cout << "Best offset: " << bestOffset << "\n";
	std::cout << "Inverted: " << (bestInverted ? "yes" : "no") << "\n";
	std::cout << "Printable chars: " << bestScore << "\n";
	std::cout << "Decoded:\n" << bestString << "\n";
}

int main()
{
	std::string output_path = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data.txt"; // or absolute path like "/tmp/data.txt"
	std::string output_path1 = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data1.txt"; // or absolute path like "/tmp/data.txt"
	std::string output_path2 = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data2.txt"; // or absolute path like "/tmp/data.txt"

	std::string wavfile_path = "C:/Users/thaumatichthys/PycharmProjects/bpsk/output.bin";

	
	float initial_freq = 8005;
	float max_dev = 10;
	int chip_rate = 150;
	int seq_len = 41;
	int oversample_ratio = 8;
	int data_bitrate = 30;


	DSSSDemodulator demod(initial_freq, max_dev, 6.0f, 1, seq_len, oversample_ratio, chip_rate, data_bitrate, oversample_ratio);

	auto wavfile_data = load_float_array(wavfile_path);
	int num_cycles = wavfile_data.size();

	std::vector<float> pll_freqs;

	std::vector<float> pll_out;
	std::vector<float> ref_out;

	std::vector<bool> output;


	for (int i = 0; i < num_cycles; i++) {
		float value = wavfile_data[i];

		int ret = demod.Update(value);

		if (ret > -1) {
			output.push_back(ret);
		}
	}

	findBestAsciiDecode(output);
	

	//write_vector_to_file(pll_freqs, output_path);
	//write_vector_to_file(pll_out, output_path1);
	//write_vector_to_file(ref_out, output_path2);
	
	return 0;
}

