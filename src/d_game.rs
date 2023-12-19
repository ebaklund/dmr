extern crate libc;

use super::c_ffi::ToCString;
use super::m_argv::DArgv;
use libc::c_char;

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

    pub fn main(self: &Self, dargv: &DArgv) -> Result<(), String> {
        let iwadname = dargv.iwad.to_cstring()?;

        println!("Z_Init: Init zone memory allocation daemon.");

        unsafe {
            Z_Init();
            D_DoomMain(iwadname.as_ptr());
        }

        Ok(())
    }
}
