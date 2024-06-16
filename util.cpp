#include "util.h"

float extend(float in, float lower, float upper) {
	return in * (upper - lower) + lower;
}

float db2mag(float db) {
	return std::pow(10.0, db / 20.0);
}

float mag2db(float mag) {
	return 20.0 * std::log10(mag);
}

void convertToPolar(float* buffer, const int& size)
{
    int hN = size / 2;

    for (int n = 0; n < hN; n++)
    {
        float real = buffer[n];
        float imaginary = buffer[hN + n];
        
        float magnitude = sqrt(pow(real, 2) + pow(imaginary, 2));
        float phase = atan2(imaginary, real);
        
        buffer[n] = magnitude;
        buffer[hN + n] = phase;
    }
}

void convertToRectangular(float* buffer, const int& size)
{
    int hN = size / 2;

    for (int n = 0; n < hN; n++)
    {
        float magnitude = buffer[n];
        float phase = buffer[hN + n];
        
        float real = magnitude * cos(phase);
        float imaginary = magnitude * sin(phase);
        
        buffer[n] = real;
        buffer[hN + n] = imaginary;
    }
}