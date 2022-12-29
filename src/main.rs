extern crate libc;

use std::ffi::CString;
use libc::c_char;
use libc::c_int;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

//#[link(name="i_main")]
#[link(name = "dmc", kind = "static")]
extern "C"
{
    fn i_main(argc: c_int, argv: *const *const c_char) -> c_int;
}

fn main()
{
    println!("Hello, world!");

    let pinned_args = std::env::args()
        .map(|arg| CString::new(arg).unwrap())
        .collect::<Vec<CString>>();

    let argv: Vec<*const c_char> = pinned_args.iter().map(|s| s.as_ptr()).collect();

    unsafe { i_main(argv.len() as c_int, argv.as_ptr()) };
}
