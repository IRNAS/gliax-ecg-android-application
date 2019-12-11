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
#include "DifferenceEcgCompressor.hpp"

#include <stdio.h>
using namespace ecg;


DifferenceEcgCompressor::DifferenceEcgCompressor(BitFifo &pBitStream, IEcgPredictor& pEcgPredictor):
	bitStream(pBitStream),
	ecgPredictor(pEcgPredictor)
{
	numChannels=0;
}

bool DifferenceEcgCompressor::putSample(const int32_t* channels) {
	for(unsigned int i = 0; i < numChannels; ++i) {
		int32_t diff = channels[i] - ecgPredictor.getPrediction(i);
		//printf(" newdata=%d pred=%d\n", channels[i], ecgPredictor.getPrediction(i));
		bool okay = true;
		if(diff >= smallMin && diff <= smallMax) {
			okay &= bitStream.pushBits(0, 1);
			okay &= bitStream.pushBits(diff, smallBitNum);
			//printf("S %d\n", diff);
		} else {
			okay &= bitStream.pushBits(1, 1);
			okay &= bitStream.pushBits(channels[i], fullBitNum);
			//printf("F %d\n", diff);
		}
		if(!okay)
			return false;
	}
	ecgPredictor.putSample(channels);
	return true;
}

bool DifferenceEcgCompressor::getSample(int32_t *channels) {
	for(unsigned int i = 0; i < numChannels; ++i) {
		int full = bitStream.popBits(1);
		//printf('');
		int bitNum = full ? fullBitNum : smallBitNum;
		if(bitStream.getAvailableBits() < bitNum)
			return false;
		if(full)
			channels[i] = bitStream.popBitsSigned(fullBitNum);
		else
			channels[i] = ecgPredictor.getPrediction(i) + bitStream.popBitsSigned(smallBitNum);
	}	
	ecgPredictor.putSample(channels);
	return true;
}

void DifferenceEcgCompressor::setNumChannels(unsigned pNumChannels){
	numChannels=pNumChannels;
	if(numChannels > maxChannels)
		numChannels = maxChannels;
}
