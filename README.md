# ns

A command line numerical sequencer for digital photos. Sorts files by the EXIF Date and Time Captured attribute, then renames them with a base string, a seperator, and a sequence number.

## Prerequisites
You need to have the following prerequisite tools and libraries installed on your machine.

 * C compiler
 * libexif
 * argp-standalone (Mac OS X only)

### Mac OS X
If you havn't already, install [Homebrew](https://github.com/Homebrew/homebrew/wiki/Installation), then you can just `brew install argp-standalone exif` to get the required packages. If you don't want to, I may provide a precompiled binary in the future, otherwise you're on your own.
### Linux
I'm assuming you're a nerd who knows what to do. However, you'll need to install `libexif-dev` prior to building.
## Building
If you download and build it and get errors, *please* let me know. File an issue.

To build it, for now, you just need to run the following commands:

    ./configure
    make
    sudo make install

##Contributing
Please fork the repo on gitbub and send a pull-request. You are awesome.
