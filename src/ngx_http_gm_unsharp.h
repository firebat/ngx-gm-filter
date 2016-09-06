#ifndef NGX_HTTP_GM_UNSHARP_H
#define NGX_HTTP_GM_UNSHARP_H

#include "ngx_http_gm_common.h"

ngx_int_t gm_unsharp_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info);

#endif
