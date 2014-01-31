### G.729 and G.723.1 codecs for Asterisk open source PBX

[Primary website] / [Google group]

[Asterisk] 1.4, 1.6, 1.8, 10, 11, and 12 are supported. For Asterisk 1.2 and Callweaver use the pre-built binaries from the [website].

To compile the codecs it is recommended to install [Intel IPP] libraries for better performance. Alternatively, you need to download and install [Bcg729] - a slightly slower implementation written in portable C99. Only G.729 will be available in that case.

The codecs are tested against Bcg729 1.0.0, IPP 5.3, 6.0, 6.1, 7.0, 7.1, 8.0. Use IPP 5.3 for Pentium3, and 6.0+ for Atom CPU. AMD processors works with IPP without problems.

Use `./autogen.sh` to generate [GNU Autoconf] files, then `./configure`. Check available options with `./configure --help`. Specify `--prefix` in case Asterisk is installed in non-standard location.

G.723.1 send rate is configured in Asterisk codecs.conf file:

    [g723]
    ; 6.3kbps stream, default
    sendrate=63
    ; 5.3kbps
    ;sendrate=53

This option is for outgoing voice stream only. It does not affect incoming stream that should be decoded automatically whatever the bit-rate is.

There are also two Asterisk CLI commands `g723 debug` and `g729 debug` to print statistics about received frames sizes. This can aid in debugging audio problems. You need to bump Asterisk verbosity level to 3 to see the numbers.

`astconv` is audio format conversion utility similar to Asterisk `file convert` command. Build it with supplied `build-astconv.sh` script against Asterisk 1.8 or later. astconv uses codec_*.so modules directly to perform the conversion. You need codec module that was compiled against same Asterisk version the astconv was built against.

The translation result could be used to: (a) confirm the codec is working properly; (b) prepare voice-mail prompts, for example:

    ./astconv ./codec_g729.so -e 160 file.slin file.g729
    ./astconv ./codec_g729.so -d 10  file.g729 file.slin
    ./astconv ./codec_g723.so -e 480 file.slin file.g723
    ./astconv ./codec_g723.so -d 24  file.g723 file.slin

`file.slin` is signed linear 16-bin 8kHz mono audio, you can play it with

    aplay -f S16_LE file.slin

and convert to/from other formats with SOX:

    sox input.wav -e signed-integer -b 16 -c 1 -r 8k -t raw output.slin
    sox -t raw -e signed-integer -b 16 -c 1 -r 8k input.slin output.wav

Files:

- codec_g72x.c - GPLv3, code is based on code by Daniel Pocock at http://www.readytechnology.co.uk/open/ipp-codecs/ and various Asterisk bundled codecs;
- astconv.c, build-astconv.sh - GPLv3;
- autoconf files initially contributed by Michael.Kromer at computergmbh dot de;
- g723_slin_ex.h, g729_slin_ex.h, slin_g72x_ex.h - sample speech data;
- ipp/ files are copied from IPP samples, IPP license apply.

Before reporting problem with the codecs, please read the [website] and make sure you know what you're doing - compiling the codecs is not a novice task. Asking [Asterisk G.729 Google group] first is also good idea.

Author: Arkadi.Shishlov at gmail dot com

[Asterisk]: http://www.asterisk.org/
[Primary website]: http://asterisk.hosting.lv/
[website]: http://asterisk.hosting.lv/
[Intel IPP]: http://software.intel.com/en-us/non-commercial-software-development
[Bcg729]: http://www.linphone.org/eng/documentation/dev/bcg729.html
[GNU Autoconf]: https://www.gnu.org/software/autoconf/
[Asterisk G.729 Google group]: http://groups.google.com/group/asterisk-g729
[Google group]: http://groups.google.com/group/asterisk-g729
