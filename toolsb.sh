#!/bin/sh
#
# toolsb.sh: helper script to build the tools for Minix i86 and i386
# by pts@fazekas.hu at Thu Jan  8 15:51:45 CET 2026
#
# Please don't invoke this script directly, but let build.sh do it for you.
#
# Output files; sc.tool, as.tool, ld.tool, cr.tool.
#
# This code in this script has been moved out of build.sh to avoid failing
# with `argument list too long' and `out of string space' errors in Minix
# 1.5.10 i86 /bin/sh.
#

test "$ZSH_VERSION" && set -y 2>/dev/null  # SH_WORD_SPLIT for zsh(1). It's an i nvalid option in bash(1), and it's harmful (prevents echo) in ash(1).

set -x  # Echo all commands run.
a03="$1"; sc="$2"; as="$3"; ld="$4"; cflags="$5"; cmp="$6"; diff="$7"; sedi="$8"
if test "$cmp" = tcmp; then
  # Indirect eval to avoid `./ld.host: argument list too long' below.
  a='tcmp( ) { cmp  "$1" "$2" >>tools.diff || echo "cp -p \"$1\" \"$2\""; }'; eval "$a"  # eval to make Minix /bin/sh skip it.
  a='tdiff() { diff "$1" "$2" >>tools.diff || echo "cp -p \"$1\" \"$2\""; }'; eval "$a"  # eval to make Minix /bin/sh skip it.
fi
if test "$a03" = 0; then aa=-a; cflags="-DSMALLMEM -DSC_ALIGNMENT=1 -DLD_ALIGNMENT=1 -DAS_ALIGNMENT=1 $cflags"  # For Minix i86. Save memory by using a small alignment.
else                     aa=;   cflags="$cflags"
fi

if test "$sedi"; then  # as v0 abort()s on the include pseudo-op, so we manually process it with sed+cat.
  sed "s/^include.*//" <sc/lscs"$a03".s >sc/lscs"$a03$sedi".s || exit "$?"  
  cat libt"$a03".si >>sc/lscs"$a03$sedi".s || exit "$?"
fi
"$as" -"$a03" $aa -w -o "$a03"/lscs.o sc/lscs"$a03$sedi".s || exit "$?"
"$cmp" "$a03"/lscs.o "$a03"g/lscs.n || exit "$?"
for b in bcc-cc1 assign codefrag debug declare express exptree floatop function gencode genloads glogcode hardop input label loadexp longop output preproc preserve scan softop state table type; do
  "$sc" -"$a03" $cflags -DOPEN00 -DSBRK -DMXMALLOC -DLIBCH -o "$a03"/sc"$b".s sc/"$b".c || exit "$?"
  "$diff" "$a03"/sc"$b".s "$a03"g/sc"$b".r || exit "$?"
  "$as" -"$a03" -u -w -o "$a03"/sc"$b".o "$a03"/sc"$b".s || exit "$?"
  "$cmp"  "$a03"/sc"$b".o "$a03"g/sc"$b".n || exit "$?"
done
rm -f sc.tool
if test "$a03" = 3; then cv=150000; else cv=; fi  # chmem =150000 sc || exit "$?"  # C compiler backend.
if test "$sedi"; then hflag=; elif test "$cv"; then hflag="-h $cv"; cv=; else hflag=; fi
"$ld" -"$a03" -i $hflag -o sc.tool "$a03"/lscs.o "$a03"/scbcc-cc1.o "$a03"/scassign.o "$a03"/sccodefrag.o "$a03"/scdebug.o "$a03"/scdeclare.o "$a03"/scexpress.o "$a03"/scexptree.o "$a03"/scfloatop.o "$a03"/scfunction.o "$a03"/scgencode.o "$a03"/scgenloads.o "$a03"/scglogcode.o "$a03"/schardop.o "$a03"/scinput.o "$a03"/sclabel.o "$a03"/scloadexp.o "$a03"/sclongop.o "$a03"/scoutput.o "$a03"/scpreproc.o "$a03"/scpreserve.o "$a03"/scscan.o "$a03"/scsoftop.o "$a03"/scstate.o "$a03"/sctable.o "$a03"/sctype.o || exit "$?"
test -z "$cv" || chmem "=$cv" sc.tool || exit "$?"
ls -l sc.tool >&2 || exit "$?"
test -x sc.tool || exit "$?"
"$cmp" sc.tool "$a03"g/sc.d || exit "$?"

if test "$sedi"; then  # as v0 abort()s on the include pseudo-op, so we manually process it with sed+cat.
  sed "s/^include.*//" <as/lass"$a03".s >as/lass"$a03$sedi".s || exit "$?"  
  cat libt"$a03".si >>as/lass"$a03$sedi".s || exit "$?"
fi
"$as" -"$a03" $aa -w -o "$a03"/lass.o as/lass"$a03$sedi".s || exit "$?"
"$cmp" "$a03"/lass.o "$a03"g/lass.n || exit "$?"
for b in as assemble error express genbin genlist genobj gensym heap keywords macro mops pops readsrc scan table typeconv; do
  "$sc" -"$a03" $cflags -DMINIXHEAP -DBRKSIZE -DOPEN00 -DLIBCH -o "$a03"/as"$b".s as/"$b".c || exit "$?"
  "$diff" "$a03"/as"$b".s "$a03"g/as"$b".r || exit "$?"
  "$as" -"$a03" -u -w -o "$a03"/as"$b".o "$a03"/as"$b".s || exit "$?"
  "$cmp"  "$a03"/as"$b".o "$a03"g/as"$b".n || exit "$?"
done
rm -f as.tool
if test "$a03" = 3; then cv=192480; else cv=; fi  # chmem =192480 as || exit "$?"  # Assembler. v0 /local/bin/as has only =150000.
if test "$sedi"; then hflag=; elif test "$cv"; then hflag="-h $cv"; cv=; else hflag=; fi
"$ld" -"$a03" -i $hflag -o as.tool "$a03"/lass.o "$a03"/asas.o "$a03"/asassemble.o "$a03"/aserror.o "$a03"/asexpress.o "$a03"/asgenbin.o "$a03"/asgenlist.o "$a03"/asgenobj.o "$a03"/asgensym.o "$a03"/asheap.o "$a03"/askeywords.o "$a03"/asmacro.o "$a03"/asmops.o "$a03"/aspops.o "$a03"/asreadsrc.o "$a03"/asscan.o "$a03"/astable.o "$a03"/astypeconv.o || exit "$?"
test -z "$cv" || chmem "=$cv" as.tool || exit "$?"
ls -l as.tool >&2 || exit "$?"
"$cmp" as.tool "$a03"g/as.d || exit "$?"

if test "$sedi"; then  # as v0 abort()s on the include pseudo-op, so we manually process it with sed+cat.
  sed "s/^include.*//" <ld/llds"$a03".s >ld/llds"$a03$sedi".s || exit "$?"  
  cat libt"$a03".si >>ld/llds"$a03$sedi".s || exit "$?"
fi
"$as" -"$a03" $aa -w -o "$a03"/llds.o ld/llds"$a03$sedi".s || exit "$?"
"$cmp" "$a03"/llds.o "$a03"g/llds.n || exit "$?"
for b in dumps heap io ld readobj table typeconv writebin; do
  "$sc" -"$a03" $cflags -DOPEN00 -DMINIXHEAP -DBRKSIZE -DLIBCH -DLIBCHMINIX -o "$a03"/ld"$b".s ld/"$b".c || exit "$?"
  "$diff" "$a03"/ld"$b".s "$a03"g/ld"$b".r || exit "$?"
  "$as" -"$a03" -u -w -o "$a03"/ld"$b".o "$a03"/ld"$b".s || exit "$?"
  "$cmp"  "$a03"/ld"$b".o "$a03"g/ld"$b".n || exit "$?"
done
rm -f ld.tool
if test "$a03" = 3; then cv=150000; else cv=; fi  # chmem =150000 ld || exit "$?"  # Linker.
if test "$sedi"; then hflag=; elif test "$cv"; then hflag="-h $cv"; cv=; else hflag=; fi
"$ld" -"$a03" -i $hflag -o ld.tool "$a03"/llds.o "$a03"/lddumps.o "$a03"/ldheap.o "$a03"/ldio.o "$a03"/ldld.o "$a03"/ldreadobj.o "$a03"/ldtable.o "$a03"/ldtypeconv.o "$a03"/ldwritebin.o || exit "$?"
test -z "$cv" || chmem "=$cv" ld.tool || exit "$?"
ls -l ld.tool >&2 || exit "$?"
"$cmp" ld.tool "$a03"g/ld.d || exit "$?"

if test "$sedi"; then  # as v0 abort()s on the include pseudo-op, so we manually process it with sed+cat.
  sed "s/^include.*//" <cr/lcrs"$a03".s >cr/lcrs"$a03$sedi".s || exit "$?"  
  cat libt"$a03".si >>cr/lcrs"$a03$sedi".s || exit "$?"
fi
"$as" -"$a03" $aa -w -o "$a03"/lcrs.o cr/lcrs"$a03$sedi".s || exit "$?"
"$cmp" "$a03"/lcrs.o "$a03"g/lcrs.n || exit "$?"
for b in cr; do
  "$sc" -"$a03" $cflags -DOPEN00 -DLIBCH -o "$a03"/cr"$b".s cr/"$b".c || exit "$?"
  "$diff" "$a03"/cr"$b".s "$a03"g/cr"$b".r || exit "$?"
  "$as" -"$a03" -u -w -o "$a03"/cr"$b".o "$a03"/cr"$b".s || exit "$?"
  "$cmp"  "$a03"/cr"$b".o "$a03"g/cr"$b".n || exit "$?"
done
rm -f cr.tool
if test "$a03" = 3; then cv=40000; else cv=; fi  # chmem =40000 cr || exit "$?"  # Library builder. The memory includes the argv (very long list of member filenames).
if test "$sedi"; then hflag=; elif test "$cv"; then hflag="-h $cv"; cv=; else hflag=; fi
"$ld" -"$a03" -i $hflag -o cr.tool "$a03"/lcrs.o "$a03"/crcr.o || exit "$?"
test -z "$cv" || chmem "=$cv" cr.tool || exit "$?"
ls -l cr.tool >&2 || exit "$?"
"$cmp" cr.tool "$a03"g/cr.d || exit "$?"
