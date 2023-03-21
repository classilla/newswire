# The Newsroom Wire Service Has a Posse

[Another Old VCR Project!](https://oldvcr.blogspot.com/2023/03/printing-real-headline-news-on.html)

If you've ever wondered what the Wire Service in Springboard Software's The Newsroom newsletter and desktop publishing package actually does, now you can send articles and photos through it with this reverse-engineered implementation.

This is a toy, not a serious tool for publication and not an all-consuming personal obsession. Use it in the spirit intended.

Copyright (C) 2023 Cameron Kaiser.  
All rights reserved.  
3-clause BSD license, except media files (see `example/orig` for details).

## Example output
![Example six-panel output from VICE, converted with `vicetopnm`.](https://github.com/classilla/newswire/blob/main/slayout.png?raw=true)

Pretty, huh? [Read more about the process.](https://oldvcr.blogspot.com/2023/03/printing-real-headline-news-on.html)

## Supports
* Creates Newsroom photos from 231x148 Netpbm portable bitmaps (`genpho`)
* Creates captioned Newsroom photo panels (38 characters max) (`gencap`)
* Turns text files into multiple spanned Newsroom panels with word wrap (see `example/orig` for format) (`genart`)
* Can push these files over a serial connection to a running instance of The Newsroom Wire Service (`xmit.c`)
* Also has tools to serve as a dummy Wire Service receiver (`recv.c`) and to act as an MITM between two serial ports and snoop serial data (`mitm300.c`)
* Also can turn VICE text printer output into Netpbm portable bitmaps for printing (convert to PDF with your tool of choice) (`vicetopnm` and eventually `mps803topnm`)

`example/` contains a sample set of Newsroom assets created with these tools from the original images and articles in `example/orig/`.

## Known gaps
* Only tested with the Commodore 64 version (though the C-based serial tools, `xmit.c`, `mitm300.c` and `recv.c`, should be able to talk to Apple and IBM PC versions of The Newsroom; the Atari release does not have a Wire Service)
* Filenames need to be all lowercase and not counting the `ph.` and `pn.` prefix should be eight characters or less, or there will be blood
* Not compatible with The Newsroom _Pro_'s Wire Service
* Serial tools have hardcoded 300bps speed and paths to USB serial devices; modify as you see fit
* The `gen*` tools only create Commodore 64-compatible files
* Supports only text-only panels and fixed-size (231x148) photos and photo panels, does not yet transfer banners or layouts or use more complex multi-image panels

## How to use
The Perl scripts (anything not ending in `*.c`) take data on standard input and/or as command line arguments, and emit the result on standard output. You should consistently name photo files starting with the prefix `ph.` and panel files with the prefix `pn.` or some of these tools will get confused. They are self-contained and require no additional libraries or programs other than Perl (tested with 5.8+) itself, though you may need Netpbm or ImageMagick to convert the image to something more practical.

The C files are compiled simply with something like `gcc -o program program.c` and have no prerequisites other than a POSIX environment and `termios.h`. They are intended for use with an RS-232 port, such as a USB-serial dongle. Unless otherwise noted, they are hardcoded to use `/dev/ttyUSB0` (and, for `mitm300`, `/dev/ttyUSB1`) and run at 300bps. Modify the program if necessary for your specific setup.

* `recv` will simply repeatedly `ACK` packets to a sending instance of The Newsroom Wire Service so you can see a transmission. It does not verify the packet other than checking it got a full 134 bytes. It does not periodically retransmit the `ACK`, so multi-file transfers may require you to restart `recv` with the connection still live (acknowledged issue). If you pass the path to a port, that will be used instead of `/dev/ttyUSB0`.
* `mitm300` will transmit data between two USB serial ports as if they were directly connected, displaying to the screen who sent what. This can also be used as a simple serial sniffer/serial snooper program.
* `xmit` will take a list of filenames, combine them into a unified transfer, and send them block-by-block to a receiving instance of The Newsroom Wire Service. `xmit` can only currently preprocess photos, single photo panels, and text-only panels, and only in the formats the Perl scripts generate.

## Don't open issues on this repo
Or at least, if you do, don't open issues that are already documented, pertain specifically to The Newsroom or your port of The Newsroom, pertain to your emulator, pertain to your printer, or are stupid, pointless, and/or reflect your failure to read this paragraph.

This is a toy project, so pull requests should be worth my time. If you want to open a pull request, please try to match existing style and don't refactor or rewrite without a damn good reason. I still might tell you to fork the repo anyway. Nothing personal.

## License
Copyright (C) 2023 Cameron Kaiser. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. 

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE, SO THERE.
