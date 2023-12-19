extern crate libc;

use clap::{command, Arg};

mod c_ffi;
mod d_event;
mod d_game;
mod m_argv;
mod m_box;
mod m_fixed;
mod w_wad;

use d_game::DGame;
use m_argv::DArgv;
use w_wad::*;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

fn main() -> Result<(), String> {
    let _dargv = DArgv::try_create()?;

    let args = command!()
        .arg(Arg::new("iwad").long("iwad").required(true))
        .get_matches();

    let iwad_path = args
        .get_one::<String>("iwad")
        .ok_or_else(|| "Failed to get iwad arg.".to_string())?;

    let _wad = Wad::new(&iwad_path)?;

    let _dgame = DGame::new();

    _dgame.main(&_dargv)?;

    Ok(())
}
