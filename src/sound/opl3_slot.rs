
use super::opl3_channel::Opl3Channel;
use super::opl3_chip::Opl3Chip;

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

/*
#[no_mangle]
pub extern "C" fn OPL_Queue_AdjustCallbacks(qptr: *mut c_void, time: u64, factor: f32) {
    unsafe {
        let queue = &mut *(qptr as *mut OplQueue);

        queue.entries = queue.entries.first().iter().map(|entry| {
            OplQueueEntry {
                callback: entry.callback,
                data: entry.data,
                time: time + ((entry.time - time) as f32 / factor) as u64
            }
        }).collect();
    }
}
*/