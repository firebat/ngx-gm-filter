#ifndef NGX_HTTP_GM_COMPOSITE_H
#define NGX_HTTP_GM_COMPOSITE_H

#include "ngx_http_gm_common.h"

ngx_int_t gm_parse_composite_option(ngx_conf_t *cf, ngx_array_t *args, void **option);
ngx_int_t gm_composite_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info);

#endif
