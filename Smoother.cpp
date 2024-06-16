#include "Smoother.h"

Smoother::Smoother(float coeff) : _coeff(coeff) {
    _mem = 0.0;
    _target = 0.0;
}

void Smoother::setTarget(float target) {
    _target = target;
}

float Smoother::process() {
    _mem = _coeff * _target + (1 - _coeff) * _mem;

    return _mem;
}

float Smoother::process(float target) {
    setTarget(target);
    return process();
}

float Smoother::getValue() {
    return _mem;
}