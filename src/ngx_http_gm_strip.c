#include "ngx_http_gm_strip.h"

ngx_int_t 
gm_strip_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    dd("start");

    (void) StripImage(*image);

    return NGX_OK;
}

