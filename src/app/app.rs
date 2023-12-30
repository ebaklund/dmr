

use clap::{command, Arg, ArgMatches};
use crate::game::d_game::DGame;
use crate::data::w_wad::*;
use crate::data::data_config;
use crate::data::data_config::*;
use crate::app::argv::DArgv;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

// PRIVATE

fn get_data_config() -> Result<DataConfig, String> {
    let args = command!()
    .arg(Arg::new("iwad").long("iwad").env("DOOM_IWAD_PATH").required(true))
    .get_matches();

    let iwad_path = args.get_one::<String>("iwad")
    .ok_or_else(|| "Failed to obtain iwad path from command line.".to_string())?
    .clone();

    Ok(DataConfig {
        iwad_path,
    })
}

// PUBLIC

pub fn app_main() -> Result<(), String> {
    let _dargv = DArgv::try_create()?;
    let data_config = get_data_config()?;

    let _wad = Wad::new(&data_config.iwad_path)?;

    let _dgame = DGame::new();

    _dgame.main(&data_config.iwad_path)?;

    Ok(())
}
