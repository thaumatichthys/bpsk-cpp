// bpsk.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <complex>
#include <vector>
#include <fstream>

#include "IIRFilter.hpp"
#include "PLL.hpp"
#include "SquaringLoop.hpp"
#include "DSSSDemodulator.hpp"


#define PI 3.14159265358979323846


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

int main()
{
	std::string output_path = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data.txt"; // or absolute path like "/tmp/data.txt"
	std::string output_path1 = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data1.txt"; // or absolute path like "/tmp/data.txt"
	std::string output_path2 = "C:/Users/thaumatichthys/PycharmProjects/bpsk/data2.txt"; // or absolute path like "/tmp/data.txt"

	std::string wavfile_path = "C:/Users/thaumatichthys/PycharmProjects/bpsk/output.bin";

	
	float initial_freq = 5000.1;
	float max_dev = 20;
	int chip_rate = 64;
	int seq_len = 41;
	int oversample_ratio = 16;
	int data_bitrate = 50;
	//float sample_rate = chip_rate * data_bitrate * oversample_ratio;

	/*
	DSSSDemodulator(
		float initial_freq,
		float max_deviation,
		float peak_finder_min_above_average,
		int prng_seed,
		int prng_seq_len,
		int oversample_ratio,
		int chip_coeff,
		int data_bitrate);
	*/

	DSSSDemodulator demod(initial_freq, max_dev, 3.0f, 1, seq_len, oversample_ratio, chip_rate, data_bitrate);

	//SquaringLoop squaring_loop(sample_rate, initial_freq, max_dev);

	//PLL pll(sample_rate, initial_freq, max_dev);
	//pll.SetRefDivider(1);
	//pll.SetFBDivider(1);

	float actual_freq = 1090;

	//NCO ref(sample_rate);
	//ref.ChangeFreq(actual_freq);

	//int num_cycles = sample_rate * 10;
	auto wavfile_data = load_float_array(wavfile_path);
	int num_cycles = wavfile_data.size();

	std::vector<float> pll_freqs;

	std::vector<float> pll_out;
	std::vector<float> ref_out;

	std::cout << "float size: " << wavfile_data.size();


	for (int i = 0; i < num_cycles; i++) {
		float value = wavfile_data[i];
		//float filtered_val = squaring_loop.Update(value);
		//bool ref_val = ref.Update();
		//printf("%d\n", ref_val);

		//bool pll_val = pll.Update(ref_val);
		


		//float pll_freq = squaring_loop.pll_.GetNCOFreq();
		auto ret = demod.Update(value);
		float pll_freq = ret[0];
		float pll_val = ret[1];
		if (i % 100 == 0) {
			//printf("NCO freq: %f\n", pll_freq);
			//printf("%f\n", filtered_val);
		}

		pll_freqs.push_back(pll_freq);
		pll_out.push_back(pll_val);
		//ref_out.push_back(filtered_val);
	}


	

	write_vector_to_file(pll_freqs, output_path);
	write_vector_to_file(pll_out, output_path1);
	write_vector_to_file(ref_out, output_path2);
	
	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file


