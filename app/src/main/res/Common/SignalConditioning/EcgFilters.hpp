#ifndef ECG_FILTERS_H
#define ECG_FILTERS_H

#include "SecondOrderIIR.hpp"
#include "IIRFilterChain.hpp"
#include "BidirectionalFilter.hpp"

#include "../../../jni/log.h"

class NotchFilter50: public SecondOrderIIR {
public:
	NotchFilter50(): SecondOrderIIR(
		0.97898869, -1.56606715, 0.97898869,
		1.00000000, -1.56606715, 0.95797738
	) {}
};

class NotchFilter60: public SecondOrderIIR {
public:
    NotchFilter60(): SecondOrderIIR(
        0.97489028, -1.39633962, 0.97489028,
        1.00000000, -1.39633962, 0.94978057
    ) {}
};

class BaselineFilter: public SecondOrderIIR {
public:
	BaselineFilter(): SecondOrderIIR(
		0.99361509, -1.98722936, 0.99361509,
		1.00000000, -1.98718860, 0.98727096
	) {}
};

class LowPassFilter: public SecondOrderIIR {
public:
	LowPassFilter(): SecondOrderIIR(
		0.13629773, 0.27259545, 0.13629773,
		1.00000000, -0.71939982, 0.26459073
	) {}
};

class HalfEcgFilter: public IIRFilterChain {
public:
	HalfEcgFilter(): IIRFilterChain() {
		add(&baselineFilter);
        add(&notchFilter50);
		add(&lowPassFilter);
	}
private:
	BaselineFilter baselineFilter;
	NotchFilter50 notchFilter50;
	LowPassFilter lowPassFilter;
};

class EcgFilter: public BidirectionalFilter<1200, 240> {
public:
	EcgFilter():
		BidirectionalFilter(&forward, &backward)
	{}
private:
	HalfEcgFilter forward;
	HalfEcgFilter backward;
};

#endif
