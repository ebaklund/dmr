extern crate libc;

use std::ffi::CString;
use libc::c_char;
use libc::c_int;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

//#[link(name="i_main")]
#[link(name = "dmc", kind = "static")]
extern "C"
{
    static mut myargc: c_int;
    static mut myargv: *const *const c_char;

    fn M_FindResponseFile();
    fn D_DoomMain ();
}

fn main()
{
    let pinned_args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();

    let argv: Vec<*const c_char> = pinned_args.iter().map(|s| s.as_ptr()).collect();

    unsafe
    {
        myargc = argv.len() as c_int;
        myargv = argv.as_ptr();

        M_FindResponseFile();
        D_DoomMain ();
    };
}
