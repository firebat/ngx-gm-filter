#include "ngx_http_gm_common.h"

static ngx_int_t
gm_parse_option_value(ngx_conf_t *cf, ngx_str_t *value, ngx_http_gm_option_t *option)
{
    ngx_http_complex_value_t           cv;
    ngx_http_compile_complex_value_t   ccv;

    ngx_memzero(&ccv, sizeof(ngx_http_compile_complex_value_t));

    ccv.cf = cf;
    ccv.value = value;
    ccv.complex_value = &cv;

    if (ngx_http_compile_complex_value(&ccv) != NGX_OK) {
        return NGX_ERROR;
    }

    if (cv.lengths == NULL) {

        if (value->len > MaxTextExtent - 1) {
            return NGX_ERROR;
        }

        option->option = *value;

    } else {

        option->option_cv = ngx_palloc(cf->pool, sizeof(ngx_http_complex_value_t));
        if (option->option_cv == NULL) {
            return NGX_ERROR;
        }

        *option->option_cv = cv;
    }

    return NGX_OK;
}

ngx_int_t
gm_parse_nth_option(ngx_conf_t *cf, ngx_array_t *args, ngx_uint_t n, void **option)
{
    ngx_http_gm_option_t       *opt;
    ngx_str_t                  *value;
    ngx_uint_t                  rc;

    if (args->nelts - n < 2) { /* gm $option $value */
        return NGX_ERROR;
    }

    value = args->elts;
    value += n + 1;            /* $value */

    dd("%s", value->data);

    opt = ngx_palloc(cf->pool, sizeof(ngx_http_gm_option_t));
    if (opt == NULL) {
        return NGX_ERROR;
    }
    ngx_memzero(opt, sizeof(ngx_http_gm_option_t));

    rc = gm_parse_option_value(cf, value, opt);
    if (rc == NGX_OK) {
        *option = opt;
    }

    return rc;
}

u_char * gm_get_option_value(ngx_http_request_t *r, ngx_http_gm_option_t *option)
{
    u_char                 *buf;
    ngx_str_t               str;

    if (option == NULL) {
        return NULL;
    }

    if (option->option_cv != NULL) {
        if (ngx_http_complex_value(r, option->option_cv, &str) != NGX_OK) {
            return NULL;
        }
    } else {
        str = option->option;
    }

    if (str.data == NULL) {
        return NULL;
    }

    buf = ngx_palloc(r->pool, str.len + 1);
    if (buf == NULL) {
        return NULL;
    }

    ngx_memcpy(buf, str.data, str.len);
    buf[str.len] = '\0';

    return buf;
}
