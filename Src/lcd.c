#include "lcd_internal.h"
#include "lcd_font.h"
#include <float.h>

LCD_Status lcd_combine(LCD_Status a, LCD_Status b)
{
    if (a >= LCD_INVALID_ARGUMENT) return a;
    if (b >= LCD_INVALID_ARGUMENT) return b;
    return a > b ? a : b;
}

LCD_Status lcd_clip(int32_t x, int32_t y, uint16_t w, uint16_t h, LCD_Region *r)
{
    int32_t right=x+w, bottom=y+h, left=x, top=y;
    r->w=0; r->h=0; r->skip_x=0; r->skip_y=0; r->clipped=0;
    if (w==0U || h==0U) return LCD_INVALID_ARGUMENT;
    if (right<=0 || bottom<=0 || x>=LCD_WIDTH || y>=LCD_HEIGHT) return LCD_CLIPPED;
    if (left<0) left=0;
    if (top<0) top=0;
    if (right>LCD_WIDTH) right=LCD_WIDTH;
    if (bottom>LCD_HEIGHT) bottom=LCD_HEIGHT;
    r->x=(uint16_t)left; r->y=(uint16_t)top;
    r->w=(uint16_t)(right-left); r->h=(uint16_t)(bottom-top);
    r->skip_x=(uint16_t)(left-x); r->skip_y=(uint16_t)(top-y);
    r->clipped=(uint8_t)(left!=x || top!=y || right!=x+w || bottom!=y+h);
    return r->clipped ? LCD_CLIPPED : LCD_OK;
}

const char *LCD_StatusName(LCD_Status status)
{
    static const char *const names[]={"OK","CLIPPED","MISSING_GLYPH","INVALID_UTF8",
        "INVALID_ARGUMENT","NOT_INITIALIZED","IO_ERROR","OVERFLOW"};
    return (unsigned)status < sizeof(names)/sizeof(names[0]) ? names[status] : "UNKNOWN";
}

LCD_Status LCD_FillRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    LCD_Region r;
    LCD_Status clipped, status;
    uint8_t buffer[LCD_TRANSFER_BYTES];
    uint32_t remaining;
    size_t i;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    clipped=lcd_clip(x,y,w,h,&r);
    if (clipped==LCD_INVALID_ARGUMENT || r.w==0U) return clipped;
    for (i=0; i<sizeof(buffer); i+=2U) {
        buffer[i]=(uint8_t)(color>>8); buffer[i+1U]=(uint8_t)color;
    }
    status=lcd_window(r.x,r.y,r.w,r.h);
    if (status!=LCD_OK) return status;
    remaining=(uint32_t)r.w*r.h*2U;
    while (remaining!=0U) {
        size_t count=remaining>sizeof(buffer) ? sizeof(buffer) : (size_t)remaining;
        if (!LCD_PortWrite(buffer,count)) {LCD_PortSelect(0);return LCD_IO_ERROR;}
        remaining-=(uint32_t)count;
    }
    LCD_PortSelect(0);
    return clipped;
}

LCD_Status LCD_Clear(uint16_t color) {return LCD_FillRect(0,0,LCD_WIDTH,LCD_HEIGHT,color);}
LCD_Status LCD_DrawPoint(int16_t x, int16_t y, uint16_t color) {return LCD_FillRect(x,y,1,1,color);}

LCD_Status LCD_DrawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
    int32_t x=x0, y=y0, dx=(int32_t)x1-x0, dy=(int32_t)y1-y0;
    int32_t sx=dx<0?-1:1, sy=dy<0?-1:1, error, twice;
    LCD_Status result=LCD_OK, status;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (dx<0) dx=-dx;
    if (dy<0) dy=-dy;
    error=dx-dy;
    for (;;) {
        status=LCD_DrawPoint((int16_t)x,(int16_t)y,color);
        if (status>=LCD_INVALID_ARGUMENT) return status;
        result=lcd_combine(result,status);
        if (x==x1 && y==y1) break;
        twice=2*error;
        if (twice>-dy) {error-=dy;x+=sx;}
        if (twice<dx) {error+=dx;y+=sy;}
    }
    return result;
}

LCD_Status LCD_DrawRect(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    int32_t right=(int32_t)x+w-1, bottom=(int32_t)y+h-1;
    LCD_Status result, status;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (w==0U || h==0U || right>32767 || bottom>32767) return LCD_INVALID_ARGUMENT;
    result=LCD_FillRect(x,y,w,1,color);
    if (result>=LCD_INVALID_ARGUMENT) return result;
    status=LCD_FillRect(x,(int16_t)bottom,w,1,color);
    if (status>=LCD_INVALID_ARGUMENT) return status;
    result=lcd_combine(result,status);
    status=LCD_FillRect(x,y,1,h,color);
    if (status>=LCD_INVALID_ARGUMENT) return status;
    result=lcd_combine(result,status);
    return lcd_combine(result,LCD_FillRect((int16_t)right,y,1,h,color));
}

LCD_Status LCD_DrawCircle(int16_t x, int16_t y, uint16_t radius, uint16_t color)
{
    int32_t a=radius,b=0,error=1-(int32_t)radius;
    LCD_Status result=LCD_OK, status;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (radius>(LCD_WIDTH>LCD_HEIGHT?LCD_WIDTH:LCD_HEIGHT)) return LCD_INVALID_ARGUMENT;
    while (a>=b) {
        int32_t px[8]={x+a,x-a,x+a,x-a,x+b,x-b,x+b,x-b};
        int32_t py[8]={y+b,y+b,y-b,y-b,y+a,y+a,y-a,y-a};
        unsigned i;
        for (i=0;i<8U;++i) {
            if (px[i]<0 || px[i]>=LCD_WIDTH || py[i]<0 || py[i]>=LCD_HEIGHT) {
                result=lcd_combine(result,LCD_CLIPPED);continue;
            }
            status=LCD_DrawPoint((int16_t)px[i],(int16_t)py[i],color);
            if (status>=LCD_INVALID_ARGUMENT) return status;
        }
        ++b;
        if (error<0) error+=2*b+1;
        else {--a;error+=2*(b-a)+1;}
    }
    return result;
}

LCD_Status LCD_DrawImage(int16_t x, int16_t y, uint16_t w, uint16_t h,
                         const uint8_t *data, size_t data_size)
{
    LCD_Region r;
    LCD_Status clipped,status;
    uint16_t row;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (data==NULL || w==0U || h==0U || (size_t)w>((size_t)-1)/2U/h ||
        data_size<(size_t)w*h*2U) return LCD_INVALID_ARGUMENT;
    clipped=lcd_clip(x,y,w,h,&r);
    if (r.w==0U) return clipped;
    status=lcd_window(r.x,r.y,r.w,r.h);
    if (status!=LCD_OK) return status;
    for (row=0;row<r.h;++row) {
        size_t offset=((size_t)(row+r.skip_y)*w+r.skip_x)*2U;
        if (!LCD_PortWrite(data+offset,(size_t)r.w*2U)) {LCD_PortSelect(0);return LCD_IO_ERROR;}
    }
    LCD_PortSelect(0);
    return clipped;
}

/* 错误序列每次消耗一字节；遇到截断字符串时不读取NUL之后的字节。 */
static LCD_Status decode_utf8(const char **cursor, uint32_t *codepoint)
{
    const uint8_t *p=(const uint8_t *)*cursor;
    uint32_t value,minimum;
    unsigned count,i;
    if (p[0]<0x80U) {*codepoint=p[0];++*cursor;return LCD_OK;}
    if (p[0]>=0xC2U && p[0]<=0xDFU) {count=2;value=p[0]&0x1FU;minimum=0x80U;}
    else if (p[0]>=0xE0U && p[0]<=0xEFU) {count=3;value=p[0]&0x0FU;minimum=0x800U;}
    else if (p[0]>=0xF0U && p[0]<=0xF4U) {count=4;value=p[0]&0x07U;minimum=0x10000U;}
    else {count=0;value=0;minimum=0;}
    for (i=1;i<count;++i) {
        if (p[i]==0U || (p[i]&0xC0U)!=0x80U) break;
        value=(value<<6)|(p[i]&0x3FU);
    }
    if (count!=0U && i==count && value>=minimum && value<=0x10FFFFU &&
        !(value>=0xD800U && value<=0xDFFFU)) {
        *cursor+=count;*codepoint=value;return LCD_OK;
    }
    ++*cursor;*codepoint=0xFFFDU;return LCD_INVALID_UTF8;
}

typedef struct {
    const uint8_t *pixels;
    uint8_t width,height,stride,top_padding,placeholder;
} LCD_Glyph;

static LCD_Status select_glyph(uint32_t codepoint, LCD_Font font, LCD_Glyph *glyph)
{
    uint16_t i;
    glyph->pixels=NULL;glyph->placeholder=0;
    glyph->height=(uint8_t)(font==LCD_FONT_MIXED32?32:24);
    glyph->top_padding=0;glyph->stride=0;
    if (codepoint<32U || codepoint==127U) return LCD_INVALID_ARGUMENT;
    if (codepoint<=126U) {
        glyph->pixels=lcd_ascii_12x24[codepoint-32U];
        glyph->width=12;glyph->stride=2;
        glyph->top_padding=(uint8_t)(font==LCD_FONT_MIXED32?4:0);
        return LCD_OK;
    }
    glyph->width=glyph->height;
    if (font==LCD_FONT_MIXED32) {
        for (i=0;i<lcd_chinese_count;++i) if (lcd_chinese_32[i].codepoint==codepoint) {
            glyph->pixels=lcd_chinese_32[i].pixels;glyph->stride=4;return LCD_OK;
        }
    }
    glyph->placeholder=1;
    return LCD_MISSING_GLYPH;
}

static LCD_Status draw_glyph(int32_t x, int32_t y, const LCD_Glyph *glyph,
                              uint16_t foreground, uint16_t background)
{
    LCD_Region r;
    LCD_Status clipped,status;
    uint8_t row_buffer[64];
    uint16_t row,col;
    clipped=lcd_clip(x,y,glyph->width,glyph->height,&r);
    if (r.w==0U) return clipped;
    status=lcd_window(r.x,r.y,r.w,r.h);
    if (status!=LCD_OK) return status;
    for (row=0;row<r.h;++row) {
        unsigned gy=row+r.skip_y;
        for (col=0;col<r.w;++col) {
            unsigned gx=col+r.skip_x;
            uint8_t ink=0;
            uint16_t color;
            if (glyph->placeholder) {
                ink=(uint8_t)(gx==0U || gy==0U || gx==glyph->width-1U ||
                    gy==glyph->height-1U || gx==gy);
            } else if (gy>=glyph->top_padding &&
                       gy<(unsigned)glyph->height-glyph->top_padding) {
                unsigned source_y=gy-glyph->top_padding;
                ink=(uint8_t)((glyph->pixels[source_y*glyph->stride+gx/8U]>>(gx%8U))&1U);
            }
            color=ink?foreground:background;
            row_buffer[col*2U]=(uint8_t)(color>>8);
            row_buffer[col*2U+1U]=(uint8_t)color;
        }
        if (!LCD_PortWrite(row_buffer,(size_t)r.w*2U)) {LCD_PortSelect(0);return LCD_IO_ERROR;}
    }
    LCD_PortSelect(0);
    return clipped;
}

LCD_Status LCD_MeasureText(const char *text, LCD_Font font, uint16_t *width,
                           uint16_t *height)
{
    uint32_t total=0,codepoint;
    LCD_Glyph glyph;
    LCD_Status result=LCD_OK,status;
    if (text==NULL || width==NULL || height==NULL ||
        (font!=LCD_FONT_ASCII24 && font!=LCD_FONT_MIXED32)) return LCD_INVALID_ARGUMENT;
    *width=0;*height=(uint16_t)(font==LCD_FONT_MIXED32?32:24);
    while (*text!='\0') {
        result=lcd_combine(result,decode_utf8(&text,&codepoint));
        status=select_glyph(codepoint,font,&glyph);
        if (status>=LCD_INVALID_ARGUMENT) return status;
        result=lcd_combine(result,status);
        total+=glyph.width;
        if (total>65535U) return LCD_OVERFLOW;
    }
    *width=(uint16_t)total;
    return result;
}

LCD_Status LCD_DrawText(int16_t x, int16_t y, const char *text, LCD_Font font,
                        uint16_t foreground, uint16_t background)
{
    int32_t cursor=x;
    uint32_t codepoint;
    LCD_Glyph glyph;
    LCD_Status result=LCD_OK,status;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (text==NULL || (font!=LCD_FONT_ASCII24 && font!=LCD_FONT_MIXED32)) return LCD_INVALID_ARGUMENT;
    while (*text!='\0') {
        if (cursor>=LCD_WIDTH) return lcd_combine(result,LCD_CLIPPED);
        result=lcd_combine(result,decode_utf8(&text,&codepoint));
        status=select_glyph(codepoint,font,&glyph);
        if (status>=LCD_INVALID_ARGUMENT) return status;
        result=lcd_combine(result,status);
        status=draw_glyph(cursor,y,&glyph,foreground,background);
        if (status>=LCD_INVALID_ARGUMENT) return status;
        result=lcd_combine(result,status);
        cursor+=glyph.width;
    }
    return result;
}

static char *append_unsigned(char *output, uint32_t value)
{
    char reverse[10];
    unsigned count=0;
    do {reverse[count++]=(char)('0'+value%10U);value/=10U;} while (value!=0U);
    while (count!=0U) *output++=reverse[--count];
    return output;
}

static LCD_Status draw_field(int16_t x, int16_t y, uint16_t field_width,
                              const char *text, LCD_Status format_status,
                              uint16_t foreground, uint16_t background)
{
    unsigned length=0;
    LCD_Status status;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (field_width==0U) return LCD_INVALID_ARGUMENT;
    /* 固定区域必须完整落在屏幕内，避免清除相邻数字或单位。 */
    if (x<0 || y<0 || (int32_t)x+field_width>LCD_WIDTH || (int32_t)y+24>LCD_HEIGHT)
        return LCD_CLIPPED;
    while (text[length]!='\0') ++length;
    if (length*12U>field_width) {
        text="OVER";length=4;format_status=LCD_OVERFLOW;
    }
    status=LCD_FillRect(x,y,field_width,24,background);
    if (status!=LCD_OK) return status;
    if (length*12U>field_width) return LCD_OVERFLOW;
    status=LCD_DrawText((int16_t)(x+field_width-length*12U),y,text,LCD_FONT_ASCII24,
                         foreground,background);
    return status==LCD_OK ? format_status : status;
}

LCD_Status LCD_DrawInt(int16_t x, int16_t y, uint16_t field_width, int32_t value,
                       uint16_t foreground, uint16_t background)
{
    char buffer[12],*end=buffer;
    uint32_t magnitude;
    if (value<0) {
        *end++='-';
        magnitude=(uint32_t)(-(value+1))+1U; /* 避免INT32_MIN直接取负溢出。 */
    } else magnitude=(uint32_t)value;
    end=append_unsigned(end,magnitude);
    *end='\0';
    return draw_field(x,y,field_width,buffer,LCD_OK,foreground,background);
}

LCD_Status LCD_DrawNumber(int16_t x, int16_t y, uint16_t field_width, double value,
                          uint8_t decimals, uint16_t foreground, uint16_t background)
{
    static const uint32_t scales[4]={1U,10U,100U,1000U};
    uint32_t scale,rounded,fraction,divisor;
    double magnitude,scaled;
    char buffer[16],*end=buffer;
    unsigned i;
    if (!lcd_initialized) return LCD_NOT_INITIALIZED;
    if (decimals>3U) return LCD_INVALID_ARGUMENT;
    if (value!=value || value>DBL_MAX || value<-DBL_MAX)
        return draw_field(x,y,field_width,"ERR",LCD_INVALID_ARGUMENT,foreground,background);
    magnitude=value<0.0 ? -value : value;
    scale=scales[decimals];
    /* 乘法前先检查范围，避免算术或浮点转整数溢出。 */
    if (magnitude>4294967294.5/(double)scale)
        return draw_field(x,y,field_width,"OVER",LCD_OVERFLOW,foreground,background);
    scaled=magnitude*(double)scale;
    rounded=(uint32_t)(scaled+0.5);
    if (value<0.0 && rounded!=0U) *end++='-';
    end=append_unsigned(end,rounded/scale);
    if (decimals!=0U) {
        *end++='.';fraction=rounded%scale;divisor=scale/10U;
        for (i=0;i<decimals;++i) {
            *end++=(char)('0'+fraction/divisor);
            fraction%=divisor;divisor/=10U;
        }
    }
    *end='\0';
    return draw_field(x,y,field_width,buffer,LCD_OK,foreground,background);
}
