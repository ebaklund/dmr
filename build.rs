
use std::env;
use std::fs;
use std::path::PathBuf;

fn c_files_in(dir: &str) -> Vec<PathBuf>
{
    return fs::read_dir(dir).unwrap()
        .map(|res| res.unwrap().path())
        .filter(|path| !path.is_dir() && path.extension().unwrap() == "c")
        .collect::<Vec<PathBuf>>();
}


fn main()
{
    env::set_var("CFLAGS", "-Wno-unused-but-set-parameter");

    // https://docs.rs/cc/latest/cc/

    //#[cfg(feature = "static")]
    cc::Build::new()
        .static_flag(true)
        .include("/usr/include/SDL2")
        .include("dmc/src")
        .include("dmc/pcsound")
        .include("dmc/textscreen")
        .include("dmc/opl")
        .files(c_files_in("./dmc/src"))
        .files(c_files_in("./dmc/src/doom"))
        .files(c_files_in("./dmc/src/setup"))
        .files(c_files_in("./dmc/opl"))
        .files(c_files_in("./dmc/pcsound"))
        .files(c_files_in("./dmc/textscreen"))
        .compile("dmc");

        // https://doc.rust-lang.org/cargo/reference/build-scripts.html#rustc-link-arg
        println!("cargo:rustc-link-search=/usr/lib/x86_64-linux-gnu");
        println!("cargo:rustc-link-lib=SDL2");
        println!("cargo:rustc-link-lib=SDL2_mixer");
        println!("cargo:rustc-link-lib=z");
        println!("cargo:rustc-link-lib=png");
}
