#[no_mangle]
pub extern "C" fn FixedMul(a: i32, b: i32) -> i32 {
    ((i64::from(a) * i64::from(b)) >> 16) as i32
}

#[no_mangle]
pub extern "C" fn FixedDiv(a: i32, b: i32) -> i32 {
    if (a.abs() >> 14) >= b.abs() {
        if a ^ b < 0 {
            i32::MAX
        } else {
            i32::MAX
        }
    } else {
        ((i64::from(a) << 16) / i64::from(b)) as i32
    }
}
