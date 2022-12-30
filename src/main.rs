
use std::ffi::CString;

extern crate libc;
use libc::c_char;
use libc::c_int;

mod m_argv;
use m_argv::{DArgv};

mod d_game;
use d_game::{DGame};

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

#[link(name = "dmc", kind = "static")]
extern "C"
{
    static mut myargc: c_int;
    static mut myargv: *const *const c_char;
}

fn main()
{
    let _dargv = DArgv::new();

    let args: Vec<String> = std::env::args().collect();

    let pinned_args: Vec<CString> = args
        .iter()
        .map(|s| CString::new(s.to_string()).unwrap())
        .collect();

    let c_argv: Vec<*const c_char> = pinned_args
        .iter()
        .map(|s| s.as_ptr())
        .collect();

    unsafe
    {
        myargc = c_argv.len() as c_int;
        myargv = c_argv.as_ptr();
    };

    let _dgame = DGame::new();

    _dgame.main();
}
