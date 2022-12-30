
extern crate libc;
use libc::c_char;
use libc::c_int;

#[link(name = "dmc", kind = "static")]
extern "C"
{
    fn Z_Init();
    fn D_DoomMain();
}


pub struct DGame
{

}

impl DGame
{
    pub fn new() -> DGame
    {
        DGame
        {

        }
    }

    pub fn main(self: &Self)
    {
        println!("Z_Init: Init zone memory allocation daemon.");

        unsafe
        {
            Z_Init();
            D_DoomMain();
        }
    }
}
