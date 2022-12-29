
https://github.com/fabiangreffrath/crispy-doom

### Preparations
Download source code and install prerequisites
```
$ git clone https://github.com/ebaklund/dmr.git
```

### Build dmr
```
cargo clean; cargo build
```

### WAD
```
$ cd dmr
wget http://cdn.debian.net/debian/pool/non-free/d/doom-wad-shareware/doom-wad-shareware_1.9.fixed.orig.tar.gz
gunzip doom-wad-shareware_1.9.fixed.orig.tar.gz
tar -xf doom-wad-shareware_1.9.fixed.orig.tar
```

### Run
```
cd dmr
cargo run -- -iwad doom-wad-shareware-1.9.fixed/doom1.wad
```
