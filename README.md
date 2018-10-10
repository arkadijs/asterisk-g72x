### G.729 and G.723.1 codecs for Asterisk open source PBX

[Primary website] / [Google group]

[Asterisk] 1.4, 1.6, 1.8, 10.0 - 16.0 are supported.

To compile the codecs it is recommended to install [Intel IPP] libraries for better performance. Alternatively, download and install [Bcg729] - a slightly slower implementation written in portable C99. Only G.729 will be available in that case.

The codecs are tested against Bcg729 1.0.2, IPP 5.3 - 8.2. Users of IPP 9.0 and IPP 2017 must also install [IPP Legacy] libraries. AMD processors works with IPP too.

#### IPP

To install legacy IPP libraries:

    tar xf ipp90legacy_lin_9.0.0.008.tar
    cd ipp90legacy_lin/
    unzip linux.zip 
    ... password: accept
    mv linux /opt/intel/ipp/legacy

Place `legacy/` in the root of IPP installation directory (under `ipp` symlink).

Additionally, static libraries from the Intel compiler are required:

    cd /opt/intel/ipp/legacy/lib
    wget http://asterisk.hosting.lv/bin/icc-static-libs.tar.bz2
    tar xjf icc-static-libs.tar.bz2

#### Codecs

Use `./autogen.sh` to generate [GNU Autoconf] files, then `./configure`. Check available options with `./configure --help`. Specify `--prefix` in case Asterisk is installed in non-standard location.

G.723.1 send rate is configured in Asterisk codecs.conf file:

    [g723]
    ; 6.3kbps stream, default
    sendrate=63
    ; 5.3kbps
    ;sendrate=53

This option is for outgoing voice stream only. It does not affect incoming stream that should be decoded automatically whatever the bit-rate is.

There are also two Asterisk CLI commands `g723 debug` and `g729 debug` to print statistics about received frames sizes. This can aid in debugging audio problems. Bump Asterisk verbosity level to 3 to see the numbers.

`astconv` is audio format conversion utility similar to Asterisk `file convert` command. Build it with supplied `build-astconv.sh` script against Asterisk 1.8 or later. astconv loads codec_*.so modules directly to perform the conversion. Use codec module that was compiled against same Asterisk version the astconv was built against.

The translation result could be used to: (a) confirm the codec is working properly; (b) prepare voice-mail prompts, for example:

    ./astconv ./codec_g729.so -e 160 file.slin file.g729
    ./astconv ./codec_g729.so -d 10  file.g729 file.slin
    ./astconv ./codec_g723.so -e 480 file.slin file.g723
    ./astconv ./codec_g723.so -d 24  file.g723 file.slin

`file.slin` is signed linear 16-bin 8kHz mono audio, you can play it with alsa-utils:

    aplay -f S16_LE file.slin

and convert between other formats with SOX:

    sox input.wav -e signed-integer -b 16 -c 1 -r 8k -t raw output.slin
    sox -t raw -e signed-integer -b 16 -c 1 -r 8k input.slin output.wav

Files:

- codec_g72x.c, astconv.c, build-astconv.sh - GPLv3;
- autoconf files initially contributed by Michael.Kromer at computergmbh dot de;
- g723_slin_ex.h, g729_slin_ex.h, slin_g72x_ex.h - sample speech data;
- ipp/ files are a copy from IPP samples, IPP license apply.

Before reporting problem with the codecs, please read the [website] - compiling the codecs is not a trivial task. Asking [Asterisk G.729 Google group] first is also good idea.


[Asterisk]: http://www.asterisk.org/
[Primary website]: http://asterisk.hosting.lv/
[website]: http://asterisk.hosting.lv/
[Intel IPP]: https://software.intel.com/en-us/intel-ipp
[IPP Legacy]: https://software.intel.com/en-us/articles/intel-ipp-legacy-libraries
[Bcg729]: http://www.linphone.org/eng/documentation/dev/bcg729.html
[GNU Autoconf]: https://www.gnu.org/software/autoconf/
[Asterisk G.729 Google group]: http://groups.google.com/group/asterisk-g729
[Google group]: http://groups.google.com/group/asterisk-g729
