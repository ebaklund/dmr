
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
pub struct DmrArgs {
   #[arg(long, num_args(1))]
   iwad: Option<String>,

   #[arg(long, num_args(1))]
   response: Option<String>,
}

impl DmrArgs
{
    pub fn new() -> Result<DmrArgs, String>
    {
        let parse_res = <DmrArgs as clap::Parser>::try_parse();

        match parse_res
        {
            Ok(dmr_args) => dmr_args.try_update_from_response_file(),
            Err(err) => Err(err.to_string())
        }
    }

    // PRIVATE HELPERS

    fn try_update_from_response_file(mut self: DmrArgs) -> Result<DmrArgs, String>
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
