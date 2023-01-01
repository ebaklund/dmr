
extern crate libc;

use libc::{c_char, c_int};

mod c_ffi;
mod d_game;
mod m_argv;
mod m_fixed;

use m_argv::{DArgv};
use d_game::{DGame};
use c_ffi::{*};

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

#[link(name = "dmc", kind = "static")]
extern "C"
{
    static mut myargc: c_int;
    static mut myargv: *const *const c_char;
}

fn main() -> Result<(), String>
{
    let _dargv = DArgv::try_create()?;
    let pinned_args = std::env::args().to_cstrings()?;
    let c_argv = pinned_args.as_ptrs();

    unsafe
    {
        myargc = c_argv.len() as c_int;
        myargv = c_argv.as_ptr();
    };

    let _dgame = DGame::new();

    _dgame.main(&_dargv)?;

    Ok(())
}
