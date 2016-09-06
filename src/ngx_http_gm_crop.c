#include "ngx_http_gm_crop.h"

ngx_int_t 
gm_crop_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    RectangleInfo                      geometry;
    Image                             *crop_image = NULL;
    u_char                            *crop_geometry = NULL;

    dd("start");

    crop_geometry = gm_get_option_value(r, (ngx_http_gm_option_t *)option);
    if (crop_geometry == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: crop get geometry failed");
        return  NGX_ERROR;
    }

    if (ngx_strlen(crop_geometry) == 0) {
        return NGX_OK;
    }

    if (!IsGeometry((const char *)crop_geometry)) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: crop invalid geometry %u", crop_geometry);
        return  NGX_ERROR;
    }

    (void) GetImageGeometry(*image, (char *)crop_geometry, 0, &geometry);

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: crop geometry: \"%s\"", crop_geometry);

    crop_image = CropImage(*image, &geometry, &(*image)->exception);
    if (crop_image == (Image *) NULL) {
        return NGX_ERROR;
    }

    DestroyImage(*image);
    *image = crop_image;

    return NGX_OK;
}
