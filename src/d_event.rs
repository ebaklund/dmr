
#![allow(non_upper_case_globals)]

use std::ptr;
use std::default::{Default};


#[derive(Default, Copy, Clone)]
pub struct Event
{
    _data: [i32; 6]
}

static FRESH_EVENT: Event = Event{ _data: [0; 6] };

static mut eventhead: usize = 0;
static mut eventtail: usize = 0;
static mut events: [Event; 64] = [FRESH_EVENT; 64];

#[no_mangle]
pub extern "C" fn D_PostEvent(ev: *const Event)
{
    unsafe
    {
        events[eventhead] = *ev;
        eventhead = (eventhead + 1) % events.len();
    }
}

#[no_mangle]
pub extern "C" fn D_PopEvent() -> *const Event
{
    unsafe
    {

        if eventtail == eventhead
        {
            return ptr::null();
        }

        let ev = &events[eventtail];
        eventtail = (eventtail + 1) % events.len();

        return ev;
    }
}
