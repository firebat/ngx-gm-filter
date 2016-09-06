#include "ngx_http_gm_quality.h"

ngx_int_t gm_quality_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    u_char                     *quality;
    int                         value;

    dd("start");

    quality = gm_get_option_value(r, (ngx_http_gm_option_t *)option);
    if (quality == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: get quality failed");
        return NGX_ERROR;
    }

    if (ngx_strlen(quality) == 0){
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: quality \"%s\"", quality);

    value = atoi((char *) quality);
    if (value <= 0 || value > 100) {
        return NGX_ERROR;
    }

    image_info->quality = value;

    return NGX_OK;
}
