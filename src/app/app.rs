

use clap::{command, Arg};
use crate::game::d_game::DGame;
use crate::data::w_wad::*;
use crate::app::app_config::*;
use crate::app::argv::DArgv;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

// PUBLIC

pub fn app_main() -> Result<(), String> {
    //let _dargv = DArgv::try_create()?;
    let app_config = get_app_config()?;

    let _wad = Wad::new(&app_config.data_config.iwad_path)?;

    let _dgame = DGame::new();

    _dgame.main(&app_config.data_config.iwad_path)?;

    Ok(())
}
