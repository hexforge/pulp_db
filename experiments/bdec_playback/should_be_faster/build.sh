gcc -O3 -c *.c

gcc -O3 buffer.o digits.o fivedigits.o fopen_ascii.o ninedigits.o playback_packet.o sixdigits.o tendigits.o twodigits.o digit.o eightdigits.o fourdigits.o metaheader.o playback.o sevendigits.o slashstring.o threedigits.o variable_integer.o   main.o -o normal.exe
gcc -O3 buffer.o digits.o fivedigits.o fopen_ascii.o ninedigits.o playback_packet.o sixdigits.o tendigits.o twodigits.o digit.o eightdigits.o fourdigits.o metaheader.o playback.o sevendigits.o slashstring.o threedigits.o variable_integer.o fo_main.o -o fo.exe
