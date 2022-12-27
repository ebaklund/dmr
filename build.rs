
fn main() {
    cc::Build::new()
        .file("dmc/src/aes_prng.c")
        .define("FOO", Some("bar"))
        .include("dmc/src")
        .compile("dmc");
}