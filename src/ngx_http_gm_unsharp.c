#include "ngx_http_gm_unsharp.h"

ngx_int_t 
gm_unsharp_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    Image                             *unsharp_image = NULL;
    u_char                            *unsharp_geometry = NULL;
    /* Gaussian unsharpen */
    double                            amount = 1.0, radius = 0.0, sigma = 1.0, threshold = 0.05;

    dd("start");

    unsharp_geometry = gm_get_option_value(r, (ngx_http_gm_option_t *)option);
    if (unsharp_geometry == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: unsharp get geometry failed");
        return  NGX_ERROR;
    }

    if (ngx_strlen(unsharp_geometry) == 0) {
        return NGX_OK;
    }

    if (!IsGeometry((const char *)unsharp_geometry)) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: unsharp invalid geometry %u", unsharp_geometry);
        return  NGX_ERROR;
    }
    
    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: unsharp geometry: \"%s\"", unsharp_geometry);

    (void) GetMagickDimension((const char *)unsharp_geometry, &radius, &sigma, &amount, &threshold);

    unsharp_image=UnsharpMaskImage(*image,radius,sigma,amount,threshold, &(*image)->exception);
    if (unsharp_image == (Image *) NULL) {
        return NGX_ERROR;
    }

    DestroyImage(*image);
    *image = unsharp_image;

    return NGX_OK;
}

