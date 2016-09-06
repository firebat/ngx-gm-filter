#include "ngx_http_gm_filter.h"

ngx_int_t
gm_filter_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    FilterTypes                        filter;
    u_char                            *value;

    dd("start");

    value = gm_get_option_value(r, (ngx_http_gm_option_t *) option);
    if (value == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: filter option failed");
        return  NGX_ERROR;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: filter %s", value);

    filter = StringToFilterTypes((const char *) value);

    if (filter != UndefinedFilter) {
        (*image)->filter = filter;
    }

    return NGX_OK;
}
