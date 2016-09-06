#include "ngx_http_gm_rotate.h"

ngx_int_t 
gm_rotate_image(ngx_http_request_t *r, void *option, Image **image, ImageInfo *image_info)
{
    Image                             *rotate_image = NULL;
    u_char                            *rotate_degree = NULL;
    double                            degree;

    dd("start");

    rotate_degree = gm_get_option_value(r, (ngx_http_gm_option_t *)option);
    if (rotate_degree == NULL) {
        ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "gm filter: rotate, get degree failed");
        return  NGX_ERROR;
    }

    if (ngx_strlen(rotate_degree) == 0) {
        return NGX_OK;
    }

    if (ngx_strchr(rotate_degree, '>') != (char *) NULL) {
        if ((*image)->columns <= (*image)->rows) {
            return NGX_OK;
        }
    } else if (ngx_strchr(rotate_degree, '<') != (char *) NULL) {
        if ((*image)->columns >= (*image)->rows) {
            return NGX_OK;
        }
    }

    degree = strtod((char *) rotate_degree, (char **)NULL);
    if (degree == 0) {
        return NGX_OK;
    }

    ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0, "gm filter: rotate image degrees \"%f\"", degree);

    rotate_image = RotateImage(*image, degree, &(*image)->exception);
    if (rotate_image == (Image *) NULL) {
        return NGX_ERROR;
    }

    DestroyImage(*image);
    *image = rotate_image;

    return NGX_OK;
}
