## Required packages

Please install these packages before moving to next step. Below is a reference list for Debian-based distributions. Reference build system is Ubuntu 18.04/20.04 amd64.

    subversion git-core gcc g++ make gawk bison flex bzip2 netpbm autoconf automake libx11-dev libxext-dev libc6-dev liblzo2-dev libxxf86vm-dev libpng-dev gcc-multilib libsdl1.2-dev byacc python-mako libxcursor-dev cmake

## Clone & build

    $ mkdir myrepo
    $ cd myrepo
    $ git clone https://github.com/deadw00d/AROS.git AROS
    $ cd AROS
    $ git checkout alt-bincompat
    $ cd ..
    $ cp ./AROS/scripts/rebuild.sh .
    $ ./rebuild.sh

Proceed to build selection below

### Amiga-m68k

1. Select toolchain-alt-bincompat-m68k
2. Select alt-bincompat-amiga-m68k

Kickstart images available in

    alt-bincompat-amiga-m68k/bin/amiga-m68k/AROS/boot/amiga
