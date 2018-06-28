/*
 * This file is part of MobilECG, an open source clinical grade Holter
 * ECG. For more information visit http://mobilecg.hu
 *
 * Copyright (C) 2016  Robert Csordas, Peter Isza
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
 
#ifndef ANDROIDAPP_PACKETROUTER_H
#define ANDROIDAPP_PACKETROUTER_H

#include "../res/Common/DataFormat/Packetizer.h"

class PacketRouter {
    private:
        PacketRouter();
        int prevIndex;
    public:
        static PacketRouter &instance();
        void packetReceived(Packetizer::Header *header, char *data);
};


#endif //ANDROIDAPP_PACKETROUTER_H
