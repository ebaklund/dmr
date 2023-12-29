
pub mod app;
pub mod basics;
pub mod data;
pub mod game;
pub mod sound;

use crate::app::app::app_main;

// The purpose of this file is to export modules so that they can be tested.
// To run the game, we have app::app_main.

pub fn lib_main() -> Result<(), String> {
    app_main()
}


