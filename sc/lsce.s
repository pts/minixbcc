| Assembly syntax compatibility of this file:
|
| * asld (auto -0):  yes
| * v0 as -0:        ? (only partially possible, because .bss has a different meaning)
| * v0 as -0 -a:     ? (only partially possible, because .bss has a different meaning)
| * v0 as -3:        no
| * v0 as -3 -a:     no
| * v1 as -0:        ? (only partially possible, because .bss has a different meaning)
| * v1 as -0 -a:     ? (only partially possible, because .bss has a different meaning)
| * v1 as -3:        no
| * v1 as -3 -a:     no
|

.globl endtext, enddata, endbss, _end, _edata
.text
endtext:
.data
enddata:
_edata:
.bss
endbss:
_end:
