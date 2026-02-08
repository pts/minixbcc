#! /bin/sh --
test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an invalid option in bash(1), and it's harmful (prevents echo) in ash(1).
set -ex
test "$0" = "${0%/*}" || cd "${0%/*}"

rm -f elks.img
#cp -a "$HOME"/Downloads/elks/elks070.img elks.img
#cp -a "$HOME"/Downloads/elks/elks081.img elks.img
cp -a elks0.img elks.img  # Some files are already deleted.
chmod +w elks.img
../../mxft.pl -i elks.img mkdir d
#../../mxft.pl -i elks.img mkdir d/sc
#../../mxft.pl -i elks.img write d/sc/lscs0.s <sc/lscs0.s
#../../mxft.pl -i elks.img write d/libt0.si <libt0.si
../../mxft.pl -i elks.img write d/llks.s <llks.s
../../mxft.pl -i elks.img write d/hw.c <hw.c
../../mxft.pl -i elks.img write d/re.lk <re.lk
../../mxft.pl -i elks.img write d/sc.lk <sc.lk
../../mxft.pl -i elks.img write d/as.lk <as.lk
../../mxft.pl -i elks.img write d/ld.lk <ld.lk
cat >lktest.sh <<'END'
#!/bin/sh
set -x
echo 'int main() { return write(1, "Hello, World!\n", 14) != 14; }' >hw.c
sc=./sc.lk
as=./as.lk
ld=./ld.lk
cat hw.c
# "$as" doesn't find the command.
$as -0 -w -o llks.o llks.s || exit "$?"
$sc -0 -o hw.s hw.c || exit "$?"
$as -0 -u -w -o hw.o hw.s || exit "$?"
$ld -0 -o hwns llks.o hw.o || exit "$?"
file hwns
$ld -0 -i -o hw llks.o hw.o || exit "$?"
./hw || exit "$?"
: OK.
END
../../mxft.pl -i elks.img write d/lktest.sh <lktest.sh

qemu-system-i386 -M pc-1.0 -m 4 -drive file=elks.img,format=raw,if=floppy -boot a -debugcon stdio -net none -enable-kvm


: "$0" OK.
