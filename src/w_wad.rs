use std::array::TryFromSliceError;
use std::fs::File;
use std::io::Read;
use std::mem::size_of;

// PRIVATE

fn read_wad_buffer(wad_path: &str) -> Result<Vec<u8>, String> {
    let mut wad_file = File::open(wad_path)
        .map_err(|err| format!("Failed to open file {} because {}.", wad_path, err))?;

    let mut wad_buffer = Vec::new();

    wad_file
        .read_to_end(&mut wad_buffer)
        .map_err(|err| format!("Failed to read wad file {} because {}.", wad_path, err))?;

    Ok(wad_buffer)
}

fn extract_string(buffer: &Vec<u8>, pos: usize, size: usize) -> Result<String, String> {
    let vec = buffer[pos..pos + size].to_vec();
    let str_res = String::from_utf8(vec);

    str_res.map_err(|err| err.to_string())
}

fn extract_i32(buffer: &Vec<u8>, pos: usize) -> Result<i32, String> {
    let bytes = buffer[pos..pos + 4]
        .try_into()
        .map_err(|err: TryFromSliceError| err.to_string())?;

    Ok(i32::from_le_bytes(bytes))
}

fn extract_i32_as_usize(buffer: &Vec<u8>, pos: usize) -> Result<usize, String> {
    let i = extract_i32(buffer, pos)?;

    match i >= 0 {
        true => Ok(i as usize),
        false => Err(format!("Cannot convert {} to usize.", i)),
    }
}

fn extract_wad_header(wad_buffer: &Vec<u8>) -> Result<WadHeader, String> {
    Ok(WadHeader {
        wad_type: extract_string(wad_buffer, 0, 4)?,
        lump_count: extract_i32_as_usize(wad_buffer, 4)?,
        lumps_offset: extract_i32_as_usize(wad_buffer, 8)?,
    })
}

fn extract_wad_lump(wad_buffer: &Vec<u8>, pos: usize) -> Result<WadLump, String> {
    Ok(WadLump {
        pos: extract_i32_as_usize(wad_buffer, pos)?,
        size: extract_i32_as_usize(wad_buffer, pos + 4)?,
        name: extract_string(wad_buffer, pos + 8, 8 - 1)?,
    })
}

fn extract_wad_body(wad_buffer: &Vec<u8>, header: &WadHeader) -> Result<Vec<WadLump>, String> {
    let mut body = Vec::<WadLump>::with_capacity(header.lump_count);
    let mut lump_pos: usize = header.lumps_offset;

    for _ in 0..header.lump_count {
        let lump = extract_wad_lump(&wad_buffer, lump_pos)?;
        lump_pos += size_of::<FileLump>();
        body.push(lump);
    }

    Ok(body)
}

struct WadHeader {
    wad_type: String, // Should be "IWAD" or "PWAD".
    lump_count: usize,
    lumps_offset: usize,
}

struct FileLump {
    pos: i32,
    size: i32,
    name: [u8; 8],
}

struct WadLump {
    pos: usize,
    size: usize,
    name: String,
}

// PUBLIC

pub struct Wad {
    buffer: Vec<u8>,
    header: WadHeader,
    body: Vec<WadLump>,
}

impl Wad {
    pub fn new(wad_path: &str) -> Result<Wad, String> {
        let buffer = read_wad_buffer(wad_path)?;
        let header = extract_wad_header(&buffer)?;
        let body = extract_wad_body(&buffer, &header)?;

        Ok(Wad {
            buffer,
            header,
            body,
        })
    }
}
