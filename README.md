# QEMU Extras
Old PC Games Addons for QEMU
## Content
    openglide - OpenGLide fork optimized for QEMU (Support Glide2x & Glide3x)
    g2xwrap   - GLIDE.DLL & GLIDE3X.DLL that wrap into Glide2x APIs
    dosbox    - DOSBox SVN Games essentials

## Building OpenGLide
    $ mkdir ~/myxtra && cd ~/myxtra
    $ git clone https://github.com/kjliew/qemu-xtra.git
    $ cd qemu-xtra/openglide
    $ bash ./bootstrap
    $ mkdir ../build && cd ../build
    $ ../openglide/configure --disable-sdl && make
    
