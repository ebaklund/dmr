use super::opl3_slot::Opl3Slot;
use super::opl3_chip::Opl3Chip;

pub struct Opl3Channel {
    slots: *mut [Opl3Slot; 2],
    pair: *mut Opl3Channel,
    chip: *mut Opl3Chip,
    out: *mut [i16; 4],
    chtype: u8,
    f_num: u16,  // Frequency Value (high 2 bits)
    block: u8,   // Octave (Block) Value
    fb: u8,      // Feedback Depth
    con: u8,     // Connection Type
    alg: u8,
    ksv: u8,
    cha: u16,
    chb: u16,
    ch_num: u8,
}
