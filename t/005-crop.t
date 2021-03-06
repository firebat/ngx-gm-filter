# vim:set ft= ts=4 sw=4 et fdm=marker:

use FindBin;
use Test::Nginx::Socket;

use lib "$FindBin::Bin/lib";


repeat_each(1);

plan tests => repeat_each() * (blocks() * 3);

run_tests();


__DATA__

=== TEST 1: no crop
--- config eval
<<EOF;
    location /crop {
         alias $FindBin::Bin/data;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 597491



=== TEST 2: crop 200x100
--- config eval
<<EOF;
    location /crop {
         alias $FindBin::Bin/data;
         gm crop 200x100;
         gm quality 85;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 3646



=== TEST 3: crop 200x100+50+30
--- config eval
<<EOF;
    location /crop {
         alias $FindBin::Bin/data;
         gm crop 200x100+50+30;
         gm quality 85;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 3058



=== TEST 4: crop with variable
--- config eval
<<EOF;
    location /crop {
         set \$crop "100x100+20+30";
         alias $FindBin::Bin/data;
         gm crop \$crop;
         gm quality 85;
         gm_buffer 16M;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 2511



=== TEST 5: crop relative to Gravity 
--- config eval
<<EOF;
    location /crop {
         set \$crop "100x100";
         alias $FindBin::Bin/data;
         gm crop \$crop -gravity Center;
         gm quality 85;
         gm_buffer 16M;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 2454



=== TEST 6: crop relative to Gravity variable
--- config eval
<<EOF;
    location /crop {
         set \$crop "100x100";
         set \$g "center";
         alias $FindBin::Bin/data;
         gm crop \$crop -gravity \$g;
         gm quality 85;
         gm_buffer 16M;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 2454



=== TEST 7: crop relative to Gravity variable empty
--- config eval
<<EOF;
    location /crop {
         set \$crop "100x100";
         set \$g "";
         alias $FindBin::Bin/data;
         gm crop \$crop -gravity \$g;
         gm quality 85;
         gm_buffer 16M;
    }
EOF
--- request
GET /crop/1024_768.jpg
--- response_headers
Content-Type: image/jpeg
Content-Length: 2454

