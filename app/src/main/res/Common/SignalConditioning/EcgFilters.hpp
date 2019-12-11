/*
 * This file is part of gliax-ecg-android-application
 * Glia is a project with the goal of releasing high quality free/open medical hardware
 * to increase availability to those who need it.
 * For more information visit Glia Free Medical hardware webpage: https://glia.org/
 *
 * Made by Institute Irnas (https://www.irnas.eu/)
 * Copyright (C) 2019 Vid Rajtmajer
 *
 * Based on MobilECG, an open source clinical grade Holter ECG.
 * For more information visit http://mobilecg.hu
 * Authors: Robert Csordas, Peter Isza
 *
 * This project uses modified version of usb-serial-for-android driver library
 * to communicate with Irnas made ECG board.
 * Original source code: https://github.com/mik3y/usb-serial-for-android
 * Library made by mik3y and kai-morich, modified by Vid Rajtmajer
 * Licensed under LGPL Version 2.1
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
