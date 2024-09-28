/*
 * A Simple APM 1.2 adaptation from
 *  SeaBIOS https://gitlab.com/qemu-project/seabios
 *
 * Courtesy of qemu-xtra <liewkj@yahoo.com>
 *
 *  Copyright (c) 2024 ...In A Galaxy far, far away...
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this library;
 * if not, see <http://www.gnu.org/licenses/>.
 */

#include "dosbox.h"
#include "callback.h"
#include "cpu.h"

#define SEG_BIOS 0xf000

// APM installation check
static void handle_155300(void)
{
    reg_ah = 1; // APM major version
    reg_al = 2; // APM minor version
    reg_bh = 'P';
    reg_bl = 'M';
    // bit 0 : 16 bit interface supported
    // bit 1 : 32 bit interface supported
    reg_cx = 0x02;
    CALLBACK_SCF(false);
}

// APM real mode interface connect
static void handle_155301(void) { CALLBACK_SCF(false); }

// APM 16 bit protected mode interface connect
static void handle_155302(void)
{
    /* 16-bit protected mode is historically buggy
     * Just let it fail
     *
    extern Bitu entry_apm16;
    reg_bx = CALLBACK_PhysPointer(entry_apm16) & 0xFFFFU;
    reg_ax = SEG_BIOS;
    reg_si = 0xfff0;
    reg_cx = SEG_BIOS;
    reg_di = 0xfff0;
    CALLBACK_SCF(false);
    */
    CALLBACK_SCF(true);
}
// APM 32 bit protected mode interface connect
static void handle_155303(void)
{
    extern Bitu entry_apm32;
    reg_ax = SEG_BIOS;
    reg_ebx = CALLBACK_PhysPointer(entry_apm32) & 0xFFFFU;
    reg_cx = SEG_BIOS;
    reg_esi = 0xfff0fff0;
    reg_dx = SEG_BIOS;
    reg_di = 0xfff0;
    CALLBACK_SCF(false);

}
// APM interface disconnect
static void handle_155304(void) { CALLBACK_SCF(false); }
// APM cpu idle
static void handle_155305(void) { CALLBACK_SCF(false); }
// APM cpu busy
static void handle_155306(void) { CALLBACK_SCF(false); }
// APM Set Power State
static void handle_155307(void)
{
    if (reg_bx != 1) {
        CALLBACK_SCF(false);
        return;
    }
    switch (reg_cx) {
        case 1:
        case 2:
            break;
        case 3:
            do {
                void *shutdown_handler(void);
                void (*apm_shutdown)(void);
                apm_shutdown = (void (*)(void))shutdown_handler();
                apm_shutdown();
            } while(0);
            break;
    }
    CALLBACK_SCF(false);
}
static void handle_155308(void) { CALLBACK_SCF(false); }
// Get Power Status
static void handle_15530a(void)
{
    reg_bh = 0x01; // on line
    reg_bl = 0xff; // unknown battery status
    reg_ch = 0x80; // no system battery
    reg_cl = 0xff; // unknown remaining time
    reg_dx = 0xffff; //unknown remaining time
    reg_si = 0x00; // zero battery
    CALLBACK_SCF(false);
}

#define RET_NOEVENT 0x80

// Get PM Event
static void handle_15530b(void)
{
    reg_ah = RET_NOEVENT;
    CALLBACK_SCF(true);
}
// APM Driver Version
static void handle_15530e(void)
{
    reg_ah = 1;
    reg_al = 2;
    CALLBACK_SCF(false);
}
// APM Engage / Disengage
static void handle_15530f(void) { CALLBACK_SCF(false); }
// APM Get Capabilities
static void handle_155310(void)
{
    reg_bl = 0;
    reg_cx = 0;
    CALLBACK_SCF(false);
}
static void handle_1553XX(void) { CALLBACK_SCF(true); }

void handle_1553(void)
{
    switch(reg_al) {
        case 0x00: handle_155300(); break;
        case 0x01: handle_155301(); break;
        case 0x02: handle_155302(); break;
        case 0x03: handle_155303(); break;
        case 0x04: handle_155304(); break;
        case 0x05: handle_155305(); break;
        case 0x06: handle_155306(); break;
        case 0x07: handle_155307(); break;
        case 0x08: handle_155308(); break;
        case 0x0a: handle_15530a(); break;
        case 0x0b: handle_15530b(); break;
        case 0x0e: handle_15530e(); break;
        case 0x0f: handle_15530f(); break;
        case 0x10: handle_155310(); break;
        default:   handle_1553XX(); break;
    }
}
Bitu handle_apm(void)
{
    handle_1553();
    reg_ip += (reg_ax == 0x5305)? 0:2;
    return CBRET_NONE;
}
