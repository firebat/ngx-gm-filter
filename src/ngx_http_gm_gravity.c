#include "ngx_http_gm_gravity.h"

ngx_int_t
gm_gravity_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    GravityType                     gravity;
    u_char                         *value;

    dd("start");

    value = gm_get_option_value(r, (ngx_http_gm_option_t *)option);
    if (value == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: gravity option failed");
        return  NGX_ERROR;
    }

    if (ngx_strlen(value) == 0) {
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: gravity %s", value);

    gravity = StringToGravityType((char *)value);
    if (gravity != ForgetGravity) {
        (*image)->gravity = gravity;
    }

    return NGX_OK;
}
