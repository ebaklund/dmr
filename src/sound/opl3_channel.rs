
use super::opl3_slot::Opl3Slot;
use super::opl3_chip::Opl3Chip;

use libc::c_void;
use std::cmp::max;

// PRIVATE

// Key Scaling Level used to adjust the volume of a note based on its pitch.

static  KSL_ROM: [u8; 16] = [
    0, 32, 40, 45, 48, 51, 53, 55, 56, 58, 59, 60, 61, 62, 63, 64
];

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

// PUBLIC

impl Opl3Channel {
    pub fn get_kls(&self) -> u8 {
        unsafe {
            let ksl = 
                (KSL_ROM[(self.f_num >> 6) as usize] << 2)
                - ((0x08 - self.block) << 5);

            max(ksl, 0)
        }
    }
}
