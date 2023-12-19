pub struct Rect {
    top: i32,
    bot: i32,
    left: i32,
    right: i32,
}

#[no_mangle]
pub extern "C" fn M_ClearBox(rectp: *mut Rect) {
    unsafe {
        let rect = &mut *rectp;

        rect.top = i32::MIN;
        rect.right = i32::MIN;
        rect.left = i32::MAX;
        rect.bot = i32::MAX;
    }
}

#[no_mangle]
pub extern "C" fn M_AddToBox(rectp: *mut Rect, x: i32, y: i32) {
    unsafe {
        let rect = &mut *rectp;

        if x < rect.left {
            rect.left = x;
        } else if rect.right < x {
            rect.right = x;
        }

        if y < rect.bot {
            rect.bot = y;
        } else if rect.top < y {
            rect.top = y;
        }
    }
}
