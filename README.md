# interpolation

Feel free to contact me by email.

## Description

This is a simple software to "frame-interpolate" videos using only one algorithm, for now:

### Blending

Fast and reliable, but not very efficient.
[Example](http://youtu.be/ySSamjEG6DE)

## What do I need ?

You need an usual development environnement (a C compiler, GNU Make, etc). The Makefile use `c99` as an alias for the compiler. You also need FFmpeg as a command-line tool.

This program need only one library:
- OpenCV

## How to test ?

There is a Makefile so you can also compile it with only one command:

```
make all
```

*Usage: interpolation _source_ _destination_*

## License
This program is protected by the **GNU GENERAL PUBLIC LICENSE**. This repositery should contain a file named _LICENSE_.
