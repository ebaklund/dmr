//
// Copyright (C) 2013-2018 Alexey Khokholov (Nuke.YKT)
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//
//  Nuked OPL3 emulator.
//  Thanks:
//      MAME Development Team(Jarek Burczynski, Tatsuyuki Satoh):
//          Feedback and Rhythm part calculation information.
//      forums.submarine.org.uk(carbon14, opl3):
//          Tremolo and phase generator calculation information.
//      OPLx decapsulated(Matthew Gambrell, Olli Niemitalo):
//          OPL2 ROMs.
//      siliconpr0n.org(John McMaster, digshadow):
//          YMF262 and VRC VII decaps and die shots.
//
// version: 1.8
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "opl3.h"

#define RSM_FRAC    10

// Channel types

enum {
    ch_2op = 0,
    ch_4op = 1,
    ch_4op2 = 2,
    ch_drum = 3
};

// Envelope key types

enum {
    egk_norm = 0x01,
    egk_drum = 0x02
};

//
// freq mult table multiplied by 2
//
// 1/2, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 12, 12, 15, 15
//

static const Bit8u mt[16] = {
    1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20, 20, 24, 24, 30, 30
};

//
// ksl table
//

static const Bit8u kslrom[16] = {
    0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64
};

static const Bit8u kslshift[4] = {
    8, 1, 2, 0
};

//
// envelope generator constants
//

static const Bit8u eg_incstep[4][4] = {
    { 0, 0, 0, 0 },
    { 1, 0, 0, 0 },
    { 1, 0, 1, 0 },
    { 1, 1, 1, 0 }
};

//
// address decoding
//

static const Bit8s ad_slot[0x20] = {
    0, 1, 2, 3, 4, 5, -1, -1, 6, 7, 8, 9, 10, 11, -1, -1,
    12, 13, 14, 15, 16, 17, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

static const Bit8u ch_slot[18] = {
    0, 1, 2, 6, 7, 8, 12, 13, 14, 18, 19, 20, 24, 25, 26, 30, 31, 32
};

//
// Envelope generator
//

typedef Bit16s(*envelope_sinfunc)(Bit16u phase, Bit16u envelope);

extern Bit16s OPL3_EnvelopeCalcSin0(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin1(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin2(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin3(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin4(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin5(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin6(Bit16u phase, Bit16u envelope);
extern Bit16s OPL3_EnvelopeCalcSin7(Bit16u phase, Bit16u envelope);

static const envelope_sinfunc envelope_sin[8] = {
    OPL3_EnvelopeCalcSin0,
    OPL3_EnvelopeCalcSin1,
    OPL3_EnvelopeCalcSin2,
    OPL3_EnvelopeCalcSin3,
    OPL3_EnvelopeCalcSin4,
    OPL3_EnvelopeCalcSin5,
    OPL3_EnvelopeCalcSin6,
    OPL3_EnvelopeCalcSin7
};

enum envelope_gen_num
{
    envelope_gen_num_attack = 0,
    envelope_gen_num_decay = 1,
    envelope_gen_num_sustain = 2,
    envelope_gen_num_release = 3
};

static void OPL3_EnvelopeUpdateKSL(opl3_slot *slot)
{
    Bit16s ksl = (kslrom[slot->channel->f_num >> 6] << 2)
               - ((0x08 - slot->channel->block) << 5);
    if (ksl < 0)
    {
        ksl = 0;
    }
    slot->eg_ksl = (Bit8u)ksl;
}

static void OPL3_EnvelopeCalc(opl3_slot *slot)
{
    Bit8u nonzero;
    Bit8u rate;
    Bit8u rate_hi;
    Bit8u rate_lo;
    Bit8u reg_rate = 0;
    Bit8u ks;
    Bit8u eg_shift, shift;
    Bit16u eg_rout;
    Bit16s eg_inc;
    Bit8u eg_off;
    Bit8u reset = 0;
    slot->eg_out = slot->eg_rout + (slot->reg_tl << 2)
                 + (slot->eg_ksl >> kslshift[slot->reg_ksl]) + *slot->trem;
    if (slot->key && slot->eg_gen == envelope_gen_num_release)
    {
        reset = 1;
        reg_rate = slot->reg_ar;
    }
    else
    {
        switch (slot->eg_gen)
        {
        case envelope_gen_num_attack:
            reg_rate = slot->reg_ar;
            break;
        case envelope_gen_num_decay:
            reg_rate = slot->reg_dr;
            break;
        case envelope_gen_num_sustain:
            if (!slot->reg_type)
            {
                reg_rate = slot->reg_rr;
            }
            break;
        case envelope_gen_num_release:
            reg_rate = slot->reg_rr;
            break;
        }
    }
    slot->pg_reset = reset;
    ks = slot->channel->ksv >> ((slot->reg_ksr ^ 1) << 1);
    nonzero = (reg_rate != 0);
    rate = ks + (reg_rate << 2);
    rate_hi = rate >> 2;
    rate_lo = rate & 0x03;
    if (rate_hi & 0x10)
    {
        rate_hi = 0x0f;
    }
    eg_shift = rate_hi + slot->chip->eg_add;
    shift = 0;
    if (nonzero)
    {
        if (rate_hi < 12)
        {
            if (slot->chip->eg_state)
            {
                switch (eg_shift)
                {
                case 12:
                    shift = 1;
                    break;
                case 13:
                    shift = (rate_lo >> 1) & 0x01;
                    break;
                case 14:
                    shift = rate_lo & 0x01;
                    break;
                default:
                    break;
                }
            }
        }
        else
        {
            shift = (rate_hi & 0x03) + eg_incstep[rate_lo][slot->chip->timer & 0x03];
            if (shift & 0x04)
            {
                shift = 0x03;
            }
            if (!shift)
            {
                shift = slot->chip->eg_state;
            }
        }
    }
    eg_rout = slot->eg_rout;
    eg_inc = 0;
    eg_off = 0;
    // Instant attack
    if (reset && rate_hi == 0x0f)
    {
        eg_rout = 0x00;
    }
    // Envelope off
    if ((slot->eg_rout & 0x1f8) == 0x1f8)
    {
        eg_off = 1;
    }
    if (slot->eg_gen != envelope_gen_num_attack && !reset && eg_off)
    {
        eg_rout = 0x1ff;
    }
    switch (slot->eg_gen)
    {
    case envelope_gen_num_attack:
        if (!slot->eg_rout)
        {
            slot->eg_gen = envelope_gen_num_decay;
        }
        else if (slot->key && shift > 0 && rate_hi != 0x0f)
        {
            eg_inc = ((~slot->eg_rout) << shift) >> 4;
        }
        break;
    case envelope_gen_num_decay:
        if ((slot->eg_rout >> 4) == slot->reg_sl)
        {
            slot->eg_gen = envelope_gen_num_sustain;
        }
        else if (!eg_off && !reset && shift > 0)
        {
            eg_inc = 1 << (shift - 1);
        }
        break;
    case envelope_gen_num_sustain:
    case envelope_gen_num_release:
        if (!eg_off && !reset && shift > 0)
        {
            eg_inc = 1 << (shift - 1);
        }
        break;
    }
    slot->eg_rout = (eg_rout + eg_inc) & 0x1ff;
    // Key off
    if (reset)
    {
        slot->eg_gen = envelope_gen_num_attack;
    }
    if (!slot->key)
    {
        slot->eg_gen = envelope_gen_num_release;
    }
}

static void OPL3_EnvelopeKeyOn(opl3_slot *slot, Bit8u type)
{
    slot->key |= type;
}

static void OPL3_EnvelopeKeyOff(opl3_slot *slot, Bit8u type)
{
    slot->key &= ~type;
}

//
// Phase Generator
//

static void OPL3_PhaseGenerate(opl3_slot *slot)
{
    opl3_chip *chip;
    Bit16u f_num;
    Bit32u basefreq;
    Bit8u rm_xor, n_bit;
    Bit32u noise;
    Bit16u phase;

    chip = slot->chip;
    f_num = slot->channel->f_num;
    if (slot->reg_vib)
    {
        Bit8s range;
        Bit8u vibpos;

        range = (f_num >> 7) & 7;
        vibpos = slot->chip->vibpos;

        if (!(vibpos & 3))
        {
            range = 0;
        }
        else if (vibpos & 1)
        {
            range >>= 1;
        }
        range >>= slot->chip->vibshift;

        if (vibpos & 4)
        {
            range = -range;
        }
        f_num += range;
    }
    basefreq = (f_num << slot->channel->block) >> 1;
    phase = (Bit16u)(slot->pg_phase >> 9);
    if (slot->pg_reset)
    {
        slot->pg_phase = 0;
    }
    slot->pg_phase += (basefreq * mt[slot->reg_mult]) >> 1;
    // Rhythm mode
    noise = chip->noise;
    slot->pg_phase_out = phase;
    if (slot->slot_num == 13) // hh
    {
        chip->rm_hh_bit2 = (phase >> 2) & 1;
        chip->rm_hh_bit3 = (phase >> 3) & 1;
        chip->rm_hh_bit7 = (phase >> 7) & 1;
        chip->rm_hh_bit8 = (phase >> 8) & 1;
    }
    if (slot->slot_num == 17 && (chip->rhy & 0x20)) // tc
    {
        chip->rm_tc_bit3 = (phase >> 3) & 1;
        chip->rm_tc_bit5 = (phase >> 5) & 1;
    }
    if (chip->rhy & 0x20)
    {
        rm_xor = (chip->rm_hh_bit2 ^ chip->rm_hh_bit7)
               | (chip->rm_hh_bit3 ^ chip->rm_tc_bit5)
               | (chip->rm_tc_bit3 ^ chip->rm_tc_bit5);
        switch (slot->slot_num)
        {
        case 13: // hh
            slot->pg_phase_out = rm_xor << 9;
            if (rm_xor ^ (noise & 1))
            {
                slot->pg_phase_out |= 0xd0;
            }
            else
            {
                slot->pg_phase_out |= 0x34;
            }
            break;
        case 16: // sd
            slot->pg_phase_out = (chip->rm_hh_bit8 << 9)
                               | ((chip->rm_hh_bit8 ^ (noise & 1)) << 8);
            break;
        case 17: // tc
            slot->pg_phase_out = (rm_xor << 9) | 0x80;
            break;
        default:
            break;
        }
    }
    n_bit = ((noise >> 14) ^ noise) & 0x01;
    chip->noise = (noise >> 1) | (n_bit << 22);
}

//
// Slot
//

static void OPL3_SlotWrite20(opl3_slot *slot, Bit8u data)
{
    if ((data >> 7) & 0x01)
    {
        slot->trem = &slot->chip->tremolo;
    }
    else
    {
        slot->trem = (Bit8u*)&slot->chip->zeromod;
    }
    slot->reg_vib = (data >> 6) & 0x01;
    slot->reg_type = (data >> 5) & 0x01;
    slot->reg_ksr = (data >> 4) & 0x01;
    slot->reg_mult = data & 0x0f;
}

static void OPL3_SlotWrite40(opl3_slot *slot, Bit8u data)
{
    slot->reg_ksl = (data >> 6) & 0x03;
    slot->reg_tl = data & 0x3f;
    OPL3_EnvelopeUpdateKSL(slot);
}

static void OPL3_SlotWrite60(opl3_slot *slot, Bit8u data)
{
    slot->reg_ar = (data >> 4) & 0x0f;
    slot->reg_dr = data & 0x0f;
}

static void OPL3_SlotWrite80(opl3_slot *slot, Bit8u data)
{
    slot->reg_sl = (data >> 4) & 0x0f;
    if (slot->reg_sl == 0x0f)
    {
        slot->reg_sl = 0x1f;
    }
    slot->reg_rr = data & 0x0f;
}

static void OPL3_SlotWriteE0(opl3_slot *slot, Bit8u data)
{
    slot->reg_wf = data & 0x07;
    if (slot->chip->newm == 0x00)
    {
        slot->reg_wf &= 0x03;
    }
}

static void OPL3_SlotGenerate(opl3_slot *slot)
{
    slot->out = envelope_sin[slot->reg_wf](slot->pg_phase_out + *slot->mod, slot->eg_out);
}

static void OPL3_SlotCalcFB(opl3_slot *slot)
{
    if (slot->channel->fb != 0x00)
    {
        slot->fbmod = (slot->prout + slot->out) >> (0x09 - slot->channel->fb);
    }
    else
    {
        slot->fbmod = 0;
    }
    slot->prout = slot->out;
}

//
// Channel
//

static void OPL3_ChannelSetupAlg(opl3_channel *channel);

static void OPL3_ChannelUpdateRhythm(opl3_chip *chip, Bit8u data)
{
    opl3_channel *channel6;
    opl3_channel *channel7;
    opl3_channel *channel8;
    Bit8u chnum;

    chip->rhy = data & 0x3f;
    if (chip->rhy & 0x20)
    {
        channel6 = &chip->channel[6];
        channel7 = &chip->channel[7];
        channel8 = &chip->channel[8];
        channel6->out[0] = &channel6->slots[1]->out;
        channel6->out[1] = &channel6->slots[1]->out;
        channel6->out[2] = &chip->zeromod;
        channel6->out[3] = &chip->zeromod;
        channel7->out[0] = &channel7->slots[0]->out;
        channel7->out[1] = &channel7->slots[0]->out;
        channel7->out[2] = &channel7->slots[1]->out;
        channel7->out[3] = &channel7->slots[1]->out;
        channel8->out[0] = &channel8->slots[0]->out;
        channel8->out[1] = &channel8->slots[0]->out;
        channel8->out[2] = &channel8->slots[1]->out;
        channel8->out[3] = &channel8->slots[1]->out;
        for (chnum = 6; chnum < 9; chnum++)
        {
            chip->channel[chnum].chtype = ch_drum;
        }
        OPL3_ChannelSetupAlg(channel6);
        OPL3_ChannelSetupAlg(channel7);
        OPL3_ChannelSetupAlg(channel8);
        //hh
        if (chip->rhy & 0x01)
        {
            OPL3_EnvelopeKeyOn(channel7->slots[0], egk_drum);
        }
        else
        {
            OPL3_EnvelopeKeyOff(channel7->slots[0], egk_drum);
        }
        //tc
        if (chip->rhy & 0x02)
        {
            OPL3_EnvelopeKeyOn(channel8->slots[1], egk_drum);
        }
        else
        {
            OPL3_EnvelopeKeyOff(channel8->slots[1], egk_drum);
        }
        //tom
        if (chip->rhy & 0x04)
        {
            OPL3_EnvelopeKeyOn(channel8->slots[0], egk_drum);
        }
        else
        {
            OPL3_EnvelopeKeyOff(channel8->slots[0], egk_drum);
        }
        //sd
        if (chip->rhy & 0x08)
        {
            OPL3_EnvelopeKeyOn(channel7->slots[1], egk_drum);
        }
        else
        {
            OPL3_EnvelopeKeyOff(channel7->slots[1], egk_drum);
        }
        //bd
        if (chip->rhy & 0x10)
        {
            OPL3_EnvelopeKeyOn(channel6->slots[0], egk_drum);
            OPL3_EnvelopeKeyOn(channel6->slots[1], egk_drum);
        }
        else
        {
            OPL3_EnvelopeKeyOff(channel6->slots[0], egk_drum);
            OPL3_EnvelopeKeyOff(channel6->slots[1], egk_drum);
        }
    }
    else
    {
        for (chnum = 6; chnum < 9; chnum++)
        {
            chip->channel[chnum].chtype = ch_2op;
            OPL3_ChannelSetupAlg(&chip->channel[chnum]);
            OPL3_EnvelopeKeyOff(chip->channel[chnum].slots[0], egk_drum);
            OPL3_EnvelopeKeyOff(chip->channel[chnum].slots[1], egk_drum);
        }
    }
}

static void OPL3_ChannelWriteA0(opl3_channel *channel, Bit8u data)
{
    if (channel->chip->newm && channel->chtype == ch_4op2)
    {
        return;
    }
    channel->f_num = (channel->f_num & 0x300) | data;
    channel->ksv = (channel->block << 1)
                 | ((channel->f_num >> (0x09 - channel->chip->nts)) & 0x01);
    OPL3_EnvelopeUpdateKSL(channel->slots[0]);
    OPL3_EnvelopeUpdateKSL(channel->slots[1]);
    if (channel->chip->newm && channel->chtype == ch_4op)
    {
        channel->pair->f_num = channel->f_num;
        channel->pair->ksv = channel->ksv;
        OPL3_EnvelopeUpdateKSL(channel->pair->slots[0]);
        OPL3_EnvelopeUpdateKSL(channel->pair->slots[1]);
    }
}

static void OPL3_ChannelWriteB0(opl3_channel *channel, Bit8u data)
{
    if (channel->chip->newm && channel->chtype == ch_4op2)
    {
        return;
    }
    channel->f_num = (channel->f_num & 0xff) | ((data & 0x03) << 8);
    channel->block = (data >> 2) & 0x07;
    channel->ksv = (channel->block << 1)
                 | ((channel->f_num >> (0x09 - channel->chip->nts)) & 0x01);
    OPL3_EnvelopeUpdateKSL(channel->slots[0]);
    OPL3_EnvelopeUpdateKSL(channel->slots[1]);
    if (channel->chip->newm && channel->chtype == ch_4op)
    {
        channel->pair->f_num = channel->f_num;
        channel->pair->block = channel->block;
        channel->pair->ksv = channel->ksv;
        OPL3_EnvelopeUpdateKSL(channel->pair->slots[0]);
        OPL3_EnvelopeUpdateKSL(channel->pair->slots[1]);
    }
}

static void OPL3_ChannelSetupAlg(opl3_channel *channel)
{
    if (channel->chtype == ch_drum)
    {
        if (channel->ch_num == 7 || channel->ch_num == 8)
        {
            channel->slots[0]->mod = &channel->chip->zeromod;
            channel->slots[1]->mod = &channel->chip->zeromod;
            return;
        }
        switch (channel->alg & 0x01)
        {
        case 0x00:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->slots[0]->out;
            break;
        case 0x01:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->chip->zeromod;
            break;
        }
        return;
    }
    if (channel->alg & 0x08)
    {
        return;
    }
    if (channel->alg & 0x04)
    {
        channel->pair->out[0] = &channel->chip->zeromod;
        channel->pair->out[1] = &channel->chip->zeromod;
        channel->pair->out[2] = &channel->chip->zeromod;
        channel->pair->out[3] = &channel->chip->zeromod;
        switch (channel->alg & 0x03)
        {
        case 0x00:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->pair->slots[0]->out;
            channel->slots[0]->mod = &channel->pair->slots[1]->out;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->slots[1]->out;
            channel->out[1] = &channel->chip->zeromod;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x01:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->pair->slots[0]->out;
            channel->slots[0]->mod = &channel->chip->zeromod;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->pair->slots[1]->out;
            channel->out[1] = &channel->slots[1]->out;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x02:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->chip->zeromod;
            channel->slots[0]->mod = &channel->pair->slots[1]->out;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->pair->slots[0]->out;
            channel->out[1] = &channel->slots[1]->out;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x03:
            channel->pair->slots[0]->mod = &channel->pair->slots[0]->fbmod;
            channel->pair->slots[1]->mod = &channel->chip->zeromod;
            channel->slots[0]->mod = &channel->pair->slots[1]->out;
            channel->slots[1]->mod = &channel->chip->zeromod;
            channel->out[0] = &channel->pair->slots[0]->out;
            channel->out[1] = &channel->slots[0]->out;
            channel->out[2] = &channel->slots[1]->out;
            channel->out[3] = &channel->chip->zeromod;
            break;
        }
    }
    else
    {
        switch (channel->alg & 0x01)
        {
        case 0x00:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->slots[0]->out;
            channel->out[0] = &channel->slots[1]->out;
            channel->out[1] = &channel->chip->zeromod;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        case 0x01:
            channel->slots[0]->mod = &channel->slots[0]->fbmod;
            channel->slots[1]->mod = &channel->chip->zeromod;
            channel->out[0] = &channel->slots[0]->out;
            channel->out[1] = &channel->slots[1]->out;
            channel->out[2] = &channel->chip->zeromod;
            channel->out[3] = &channel->chip->zeromod;
            break;
        }
    }
}

static void OPL3_ChannelWriteC0(opl3_channel *channel, Bit8u data)
{
    channel->fb = (data & 0x0e) >> 1;
    channel->con = data & 0x01;
    channel->alg = channel->con;
    if (channel->chip->newm)
    {
        if (channel->chtype == ch_4op)
        {
            channel->pair->alg = 0x04 | (channel->con << 1) | (channel->pair->con);
            channel->alg = 0x08;
            OPL3_ChannelSetupAlg(channel->pair);
        }
        else if (channel->chtype == ch_4op2)
        {
            channel->alg = 0x04 | (channel->pair->con << 1) | (channel->con);
            channel->pair->alg = 0x08;
            OPL3_ChannelSetupAlg(channel);
        }
        else
        {
            OPL3_ChannelSetupAlg(channel);
        }
    }
    else
    {
        OPL3_ChannelSetupAlg(channel);
    }
    if (channel->chip->newm)
    {
        channel->cha = ((data >> 4) & 0x01) ? ~0 : 0;
        channel->chb = ((data >> 5) & 0x01) ? ~0 : 0;
    }
    else
    {
        channel->cha = channel->chb = (Bit16u)~0;
    }
}

static void OPL3_ChannelKeyOn(opl3_channel *channel)
{
    if (channel->chip->newm)
    {
        if (channel->chtype == ch_4op)
        {
            OPL3_EnvelopeKeyOn(channel->slots[0], egk_norm);
            OPL3_EnvelopeKeyOn(channel->slots[1], egk_norm);
            OPL3_EnvelopeKeyOn(channel->pair->slots[0], egk_norm);
            OPL3_EnvelopeKeyOn(channel->pair->slots[1], egk_norm);
        }
        else if (channel->chtype == ch_2op || channel->chtype == ch_drum)
        {
            OPL3_EnvelopeKeyOn(channel->slots[0], egk_norm);
            OPL3_EnvelopeKeyOn(channel->slots[1], egk_norm);
        }
    }
    else
    {
        OPL3_EnvelopeKeyOn(channel->slots[0], egk_norm);
        OPL3_EnvelopeKeyOn(channel->slots[1], egk_norm);
    }
}

static void OPL3_ChannelKeyOff(opl3_channel *channel)
{
    if (channel->chip->newm)
    {
        if (channel->chtype == ch_4op)
        {
            OPL3_EnvelopeKeyOff(channel->slots[0], egk_norm);
            OPL3_EnvelopeKeyOff(channel->slots[1], egk_norm);
            OPL3_EnvelopeKeyOff(channel->pair->slots[0], egk_norm);
            OPL3_EnvelopeKeyOff(channel->pair->slots[1], egk_norm);
        }
        else if (channel->chtype == ch_2op || channel->chtype == ch_drum)
        {
            OPL3_EnvelopeKeyOff(channel->slots[0], egk_norm);
            OPL3_EnvelopeKeyOff(channel->slots[1], egk_norm);
        }
    }
    else
    {
        OPL3_EnvelopeKeyOff(channel->slots[0], egk_norm);
        OPL3_EnvelopeKeyOff(channel->slots[1], egk_norm);
    }
}

static void OPL3_ChannelSet4Op(opl3_chip *chip, Bit8u data)
{
    Bit8u bit;
    Bit8u chnum;
    for (bit = 0; bit < 6; bit++)
    {
        chnum = bit;
        if (bit >= 3)
        {
            chnum += 9 - 3;
        }
        if ((data >> bit) & 0x01)
        {
            chip->channel[chnum].chtype = ch_4op;
            chip->channel[chnum + 3].chtype = ch_4op2;
        }
        else
        {
            chip->channel[chnum].chtype = ch_2op;
            chip->channel[chnum + 3].chtype = ch_2op;
        }
    }
}

static Bit16s OPL3_ClipSample(Bit32s sample)
{
    if (sample > 32767)
    {
        sample = 32767;
    }
    else if (sample < -32768)
    {
        sample = -32768;
    }
    return (Bit16s)sample;
}

void OPL3_Generate(opl3_chip *chip, Bit16s *buf)
{
    Bit8u ii;
    Bit8u jj;
    Bit16s accm;
    Bit8u shift = 0;

    buf[1] = OPL3_ClipSample(chip->mixbuff[1]);

    for (ii = 0; ii < 15; ii++)
    {
        OPL3_SlotCalcFB(&chip->slot[ii]);
        OPL3_EnvelopeCalc(&chip->slot[ii]);
        OPL3_PhaseGenerate(&chip->slot[ii]);
        OPL3_SlotGenerate(&chip->slot[ii]);
    }

    chip->mixbuff[0] = 0;
    for (ii = 0; ii < 18; ii++)
    {
        accm = 0;
        for (jj = 0; jj < 4; jj++)
        {
            accm += *chip->channel[ii].out[jj];
        }
        chip->mixbuff[0] += (Bit16s)(accm & chip->channel[ii].cha);
    }

    for (ii = 15; ii < 18; ii++)
    {
        OPL3_SlotCalcFB(&chip->slot[ii]);
        OPL3_EnvelopeCalc(&chip->slot[ii]);
        OPL3_PhaseGenerate(&chip->slot[ii]);
        OPL3_SlotGenerate(&chip->slot[ii]);
    }

    buf[0] = OPL3_ClipSample(chip->mixbuff[0]);

    for (ii = 18; ii < 33; ii++)
    {
        OPL3_SlotCalcFB(&chip->slot[ii]);
        OPL3_EnvelopeCalc(&chip->slot[ii]);
        OPL3_PhaseGenerate(&chip->slot[ii]);
        OPL3_SlotGenerate(&chip->slot[ii]);
    }

    chip->mixbuff[1] = 0;
    for (ii = 0; ii < 18; ii++)
    {
        accm = 0;
        for (jj = 0; jj < 4; jj++)
        {
            accm += *chip->channel[ii].out[jj];
        }
        chip->mixbuff[1] += (Bit16s)(accm & chip->channel[ii].chb);
    }

    for (ii = 33; ii < 36; ii++)
    {
        OPL3_SlotCalcFB(&chip->slot[ii]);
        OPL3_EnvelopeCalc(&chip->slot[ii]);
        OPL3_PhaseGenerate(&chip->slot[ii]);
        OPL3_SlotGenerate(&chip->slot[ii]);
    }

    if ((chip->timer & 0x3f) == 0x3f)
    {
        chip->tremolopos = (chip->tremolopos + 1) % 210;
    }
    if (chip->tremolopos < 105)
    {
        chip->tremolo = chip->tremolopos >> chip->tremoloshift;
    }
    else
    {
        chip->tremolo = (210 - chip->tremolopos) >> chip->tremoloshift;
    }

    if ((chip->timer & 0x3ff) == 0x3ff)
    {
        chip->vibpos = (chip->vibpos + 1) & 7;
    }

    chip->timer++;

    chip->eg_add = 0;
    if (chip->eg_timer)
    {
        while (shift < 36 && ((chip->eg_timer >> shift) & 1) == 0)
        {
            shift++;
        }
        if (shift > 12)
        {
            chip->eg_add = 0;
        }
        else
        {
            chip->eg_add = shift + 1;
        }
    }

    if (chip->eg_timerrem || chip->eg_state)
    {
        if (chip->eg_timer == 0xfffffffff)
        {
            chip->eg_timer = 0;
            chip->eg_timerrem = 1;
        }
        else
        {
            chip->eg_timer++;
            chip->eg_timerrem = 0;
        }
    }

    chip->eg_state ^= 1;

    while (chip->writebuf[chip->writebuf_cur].time <= chip->writebuf_samplecnt)
    {
        if (!(chip->writebuf[chip->writebuf_cur].reg & 0x200))
        {
            break;
        }
        chip->writebuf[chip->writebuf_cur].reg &= 0x1ff;
        OPL3_WriteReg(chip, chip->writebuf[chip->writebuf_cur].reg,
                      chip->writebuf[chip->writebuf_cur].data);
        chip->writebuf_cur = (chip->writebuf_cur + 1) % OPL_WRITEBUF_SIZE;
    }
    chip->writebuf_samplecnt++;
}

void OPL3_GenerateResampled(opl3_chip *chip, Bit16s *buf)
{
    while (chip->samplecnt >= chip->rateratio)
    {
        chip->oldsamples[0] = chip->samples[0];
        chip->oldsamples[1] = chip->samples[1];
        OPL3_Generate(chip, chip->samples);
        chip->samplecnt -= chip->rateratio;
    }
    buf[0] = (Bit16s)((chip->oldsamples[0] * (chip->rateratio - chip->samplecnt)
                     + chip->samples[0] * chip->samplecnt) / chip->rateratio);
    buf[1] = (Bit16s)((chip->oldsamples[1] * (chip->rateratio - chip->samplecnt)
                     + chip->samples[1] * chip->samplecnt) / chip->rateratio);
    chip->samplecnt += 1 << RSM_FRAC;
}

void OPL3_Reset(opl3_chip *chip, Bit32u samplerate)
{
    Bit8u slotnum;
    Bit8u channum;

    memset(chip, 0, sizeof(opl3_chip));
    for (slotnum = 0; slotnum < 36; slotnum++)
    {
        chip->slot[slotnum].chip = chip;
        chip->slot[slotnum].mod = &chip->zeromod;
        chip->slot[slotnum].eg_rout = 0x1ff;
        chip->slot[slotnum].eg_out = 0x1ff;
        chip->slot[slotnum].eg_gen = envelope_gen_num_release;
        chip->slot[slotnum].trem = (Bit8u*)&chip->zeromod;
        chip->slot[slotnum].slot_num = slotnum;
    }
    for (channum = 0; channum < 18; channum++)
    {
        chip->channel[channum].slots[0] = &chip->slot[ch_slot[channum]];
        chip->channel[channum].slots[1] = &chip->slot[ch_slot[channum] + 3];
        chip->slot[ch_slot[channum]].channel = &chip->channel[channum];
        chip->slot[ch_slot[channum] + 3].channel = &chip->channel[channum];
        if ((channum % 9) < 3)
        {
            chip->channel[channum].pair = &chip->channel[channum + 3];
        }
        else if ((channum % 9) < 6)
        {
            chip->channel[channum].pair = &chip->channel[channum - 3];
        }
        chip->channel[channum].chip = chip;
        chip->channel[channum].out[0] = &chip->zeromod;
        chip->channel[channum].out[1] = &chip->zeromod;
        chip->channel[channum].out[2] = &chip->zeromod;
        chip->channel[channum].out[3] = &chip->zeromod;
        chip->channel[channum].chtype = ch_2op;
        chip->channel[channum].cha = 0xffff;
        chip->channel[channum].chb = 0xffff;
        chip->channel[channum].ch_num = channum;
        OPL3_ChannelSetupAlg(&chip->channel[channum]);
    }
    chip->noise = 1;
    chip->rateratio = (samplerate << RSM_FRAC) / 49716;
    chip->tremoloshift = 4;
    chip->vibshift = 1;
}

void OPL3_WriteReg(opl3_chip *chip, Bit16u reg, Bit8u v)
{
    Bit8u high = (reg >> 8) & 0x01;
    Bit8u regm = reg & 0xff;
    switch (regm & 0xf0)
    {
    case 0x00:
        if (high)
        {
            switch (regm & 0x0f)
            {
            case 0x04:
                OPL3_ChannelSet4Op(chip, v);
                break;
            case 0x05:
                chip->newm = v & 0x01;
                break;
            }
        }
        else
        {
            switch (regm & 0x0f)
            {
            case 0x08:
                chip->nts = (v >> 6) & 0x01;
                break;
            }
        }
        break;
    case 0x20:
    case 0x30:
        if (ad_slot[regm & 0x1f] >= 0)
        {
            OPL3_SlotWrite20(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0x40:
    case 0x50:
        if (ad_slot[regm & 0x1f] >= 0)
        {
            OPL3_SlotWrite40(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0x60:
    case 0x70:
        if (ad_slot[regm & 0x1f] >= 0)
        {
            OPL3_SlotWrite60(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0x80:
    case 0x90:
        if (ad_slot[regm & 0x1f] >= 0)
        {
            OPL3_SlotWrite80(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0xe0:
    case 0xf0:
        if (ad_slot[regm & 0x1f] >= 0)
        {
            OPL3_SlotWriteE0(&chip->slot[18 * high + ad_slot[regm & 0x1f]], v);
        }
        break;
    case 0xa0:
        if ((regm & 0x0f) < 9)
        {
            OPL3_ChannelWriteA0(&chip->channel[9 * high + (regm & 0x0f)], v);
        }
        break;
    case 0xb0:
        if (regm == 0xbd && !high)
        {
            chip->tremoloshift = (((v >> 7) ^ 1) << 1) + 2;
            chip->vibshift = ((v >> 6) & 0x01) ^ 1;
            OPL3_ChannelUpdateRhythm(chip, v);
        }
        else if ((regm & 0x0f) < 9)
        {
            OPL3_ChannelWriteB0(&chip->channel[9 * high + (regm & 0x0f)], v);
            if (v & 0x20)
            {
                OPL3_ChannelKeyOn(&chip->channel[9 * high + (regm & 0x0f)]);
            }
            else
            {
                OPL3_ChannelKeyOff(&chip->channel[9 * high + (regm & 0x0f)]);
            }
        }
        break;
    case 0xc0:
        if ((regm & 0x0f) < 9)
        {
            OPL3_ChannelWriteC0(&chip->channel[9 * high + (regm & 0x0f)], v);
        }
        break;
    }
}

void OPL3_WriteRegBuffered(opl3_chip *chip, Bit16u reg, Bit8u v)
{
    Bit64u time1, time2;

    if (chip->writebuf[chip->writebuf_last].reg & 0x200)
    {
        OPL3_WriteReg(chip, chip->writebuf[chip->writebuf_last].reg & 0x1ff,
                      chip->writebuf[chip->writebuf_last].data);

        chip->writebuf_cur = (chip->writebuf_last + 1) % OPL_WRITEBUF_SIZE;
        chip->writebuf_samplecnt = chip->writebuf[chip->writebuf_last].time;
    }

    chip->writebuf[chip->writebuf_last].reg = reg | 0x200;
    chip->writebuf[chip->writebuf_last].data = v;
    time1 = chip->writebuf_lasttime + OPL_WRITEBUF_DELAY;
    time2 = chip->writebuf_samplecnt;

    if (time1 < time2)
    {
        time1 = time2;
    }

    chip->writebuf[chip->writebuf_last].time = time1;
    chip->writebuf_lasttime = time1;
    chip->writebuf_last = (chip->writebuf_last + 1) % OPL_WRITEBUF_SIZE;
}

void OPL3_GenerateStream(opl3_chip *chip, Bit16s *sndptr, Bit32u numsamples)
{
    Bit32u i;

    for(i = 0; i < numsamples; i++)
    {
        OPL3_GenerateResampled(chip, sndptr);
        sndptr += 2;
    }
}
