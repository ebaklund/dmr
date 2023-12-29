extern crate libc;
use libc::c_char;

use crate::basics::c_ffi::ToCString;

#[link(name = "dmc", kind = "static")]
extern "C" {
    fn Z_Init();
    fn D_DoomMain(iwadfile: *const c_char);
}

pub struct DGame {}

impl DGame {
    pub fn new() -> DGame {
        DGame {}
    }

    pub fn main(self: &Self, iwadname: &String) -> Result<(), String> {

        println!("Z_Init: Init zone memory allocation daemon.");

        unsafe {
            Z_Init();
            D_DoomMain(iwadname.to_cstring()?.as_ptr());
        }

        Ok(())
    }
}
