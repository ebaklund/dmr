
use std::iter::Iterator;
use std::path::PathBuf;
use std::fs::{File};
use std::io::{prelude::*, BufReader};
use clap::{Parser};

// https://crates.io/crates/clap
// https://github.com/clap-rs/clap
// https://docs.rs/clap/latest/clap/all.html
// https://docs.rs/clap/latest/clap/_derive/index.html
// https://blog.rng0.io/compile-time-feature-flags-in-rust-why-how-when

#[derive(Parser, Debug)]
pub struct DArgv
{
    // d_iwad

    #[arg(long, required(true), num_args(1),
      help("Load game from given IWAD."))]
    pub iwad: String,

    // i_musicpack

    #[arg(long, num_args(1),
      help("Dump MIDI example substitution to specified file."),
      long_help("Read all MIDI files from loaded WAD files, dump an example substitution music config file to the specified filename and quit."))]
    pub dumpsubstconfig: Option<String>,

    // i_video

    #[arg(long, num_args(1), default_value("2014"),
      help("Sets the width of the window. Implies -window."))]
    pub width: u32,

    #[arg(long, num_args(1), default_value("2014"),
      help("Sets the height of the window. Implies -window."))]
    pub height: u32,

    // m_argv

    #[arg(long, num_args(1),
      help("Merge command line arguments from the given response file."))]
    pub response: Option<String>,

    // m_config

    #[arg(long, num_args(1),
      help("Load main configuration from the specified file, instead of the default."))]
    pub config: Option<String>,

    #[arg(long, num_args(1),
      help("Load extra configuration from the specified file, instead of the extra default."))]
    pub extraconfig: Option<String>,

    // w_main

    #[arg(long, trailing_var_arg(true), value_delimiter(','),
      help("Merge one or more PWAD file into the main IWAD, deutex's merge style."))]
    pub merge: Option<Vec<String>>,

    #[arg(long, trailing_var_arg(true), value_delimiter(','),
      help("Merge one or more PWAD file into the main IWAD, NWT's merge style."))]
    pub nwtmerge: Option<Vec<String>>,

    #[arg(long, trailing_var_arg(true), value_delimiter(','),
      help("Merge flats into the main IWAD directory, NWT's merge style."))]
    pub nwtaf: Option<Vec<String>>,

    #[arg(long, trailing_var_arg(true), value_delimiter(','),
      help("Merge sprites into the main IWAD directory, NWT's merge style."))]
    pub nwtas: Option<Vec<String>>,

    #[arg(long, trailing_var_arg(true), value_delimiter(','),
      help("Load the specified PWAD files."))]
    pub file: Option<Vec<String>>,

    // d_main

    #[arg(long, num_args(1), default_value("1.9"), value_parser(["1.2", "1.666", "1.7", "1.8", "1.9"]),
      help("Emulate a specific version of Doom."))]
    pub gameversion: String,

    #[arg(long, num_args(1),
      help("Dump IWAD file merged with PWADs into given file. See: --merge and --nwtmerge"))]
    pub mergedump: Option<String>,

    #[arg(long, num_args(1),
      help("Play back the given demo file (*.lmp)."))]
    pub playdemo: Option<String>,

    #[arg(long, num_args(1),
      help("Play back the given demo file while tracking framerate (*.lmp)."))]
    pub timedemo: Option<String>,

    #[arg(long, num_args(1), value_parser(0..=5),
      help("Set game skill (1: easiest - 5: hardest). 0 disables all monsters."))]
    pub skill: Option<u32>,

    #[arg(long, num_args(1), value_parser(1..=4),
      help("Set game episode to start on (1-4)."))]
    pub episode: Option<u32>,

    #[arg(long, num_args(1), default_value("20"),
      help("Set level time limit for multiplayer games in minutes."))]
    pub timer: Option<u32>,
}

impl DArgv
{
    pub fn try_create() -> Result<DArgv, String>
    {
        let parse_res = <DArgv as clap::Parser>::try_parse();

        match parse_res
        {
            Ok(dmr_args) => dmr_args.try_update_from_response_file(),
            Err(err) => Err(err.to_string())
        }
    }

    // PRIVATE HELPERS

    fn try_update_from_response_file(mut self: DArgv) -> Result<DArgv, String>
    {
        match self.get_response_path()?
        {
            Some(path) =>
            {
                let file =  match File::open(path)
                {
                    Ok(file) => Ok(file),
                    Err(str) => Err(str.to_string())
                }?;

                let mut reader = BufReader::new(file);
                let words = get_response_words(&mut reader);

                match self.try_update_from(words)
                {
                    Ok(_) =>  Ok(self),
                    Err(err) => Err(err.to_string())
                }
            },
            None => Ok(self)
        }
    }

    fn get_response_path(self: &Self) -> Result<Option<PathBuf>, String>
    {
        match &self.response
        {
            Some(file_name) =>
            {
                let path = PathBuf::from(file_name);

                if !path.exists()
                {
                    return Err(format!("Path {} does not exist.", file_name))
                }

                if !path.is_file()
                {
                    return Err(format!("Path {} is not a file.", file_name))
                }

                Ok(Some(path))
            },
            _ => Ok(None)
        }
    }
}

fn get_response_words(reader: &mut BufReader<File>) -> Vec<String>
{
    let lines = reader
        .lines()
        .filter_map(|line| line.ok())
        .collect::<Vec<String>>();

    let words = lines
        .iter()
        .flat_map(|line| line.split_whitespace())
        .map(|word| word.to_string())
        .collect::<Vec<String>>();

    words
}
