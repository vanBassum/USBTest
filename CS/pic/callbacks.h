/*
 * Firmware framework for USB I/O on PIC 18F2455 (and siblings)
 * Copyright (C) 2005 Alexander Enzmann
 * adapted to MCC18 by Alberto Maccioni on 1/8/09
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 * or see <http://www.gnu.org/licenses/>
 */
#ifndef CALLBACKS_H
#define CALLBACKS_H

// These are the callbacks expected by the USB HID code.  Each must be
// at least stub functions for the firmware to compile and link correctly.
int GetInputReport(byte reportID);
int SetupOutputReport(byte reportID);
int SetOutputReport(byte reportID);
int GetFeatureReport(byte reportID);
int SetupFeatureReport(byte reportID);
void SetFeatureReport(byte reportID);

#endif //CALLBACKS_H
