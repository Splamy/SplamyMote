# Follow
https://crates.io/crates/esp8266-hal

- `git clone https://github.com/MabezDev/rust-xtensa`
- `cd rust-xtensa`
- *In Linux subsystem* `./configure --experimental-targets=Xtensa`
- `python ./x.py build`

- `rustup toolchain link xtensa /path/to/rust-xtensa/build/x86_64-unknown-linux-gnu/stage1`
   => `rustup toolchain link xtensa E:/_Projects/rust-xtensa/build/x86_64-pc-windows-msvc/stage0`

Download from
https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/windows-setup.html

put `D:\xtensa-lx106-elf`

add to path:
`D:\xtensa-lx106-elf\bin`
