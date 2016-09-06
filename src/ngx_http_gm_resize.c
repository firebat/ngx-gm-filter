#include "ngx_http_gm_resize.h"

ngx_int_t
gm_resize_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    RectangleInfo                      geometry;
    Image                             *resize_image = NULL;
    u_char                            *resize_geometry = NULL;

    dd("start");

    resize_geometry = gm_get_option_value(r, (ngx_http_gm_option_t *)option);
    if (resize_geometry == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: resize get geometry failed");
        return  NGX_ERROR;
    }

    if (ngx_strlen(resize_geometry) == 0) {
        return NGX_OK;
    }

    if (!IsGeometry((const char *)resize_geometry)) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: resize invalid geometry %u", resize_geometry);
        return  NGX_ERROR;
    }

    (void) GetImageGeometry(*image, (char *)resize_geometry, 1, &geometry);
    if ((geometry.width == (*image)->columns) && (geometry.height == (*image)->rows)) {
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: resize geometry: \"%s\"", resize_geometry);

    resize_image = ResizeImage(*image, geometry.width, geometry.height, (*image)->filter,(*image)->blur, &(*image)->exception);
    if (resize_image == (Image *) NULL) {
        return NGX_ERROR;
    }

    DestroyImage(*image);
    *image = resize_image;

    return NGX_OK;
}
