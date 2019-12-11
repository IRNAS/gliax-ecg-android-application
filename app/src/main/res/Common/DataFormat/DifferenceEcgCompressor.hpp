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
 
#ifndef DIFFERENCE_ECG_COMPRESSOR_HPP
#define DIFFERENCE_ECG_COMPRESSOR_HPP

#include <stdint.h>
#include "BitFifo.hpp"
#include "IEcgPredictor.hpp"

namespace ecg {

class DifferenceEcgCompressor {
	public:
		static const int maxChannels = 8;
		static const int fullBitNum = 19;
		static const int smallBitNum = 8;
	
		DifferenceEcgCompressor(ecg::BitFifo &bitStream, IEcgPredictor& ecgPredictor);
		bool putSample(const int32_t* channels);
		bool getSample(int32_t *channels);
		void setNumChannels(unsigned numChannels);
	private:
		ecg::BitFifo &bitStream;
		unsigned numChannels;
		IEcgPredictor &ecgPredictor;
		
		static const int32_t smallMin = (1 << (smallBitNum-1)) - (1 << smallBitNum);
		static const int32_t smallMax = -smallMin-1;
		
		static const int32_t fullMin = (1 << (fullBitNum-1)) - (1 << fullBitNum);
		static const int32_t fullMax = -fullMin-1;
		
};

}

#endif
