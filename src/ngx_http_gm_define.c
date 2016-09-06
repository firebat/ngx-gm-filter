#include "ngx_http_gm_define.h"

ngx_int_t
gm_define_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    dd("start");

    u_char *define = gm_get_option_value(r, (ngx_http_gm_option_t *) option);
    AddDefinitions(image_info,  (const char *)define, NULL);

    return NGX_OK;
}

