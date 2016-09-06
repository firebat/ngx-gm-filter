#ifndef NGX_HTTP_GM_COMMON_H
#define NGX_HTTP_GM_COMMON_H

#include "ddebug.h"

#include <ngx_http.h>
#include <magick/api.h>


extern MagickExport CompositeOperator StringToCompositeOperator(const char *option);
extern MagickExport CompressionType StringToCompressionType(const char *option);
extern MagickExport FilterTypes StringToFilterTypes(const char *option);
extern MagickExport GravityType StringToGravityType(const char *option);
extern MagickExport ImageType StringToImageType(const char *option);
extern MagickExport InterlaceType StringToInterlaceType(const char *option);
extern MagickExport void FormatString(char *string,const char *format,...);

extern MagickExport const char *CompositeOperatorToString(const CompositeOperator composite_op);
extern MagickExport const char *CompressionTypeToString(const CompressionType compression_type);
extern MagickExport const char *ImageTypeToString(const ImageType image_type);
extern MagickExport const char *InterlaceTypeToString(const InterlaceType interlace_type);
extern MagickExport const char *OrientationTypeToString(const OrientationType orientation_type);


/* option */
typedef struct {
    ngx_str_t                   option;
    ngx_http_complex_value_t   *option_cv;
} ngx_http_gm_option_t;

ngx_int_t gm_parse_nth_option(ngx_conf_t *cf, ngx_array_t *args, ngx_uint_t start, void **option);
u_char *  gm_get_option_value(ngx_http_request_t *r, ngx_http_gm_option_t *option);

#endif
