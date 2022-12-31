
use std::ffi::CString;

use libc::{c_char};

pub trait ToCString
{
    fn to_cstring(&self) -> Result<CString, String>;
}

impl<T> ToCString for T where T: ToString
{
    fn to_cstring(&self)-> Result<CString, String>
    {
        CString::new(self.to_string()).map_err(|e| e.to_string())
    }
}

pub trait ToCStrings<I>
{
    fn to_cstrings(self) -> Result<Vec<CString>, String>;
}

impl<I, T> ToCStrings<I> for I where I: IntoIterator<Item=T>, T: ToString
{
    fn to_cstrings(self) -> Result<Vec<CString>, String>
    {
        let mut vec = Vec::<CString>::new();

        for e in self
        {
            vec.push(e.to_cstring()?);
        }

        Ok(vec)
    }
}

impl<'a, I, T> ToCStrings<I> for &'a mut I where I: Iterator<Item=&'a T>, T: ToString + 'a
{
    fn to_cstrings(self) -> Result<Vec<CString>, String>
    {
        let mut vec = Vec::<CString>::new();

        for e in self
        {
            vec.push(e.to_cstring()?);
        }

        Ok(vec)
    }
}

pub trait AsPtrs
{
    fn as_ptrs(&self) -> Vec<*const c_char>;
}

impl AsPtrs for Vec<CString>
{
    fn as_ptrs(&self) -> Vec<*const c_char>
    {
        self.iter().map(|s| s.as_ptr()).collect::<Vec<*const c_char>>()
    }
}