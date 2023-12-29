
use clap::{command, Arg};
use crate::game::d_game::DGame;
use crate::data::w_wad::*;
use crate::app::argv::DArgv;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

pub fn app_main() -> Result<(), String> {
    let _dargv = DArgv::try_create()?;

    let args = command!()
        .arg(Arg::new("iwad").long("iwad").required(true))
        .get_matches();

    let iwad_path = args
        .get_one::<String>("iwad")
        .ok_or_else(|| "Failed to get iwad arg.".to_string())?;

    let _wad = Wad::new(&iwad_path)?;

    let _dgame = DGame::new();

    _dgame.main(&iwad_path)?;

    Ok(())
}
