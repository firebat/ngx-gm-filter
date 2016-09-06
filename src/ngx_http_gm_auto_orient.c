#include "ngx_http_gm_auto_orient.h"

ngx_int_t
gm_auto_orient_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    Image                             *orient_image = NULL;

    dd("starting");

    orient_image = AutoOrientImage(*image, (*image)->orientation, &(*image)->exception);
    if (orient_image == (Image *) NULL) {
        return NGX_ERROR;
    }

    DestroyImage(*image);
    *image = orient_image;

    return NGX_OK;
}
