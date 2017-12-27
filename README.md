# Pill Duck: Scriptable USB HID device for STM32F103 blue pill

A keyboard/mouse USB HID device for the STM32F103 "blue pill" development board,
inspired by the [USB Rubber Ducky](https://hakshop.com/products/usb-rubber-ducky-deluxe).

**Hardware requirements**: Any of the ARM Cortex-M3 STM32F103 "minimum development boards" should work, I've tested with
this board which can be acquired for ~$1.70: [STM32F103C8T6 ARM STM32 Minimum System Development Board Module](https://www.aliexpress.com/item/STM32F103C8T6-ARM-STM32-Minimum-System-Development-Board-Module/32656040083.html)

**Building**: Install [ARM GCC Embedded Toolchain](https://launchpad.net/gcc-arm-embedded/) (if you're on macOS
and have [Homebrew](https://brew.sh), just run `brew cask install gcc-arm-embedded`) then run `make`.

**Installation**: Flash the `pill_duck.bin` binary file to the blue pill over serial.

**Usage**: Plug in the device to your PC over USB. It should show up as several device classes, including
a serial port (USB modem), on my system the device node is `/dev/cu.usbmodemAB2`. Connect to this serial port
e.g. using `screen -L /dev/cu.usbmodemAB2` then you can type various commands, if it works:

```
duck> v
Pill Duck version da646c9-dirty
duck>
```

Command help reference:

```
v	    show firmware version
w<hex>	write flash data
d<hex>	write compiled DuckyScript flash data
j	    write mouse jiggler to flash data
r	    read flash data
@	    show current report index
p	    pause/resume execution
s	    single step execution
z	    reset report index to zero
```

**Examples**: As a test, you can try installing the built-in mouse jiggler by typing `j` at the serial prompt.
The mouse should begin moving back at forth, keeping the system awake. To pause, type `p`. You can write raw HID
packets using the `w` command, or `d` to write hex-encoded binary compiled [Duckyscript](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Duckyscript).
Compile the text scripts using [duckencoder](https://github.com/hak5darren/USB-Rubber-Ducky/wiki/Downloads) from the USB-Rubber-Ducky
project, then write it to the Pill Duck flash using the 'd' command, for example:

```
duck> d00ff00ff00ff00eb0b0208000f000f00120036002c001a00120015000f000700
wrote flash
duck>
```

will type out "Hello, world". Type `p` to resume, if execution was previously paused.

Caution: May be buggy, any help welcome!
