# qemu-xtra

Content
-------
    openglide - OpenGLide fork optimized for QEMU (Support Glide2x & Glide3x)
    g2xwrap   - GLIDE.DLL & GLIDE3X.DLL that wrap into Glide2x APIs

Building OpenGLide
------------------
    $ mkdir ~/myxtra && cd ~/myxtra
    $ git clone https://github.com/kjliew/qemu-xtra.git
    $ cd qemu-xtra/openglide
    $ ./bootstrap
    $ mkdir ../build && cd ../build
    $ ../openglide/configure --disable-sdl && make
    
