# XLCPixy

XLCPixy is arduino library for pixy that extends original
[charmedlabs's one](https://github.com/charmedlabs/pixy).
It was created because of troubles when getBlocks() returns less blocks
(usually one) than pixy detects.

The first question was how pixy communicates with arduino. Firmware code was
too difficult to understand so we looked "at wires". In this example there are
three objects in front of camera (two of signature 1, one 2). Each of
the following values is two-byte word (0xaa55 is in hex format) read from pixy
one by one:

```
0
0
0
0
0
0
0
0
0xaa55    <- start of new frame
0xaa55    <- start of first block
178
1
99
26
28
24
0xaa55    <- next block
290
1
212
26
29
22
0xaa55    <- next block
235
2
161
23
28
21
0
0
0
0
0
...
```

And a lot of zeros up to next frame. But suddenly something strange was
observed:

```
0xaa55    <- next frame
0xaa55
176
1
99
25
28
23
0xaa55    <- next frame
0xaa55
175
1
99
25
28
22
0xaa55
290
1
213
26
28
22
0xaa55
237
2
161
23
28
23
0
0
0
0
0
...
```

It seems like the first frame was overwriten by next one. Why? Actually, we
don't know yet, but what's more important is, that original library stops
communication at this point, remembers it (to continue when getBlocks() is
called next time) and returns only one block. May be it's caused by too
frequent readings.

## How deals with it this library?

* Whole trick is to read all blocks (up to zeros at the end) and in case of
double 0xaa55 (new frame) to forget all already read and to start again.
Problem may occur when sending all blocks in one frame takes so long, that
no zeros follows, just next frame. Although, this should be no problem with
reasonable amount of detected blocks and SPI communication.
* Moreover when pixy sends zeros long enough (20 ms should suffice), we can say
with sure there are no blocks detected.
* Finally, dynamically allocated array was changed to constant-sized, as we
don't like using malloc on arduino boards.

## How to use it

You should include original library for communication you use (besides
including XLCPixy, of course), i.e. Pixy, PixyI2C, PixySPI_SS, or PixyUART.
Define XLCPixy instance like `XLCPixy<link_type> pixy;`, but instead of
`link_type` write class used for communication (`LinkSPI`, `LinkI2C`,
`LinkSPI_SS`, `LinkUART`). The interface is quite the same as you were used to.
