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
 
#include "Packetizer.h"

uint32_t Packetizer::packetId=0;

Packetizer::Packetizer() {
	resetChecksum();
}

Packetizer::~Packetizer() {
}

void Packetizer::startPacket(uint8_t *destBuffer, PacketType type, uint16_t length){
	Header *header = (Header *)destBuffer;
	header->signature = SIGNATURE;
	header->packetId = packetId;
	header->length = length;
	header->type = type;
	header->version = VERSION;
	header->reserved=0;

	resetChecksum();

	header->headerChecksum = 0;
    for (uint8_t *h = (uint8_t *)header; h<(uint8_t*)&header->headerChecksum; ++h ){
        header->headerChecksum += *h;
    }

    header->headerChecksum=-header->headerChecksum;
	packetId++;
}

void Packetizer::checksumBlock(uint8_t *data, int cnt){
	for (; cnt; cnt--, data++){
		checksum += (Checksum)(*data);
	}
}

Packetizer::Checksum Packetizer::getChecksum(){
	return -checksum;
}

void Packetizer::resetChecksum(){
	checksum = 0;
}
