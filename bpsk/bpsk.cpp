// bpsk.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <math.h>
#include <complex>
#include <vector>

#include "IIRFilter.hpp"

#define PI 3.14159265358979323846
/*
std::vector<std::vector<float>> butterworth_lowpass(int order, float cutoff) {
    //std::complex<float> pole(1, 2);
    const float warped = 2.0f * tan(PI * cutoff / 2);
    std::vector<std::complex<float>> poles;
    // compute butterworth poles
    for (int k = 0; k < order; k++) {
        float theta = PI / 2.0 * (1 + (2.0 * k + 1.0) / order);
        std::complex<float> pole = warped * std::polar(1.0f, theta);
        poles.push_back(pole);
    }
    std::vector<std::complex<float>> digital_poles;
    // bilinear transform
    for (auto & pole : poles) {
        std::complex<float> digital_pole = (1.0f + pole / 2.0f) / (1.0f - pole / 2.0f);
        digital_poles.push_back(digital_pole);
    }
    // polynomial expansion
    std::vector<std::complex<float>> a = { std::complex<float>(1.0f, 0.0f) }; // initialize to 1
    for (auto& pole : digital_poles) {
        std::cout << std::endl;
        auto a_copy = a;
        a_copy.push_back(std::complex<float>(0.0f, 0.0f));
        a.insert(a.begin(), std::complex<float>(0.0f, 0.0f));
        for (auto& element : a)
            element *= -pole;
        for (size_t i = 0; i < a.size(); i++) {
            a[i] += a_copy[i];
        }
    }
    // discard imaginary parts of a, if all poles are complex conjugates they should cancel out
    std::vector<float> a_real;
    a_real.reserve(a.size());
    for (const auto& element : a) {
        a_real.push_back(element.real());
    }
    std::vector<float> b_real = { 1.0f };
    int coeff = 1;
    for (int k = 1; k < order + 1; k++) {
        coeff = (coeff * (order - k + 1)) / k;
        b_real.push_back(coeff);
    }
    // scale the numbers
    float numerator_unnorm_sum = 0;
    float denom_sum = 0;
    for (int i = 0; i < order + 1; i++) {
        numerator_unnorm_sum += b_real[i];
        denom_sum += a_real[i];
    }
    for (auto& element : b_real) {
        element *= denom_sum / numerator_unnorm_sum;
    }
    
    return { b_real, a_real };
}

std::vector<std::vector<float>> butterworth_bandpass(int order, float low_frac, float high_frac) {
    //std::complex<float> pole(1, 2);
    float warped_low = 2.0f * tanf(PI * low_frac / 2.0f);
    float warped_high = 2.0f * tanf(PI * high_frac / 2.0f);
    float B = warped_high - warped_low;
    float w0 = sqrtf(warped_high * warped_low);  // center frequency

    std::cout << "B:" << B << ". w0:" << w0 << std::endl;

    std::vector<std::complex<float>> lp_poles;
    // compute butterworth poles
    for (int k = 0; k < order; k++) {
        float theta = PI / 2.0 * (1 + (2.0 * k + 1.0) / order);
        std::complex<float> pole = std::polar(1.0f, theta);
        lp_poles.push_back(pole);
    }

    for (auto& p : lp_poles) {
        std::cout << "pole: " << p;
    }
    std::cout << std::endl;

    // transform to band pass
    std::vector<std::complex<float>> bp_poles;
    for (auto& p : lp_poles) {
        // Correct quadratic: s^2 - B*p*s + w0^2 = 0
        std::complex<float> Bp = B * p;
        std::complex<float> disc = sqrt(Bp * Bp - std::complex<float>(4.0f * w0 * w0, 0.0f));
        // roots = [ Bp ± disc ] / 2
        std::complex<float> s1 = (Bp + disc) * 0.5f;
        std::complex<float> s2 = (Bp - disc) * 0.5f;
        bp_poles.push_back(s1);
        bp_poles.push_back(s2);
    }

    for (auto& p : bp_poles) {
        std::cout << "bp pole: " << p;
    }
    std::cout << std::endl;

    std::vector<std::complex<float>> digital_poles;
    // bilinear transform
    for (auto& pole : bp_poles) {
        std::complex<float> digital_pole = (1.0f + pole / 2.0f) / (1.0f - pole / 2.0f);
        digital_poles.push_back(digital_pole);
    }

    for (auto& p : digital_poles) {
        std::cout << "digital pole: " << p;
    }
    std::cout << std::endl;

    // polynomial expansion
    std::vector<std::complex<float>> a = { std::complex<float>(1.0f, 0.0f) }; // initialize to 1
    for (auto& pole : digital_poles) {
        std::cout << std::endl;
        auto a_copy = a;
        a_copy.push_back(std::complex<float>(0.0f, 0.0f));
        a.insert(a.begin(), std::complex<float>(0.0f, 0.0f));
        for (auto& element : a)
            element *= -pole;
        for (size_t i = 0; i < a.size(); i++) {
            a[i] += a_copy[i];
        }
    }

    for (auto& p : a) {
        std::cout << "denom: " << p;
    }
    std::cout << std::endl;

    // discard imaginary parts of a, if all poles are complex conjugates they should cancel out
    std::vector<float> a_real;
    a_real.reserve(a.size());
    for (const auto& element : a) {
        a_real.push_back(element.real());
    }
    // get numerator
    std::vector<float> b_unscaled(2 * order + 1, 0.0f);
    int coeff = 1;
    for (int k = 0; k <= order; k++) {
        coeff = (k == 0) ? 1 : coeff * (order - k + 1) / k;
        b_unscaled[2 * k] = coeff * powf(-1, k); // alternate sign
    }
    auto b_real = b_unscaled;
    // scale the numbers
    float wd0 = 2.0f * atanf(w0 / 2.0f);
    std::complex<float> num(0, 0), den(0, 0);
    for (int k = 0; k < (int)a_real.size(); ++k) {
        auto zexp = std::polar(1.0f, -wd0 * k);
        num += b_unscaled[k] * zexp;
        den += a_real[k] * zexp;
    }
    float mag = std::abs(num / den);
    float K = 1.0f / mag;
    for (auto& x : b_real) x *= K;

    return { b_real, a_real };
}
*/
int main()
{
    /*std::cout << "Butter Worth!\n";

    auto returnval = butterworth_bandpass(2, 0.3, 0.5);
    auto b = returnval[0];
    auto a = returnval[1];

    std::cout << std::endl << std::endl;
    for (auto& element : a) {
        std::cout << element << ", ";
    }
    std::cout << std::endl;
    for (auto& element : b) {
        std::cout << element << ", ";
    }

    std::cout << std::endl;*/

    IIRFilter filter(FilterMode::LowPass, 2, 1, 1);

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


