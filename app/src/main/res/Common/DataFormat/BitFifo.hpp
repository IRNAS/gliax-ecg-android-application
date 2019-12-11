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
 
#ifndef FIFO_BUFFER_HPP
#define FIFO_BUFFER_HPP

#include <stdint.h>

/* Endian-safe FIFO ring buffer for binary streams. */

namespace ecg {

class BitFifo {
	public:
		BitFifo(char *buffer, int sizeBytes, int bitsAlreadyInBuffer=0);
		
		#ifndef EMBEDDED
		//to be added: constructor with dynamic memory allocation
		#endif
				
		bool pushByte(uint8_t data);
		bool pushBits(uint32_t data, char numBits);
		
		uint8_t popByte();
		uint32_t popBits(char numBits);
		int32_t popBitsSigned(char numBits);
		
		int popBytes(char *buffer, int size);
		int pushBytes(const char *buffer, int size);
		
		int getFreeBytes();
		int getFreeBits();
		
		int getAvailableBytes();
		int getAvailableBits();
		
		int getSizeBytes();
		
		void reset();
		
	private:
		char *buffer;
		int sizeBytes;
		int sizeBits; 
		int start; // bits
		int end; // bits
};

}

#endif
