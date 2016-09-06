#include "ngx_http_gm_blur.h"

ngx_int_t
gm_blur_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    double                           radius = 0.0, sigma = 1.0;
    Image                           *blur_image = NULL;
    u_char                          *blur_geometry = NULL;

    dd("start");

    blur_geometry = gm_get_option_value(r, (ngx_http_gm_option_t *) option);
    if (blur_geometry == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: blur get geometry failed");
        return NGX_ERROR;
    }

    if (ngx_strlen(blur_geometry) == 0) {
        return NGX_OK;
    }

    if (!IsGeometry((const char*) blur_geometry)) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: blur invalid geometry: \"%s\"", blur_geometry);
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: blur geometry: \"%s\"", blur_geometry);

    (void) GetMagickDimension((const char*) blur_geometry, &radius,&sigma,NULL,NULL);
    blur_image=BlurImage(*image, radius, sigma, &(*image)->exception);
    if (blur_image == (Image *) NULL) {
        return NGX_ERROR;
    }

    DestroyImage(*image);
    *image = blur_image;

    return NGX_OK;
}
