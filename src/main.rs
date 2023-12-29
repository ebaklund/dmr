
use dmr::lib_main;

// The purpose of this file is to provide an entry point for the operationg system.
// Otherwise it is empty because no code construct can escape main for testing purposes.
// To enable automatic testing, we have dmr::lib_main.
//
// https://stackoverflow.com/questions/75193675/module-cannot-be-found-from-tests-dir

fn main() -> Result<(), String> {
    lib_main()
}
