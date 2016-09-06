#include "ngx_http_gm_format.h"

// WebP is bitstream-compatible with VP8 and uses 14 bits for width and height.
// The maximum pixel dimensions of a WebP image is 16383 x 16383
#define GM_IMAGE_WEBP_MAX 16383

ngx_int_t
gm_format_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    u_char                      *format = NULL;

    dd("starting format");

    format = gm_get_option_value(r, (ngx_http_gm_option_t *) option);
    if (format == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm format: get format failed");
        return NGX_ERROR;
    }

    if (ngx_strncmp(format, "webp", 4) == 0) {
        if ((*image)->columns > GM_IMAGE_WEBP_MAX || (*image)->rows > GM_IMAGE_WEBP_MAX) {
            return NGX_OK;
        }

        strcpy((*image)->magick, "WEBP");
    }

    return NGX_OK;
}
