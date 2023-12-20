
#[cfg(test)]
mod Given_a_wad_file {

    pub fn wad_path() -> String {
        "doom-wad-shareware-1.9.fixed/doom1.wad".to_string()
    }

    mod when_reading_it {

        use dmr::w_wad::Wad;
        
        pub fn read_wad() -> Result<Wad, String> {
            Wad::new(&super::wad_path())
        }

        mod then {
            use assertor::*;

            #[test]
            fn wad_is_read() {
                assert_that!(super::read_wad()).is_ok();
            }        
        }
    }
}
