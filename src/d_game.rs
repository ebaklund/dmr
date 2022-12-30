
#[link(name = "dmc", kind = "static")]
extern "C"
{
    fn D_DoomMain ();
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
        unsafe
        {
            D_DoomMain();
        }
    }
}
