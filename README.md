ngx-gm-filter
=============

ngx-gm-filter - Another image filter based GraphicsMagick.
[![Analytics](https://ga-beacon.appspot.com/UA-5174134-2/ngx-gm-filter/readme)](https://github.com/igrigorik/ga-beacon)

Status
======

This module is under active development and is production ready.


Version
=======

This document describes ngx-gm-filter [v0.2.0](https://github.com/liseen/ngx-gm-filter/tags).


Synopsis
========

```
server {
    gm_buffer 10M;

    location /gm {
         alias imgs;

         set $resize 400x300>;
         set $rotate 180;

         gm strip;
         gm auto-orient;

         gm resize $resize;
         gm rotate $rotate;
         gm quality 85;

         gm gravity SouthEast;
         gm composite -geometry +10+10 -image '/your/watermark/file' -min-width 100;
    }
}
```

Description
===========

Directives
==========

gm_buffer
--------------
**syntax:** *gm_buffer size*

**default:** *gm_buffer 4M*

**context:** *server, location*

gm
--------------
**syntax:** *gm [blur|composite|crop|gravity|quality|resize|rotate|strip|unsharp] options*

**default:** *none*

**context:** *location*

Installation
============

Install Webp
------------

    wget https://github.com/webmproject/libwebp/archive/v0.5.0.zip
    ./autogen.sh
    ./configure
    make -j4
    sudo make install

    sudo cp /usr/local/lib/libwebp.so.6.0.0 /usr/lib64/
    sudo ln -s /usr/lib64/libwebp.so.6.0.0 /usr/lib64/libwebp.so.6
    sudo ln -s /usr/lib64/libwebp.so.6.0.0 /usr/lib64/libwebp.so

    sudo ldconfig
    ldconfig -p |grep webp

Install GraphicsMagick
------------

Install GraphicsMagick with jpeg and png

    sudo yum install -y gcc gcc-c++ make cmake autoconf automake
    sudo yum install -y libpng-devel libjpeg-devel libtiff-devel jasper-devel freetype-devel giflib-devel
    wget http://78.108.103.11/MIRROR/ftp/GraphicsMagick/1.3/GraphicsMagick-1.3.23.tar.gz
    tar xzvf GraphicsMagick-1.3.23.tar.gz
    cd GraphicsMagick-1.3.23
    ./configure --disable-openmp --enable-shared --with-webp
    make -j4
    sudo make install

    gm -version

Install ngx-gm-filter
------------

Build the source with this module:

    wget 'http://nginx.org/download/nginx-1.2.1.tar.gz'
    tar -xzvf nginx-1.2.1.tar.gz
    cd nginx-1.2.1/

    ./configure --prefix=/opt/nginx \
				--add-module=path/to/ngx-gm-filter

    make -j2
    sudo make install


Development
===========

Build & Run
-----------

    $ cd ngx-gm-filter
    $ util/build.sh
    emacs work/conf/nginx.conf

    ...
    daemon off;

    server {
        listen 8080;
    ...

    $ export PATH=work/sbin:$PATH
    $ nginx

Test
----

    sudo cpan Test::Nginx
    perl t/000-empty.t   # Test one
    prove                # Test all


Copyright and License
=====================

This module is licensed under the BSD license.

Copyright (C) 2009-2012, by "liseen" Xunxin Wan(万珣新) <liseen.wan@gmail.com>.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


See Also
========

* [GraphicsMagick](http://www.graphicsmagick.org/)  GraphicsMagick library.
* [HttpImageFilterModule](http://wiki.nginx.org/HttpImageFilterModule) .
