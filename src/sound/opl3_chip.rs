
use super::opl3_channel::Opl3Channel;
use super::opl3_slot::Opl3Slot;
use super::opl3_writebuf::Opl3Writebuf;

use libc::c_void;

// PUBLIC

#[derive(Copy, Clone)] 
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

impl Opl3Chip {
    
    pub fn new() -> Opl3Chip {
        Opl3Chip {
            channel: [Opl3Channel::new(); 18],
            slot: [Opl3Slot::new(); 36],
            timer: 0,
            eg_timer: 0,
            eg_timerrem: 0,
            eg_state: 0,
            eg_add: 0,
            newm: 0,
            nts: 0,
            rhy: 0,
            vibpos: 0,
            vibshift: 0,
            tremolo: 0,
            tremolopos: 0,
            tremoloshift: 0,
            noise: 0,
            zeromod: 0,
            mixbuff: [0; 2],
            rm_hh_bit2: 0, // Hi-Hat Key-On
            rm_hh_bit3: 0, // Hi-Hat Key-On
            rm_hh_bit7: 0, // Hi-Hat Key-On
            rm_hh_bit8: 0, // Hi-Hat Key-On
            rm_tc_bit3: 0, // Top Cymbal Key-On
            rm_tc_bit5: 0, // Top Cymbal Key-On
            //OPL3L
            rateratio: 0,
            samplecnt: 0,
            oldsamples: [0; 2],
            samples: [0; 2],
        
            writebuf_samplecnt: 0,
            writebuf_cur: 0,
            writebuf_last: 0,
            writebuf_lasttime: 0,
            writebuf: [Opl3Writebuf::new(); 1024],            
        }
    }
}

// PUBLIC

#[no_mangle]
pub extern "C" fn OPL_Chip_Create() -> *mut c_void {
    Box::into_raw(Box::new(Opl3Chip::new())) as *mut c_void
}

#[no_mangle]
pub extern "C" fn OPL_Chip_Destroy(qptr: *mut c_void) {
    unsafe {
        let _ = Box::from_raw(qptr as *mut Opl3Chip);
    }
}