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
 
#ifndef TEST_SIGNAL_GENERATOR_HPP
#define TEST_SIGNAL_GENERATOR_HPP

#include <math.h>

#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

class TestSignalGenerator {
	public:
		TestSignalGenerator(int a, int p):
			amplitude(a)
		{
			setPeriod(p);
		}
		
		void setAmplitude(int a) {
			amplitude = a;
		}
		
		void setPeriod(int p) {
			period = p;
		}
		
		int getSample(int phaseOffset = 0) {
			int phase = (index + phaseOffset) % period;
			int r = phase * 2 * amplitude / period;
			if(r > amplitude)
				r = amplitude - r;
			return r;
		}

		void next(){
			index++;
		}
	private:
		int amplitude;
		int period;
		int index;
		/*float phase;
		float delta;*/
};

#endif
