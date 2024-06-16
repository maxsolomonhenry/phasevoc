#include <cmath>

float extend(float val, float lower, float upper);
float db2mag(float val);
float mag2db(float val);

void convertToPolar(float* buffer, const int& size);
void convertToRectangular(float* buffer, const int& size);