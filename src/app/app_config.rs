
use clap::{command, Command, Arg, ArgMatches};
use crate::data::data_config::*;
use crate::gui::gui_config::*;

// https://stackoverflow.com/questions/69437925/problem-with-calling-c-function-that-receive-command-line-arguments-from-rust

// PRIVATE

trait WithArgs {
    fn with_data_args(self) -> Command;
    fn with_gui_args(self) -> Command;
}

impl WithArgs for Command {
    fn with_data_args(self) -> Command {
        self.arg(
            Arg::new("iwad").long("iwad").env("DOOM_IWAD_PATH")
            .value_parser(clap::value_parser!(String))
            .required(true)
        )
    }

    fn with_gui_args(self) -> Command {
        self.arg(
            Arg::new("fullscreen").long("fullscreen").env("DOOM_FULLSCREEN")
            .value_parser(clap::value_parser!(bool)).default_value("true")
            .required(false)
        )
    }
}

trait GetConfig {
    fn get_data_config(&self) -> Result<DataConfig, String>;
    fn get_gui_config(&self) ->  Result<GuiConfig, String>;
}

impl GetConfig for ArgMatches {
    fn get_data_config(&self) -> Result<DataConfig, String> {

        let iwad_path = self.get_one::<String>("iwad")
        .ok_or_else(|| "Failed to obtain iwad path from command line.".to_string())?
        .clone();

        Ok(DataConfig {
            iwad_path,
        })
    }

    fn get_gui_config(&self) ->  Result<GuiConfig, String> {

        let fullscreen = self.get_one::<bool>("fullscreen")
        .ok_or_else(|| "Failed to obtain fullscreeen setting from command line.".to_string())?
        .clone();

        Ok(GuiConfig {
            fullscreen,
        })
    }
}

// PUBLIC

pub struct AppConfig {
    pub data_config: DataConfig,
    pub gui_config: GuiConfig
}

pub fn get_app_config() ->  Result<AppConfig, String> {

    let cmd = command!()
        .with_data_args()
        .with_gui_args();
    
    let arg_matches = cmd.get_matches();

    Ok(AppConfig {
        data_config: arg_matches.get_data_config()?,
        gui_config: arg_matches.get_gui_config()?,
    })
} 
