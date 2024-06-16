#pragma once

class Smoother {
private:
    float _coeff;
    float _mem;
    float _target;

public:
    Smoother(float coeff);
    void setTarget(float target);
    float process();
    float process(float target);
    float getValue();

};