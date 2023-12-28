
// PRIVATE

#[derive(Copy, Clone)] 
pub struct Opl3Writebuf {
    time: u64,
    reg: u16,
    data: u8,
}

impl Opl3Writebuf {
    pub fn new() -> Opl3Writebuf {
        Opl3Writebuf {
            time: 0,
            reg: 0,
            data: 0,
        }  
    }
}