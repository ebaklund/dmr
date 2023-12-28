
use std::ptr::null_mut;

use libc::c_void;

use super::opl3_channel::Opl3Channel;
use super::opl3_chip::Opl3Chip;

// PRIVATE

#[derive(Copy, Clone)]
pub struct Opl3Slot {
    channel: *mut Opl3Channel,
    chip: *mut Opl3Chip,
    out: i16,
    fbmod: i16,
    mmod: *mut i16,
    prout: i16,
    eg_rout: i16,
    eg_out: i16,
    eg_inc: u8,
    eg_gen: u8,
    eg_rate: u8,
    eg_ksl: u8,
    trem: *mut u8,
    reg_vib: u8,     // Vibrato Enable
    reg_type: u8,    // Envelope Generator Type
    reg_ksr: u8,     // “Key Scaling of Rate” Value
    reg_mult: u8,    // Frequency Multiplier Value
    reg_ksl: u8,     // “Key Scaling of Level” Value
    reg_tl: u8,      // Total Level Value
    reg_ar: u8,      // Attack Rate
    reg_dr: u8,      //	Decay Rate
    reg_sl: u8,      // Sustain Level Value
    reg_rr: u8,      // Release Rate
    reg_wf: u8,      // Waveform Selection
    key: u8,
    pg_reset: u32,
    pg_phase: u32,
    pg_phase_out: u16,
    slot_num: u8,
}

impl Opl3Slot {

    pub fn new() -> Opl3Slot {
        Opl3Slot {
            channel: null_mut(),
            chip: null_mut(),
            out: 0,
            fbmod: 0,
            mmod: null_mut(),
            prout: 0,
            eg_rout: 0,
            eg_out: 0,
            eg_inc: 0,
            eg_gen: 0,
            eg_rate: 0,
            eg_ksl: 0,
            trem: null_mut(),
            reg_vib: 0,
            reg_type: 0,
            reg_ksr: 0,
            reg_mult: 0,
            reg_ksl: 0,
            reg_tl: 0,
            reg_ar: 0,
            reg_dr: 0,
            reg_sl: 0,
            reg_rr: 0,
            reg_wf: 0,
            key: 0,
            pg_reset: 0,
            pg_phase: 0,
            pg_phase_out: 0,
            slot_num: 0,
        }
    }

    pub fn update_kls(&mut self) {
        unsafe {
            self.eg_ksl = (&*self.channel).get_kls();
        }
    }
}

// PUBLIC

#[no_mangle]
pub extern "C" fn OPL3_EnvelopeUpdateKSL(sptr: *mut c_void) {
    unsafe {
        (&mut *(sptr as *mut Opl3Slot)).update_kls();
    }
}
