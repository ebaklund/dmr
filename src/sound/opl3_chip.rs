
use super::opl3_channel::Opl3Channel;
use super::opl3_slot::Opl3Slot;

// PRIVATE

pub struct Opl3Writebuf {
    time: u64,
    reg: u16,
    data: u8,
}

// PUBLIC

pub struct Opl3Chip {
    channel: [Opl3Channel; 18],
    slot: [Opl3Slot; 36],
    timer: u16,
    eg_timer: u64,
    eg_timerrem: u8,
    eg_state: u8,
    eg_add: u8,
    newm: u8,
    nts: u8,
    rhy: u8,
    vibpos: u8,
    vibshift: u8,
    tremolo: u8,
    tremolopos: u8,
    tremoloshift: u8,
    noise: u32,
    zeromod: i16,
    mixbuff: [i32; 2],
    rm_hh_bit2: u8, // Hi-Hat Key-On
    rm_hh_bit3: u8, // Hi-Hat Key-On
    rm_hh_bit7: u8, // Hi-Hat Key-On
    rm_hh_bit8: u8, // Hi-Hat Key-On
    rm_tc_bit3: u8, // Top Cymbal Key-On
    rm_tc_bit5: u8, // Top Cymbal Key-On
    //OPL3L
    rateratio: i32,
    samplecnt: i32,
    oldsamples: [i16; 2],
    samples: [i16; 2],

    writebuf_samplecnt: u64,
    writebuf_cur: u32,
    writebuf_last: u32,
    writebuf_lasttime: u64,
    writebuf: [Opl3Writebuf; 1024],
}
