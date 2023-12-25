
extern crate libc; 
use libc::c_void;

use std::collections::BTreeSet;
use std::cmp::Ordering;

// PRIVATE

type OplCallback = fn(*mut c_void);

struct OplQueueEntry {
    callback: OplCallback,
    data: *mut c_void,
    time: u64
}

impl PartialEq for OplQueueEntry {
    fn eq(&self, other: &Self) -> bool {
        self.time == other.time
    }
}

impl Eq for OplQueueEntry {
}

impl PartialOrd for OplQueueEntry {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for OplQueueEntry {
    fn cmp(&self, other: &Self) -> Ordering {
        self.time.cmp(&other.time)
    }
}

#[repr(C)] 
pub struct OplQueue {
    entries: BTreeSet::<OplQueueEntry>
}

impl OplQueue {
    pub fn new() -> OplQueue {
        OplQueue {
            entries:  BTreeSet::<OplQueueEntry>::new()
        }
    }
}

#[no_mangle]
pub extern "C" fn OPL_Queue_Create() -> *mut c_void {
    Box::into_raw(Box::new(OplQueue::new())) as *mut c_void
}

#[no_mangle]
pub extern "C" fn OPL_Queue_IsEmpty(qptr: *mut c_void) -> i32 {
    unsafe {
        (&mut *(qptr as *mut OplQueue)).entries.is_empty() as i32
    }
}

#[no_mangle]
pub extern "C" fn OPL_Queue_Clear(qptr: *mut c_void) {
    unsafe {
        (&mut *(qptr as *mut OplQueue)).entries.clear();
    }
}

#[no_mangle]
pub extern "C" fn OPL_Queue_Destroy(qptr: *mut c_void) {
    unsafe {
        let _ = Box::from_raw(qptr as *mut OplQueue);
    }
}

#[no_mangle]
pub extern "C" fn OPL_Queue_Push(qptr: *mut c_void, callback: OplCallback, data: *mut c_void, time: u64) {
    unsafe {
        (&mut *(qptr as *mut OplQueue)).entries.insert(OplQueueEntry { callback, data, time });
    }
}

#[no_mangle]
pub extern "C" fn OPL_Queue_Pop(qptr: *mut c_void, callback: *mut OplCallback, data: *mut *mut c_void) -> i32 {
    unsafe {
        if let Some(first) = (&mut *(qptr as *mut OplQueue)).entries.pop_first() {
            ( *callback, *data ) = ( first.callback, first.data );
            true as i32
        } else {
            false as i32
        }
    }
}

#[no_mangle]
pub extern "C" fn OPL_Queue_NextTimeOrInf(qptr: *mut c_void) -> u64 {
    unsafe {
        if let Some(first) = (&mut *(qptr as *mut OplQueue)).entries.first() { 
            first.time 
        } else { 
            u64::MAX 
        }
    }
}

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
