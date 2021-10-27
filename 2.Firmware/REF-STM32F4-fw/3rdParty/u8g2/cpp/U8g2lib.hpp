#ifndef U8G2LIB_HH
#define U8G2LIB_HH

#include <Print.h>
#include "U8x8lib.h"
#include "u8g2.h"
#include "soft_i2c.h"

extern I2C_HandleTypeDef *U8G2_I2C_HANDLE;
extern SPI_HandleTypeDef *U8G2_SPI_HANDLE;

class U8G2 : public Print
{
protected:
    u8g2_t u8g2;
    u8x8_char_cb cpp_next_cb; /*  the cpp interface has its own decoding function for the Arduino print command */
public:
    u8g2_uint_t tx, ty;

    U8G2(void)
    {
        cpp_next_cb = u8x8_ascii_next;
        home();
    }

    u8x8_t *getU8x8(void)
    { return u8g2_GetU8x8(&u8g2); }

    u8g2_t *getU8g2(void)
    { return &u8g2; }

    void sendF(const char *fmt, ...)
    {
        va_list va;
        va_start(va, fmt);
        u8x8_cad_vsendf(u8g2_GetU8x8(&u8g2), fmt, va);
        va_end(va);
    }


    uint32_t getBusClock(void)
    { return u8g2_GetU8x8(&u8g2)->bus_clock; }

    void setBusClock(uint32_t clock_speed)
    { u8g2_GetU8x8(&u8g2)->bus_clock = clock_speed; }

    void setI2CAddress(uint8_t adr)
    { u8g2_SetI2CAddress(&u8g2, adr); }


    void enableUTF8Print(void)
    { cpp_next_cb = u8x8_utf8_next; }

    void disableUTF8Print(void)
    { cpp_next_cb = u8x8_ascii_next; }

    /* u8x8 interface */
    uint8_t getCols(void)
    { return u8x8_GetCols(u8g2_GetU8x8(&u8g2)); }

    uint8_t getRows(void)
    { return u8x8_GetRows(u8g2_GetU8x8(&u8g2)); }

    void drawTile(uint8_t x, uint8_t y, uint8_t cnt, uint8_t *tile_ptr)
    {
        u8x8_DrawTile(u8g2_GetU8x8(&u8g2), x, y, cnt, tile_ptr);
    }

    /* return 0 for no event or U8X8_MSG_GPIO_MENU_SELECT, */
    /* U8X8_MSG_GPIO_MENU_NEXT, U8X8_MSG_GPIO_MENU_PREV, */
    /* U8X8_MSG_GPIO_MENU_HOME */
    uint8_t getMenuEvent(void)
    { return u8x8_GetMenuEvent(u8g2_GetU8x8(&u8g2)); }

    void initDisplay(void)
    {
        u8g2_InitDisplay(&u8g2);
    }

    void clearDisplay(void)
    {
        u8g2_ClearDisplay(&u8g2);
    }

    void setPowerSave(uint8_t is_enable)
    {
        u8g2_SetPowerSave(&u8g2, is_enable);
    }

    void setFlipMode(uint8_t mode)
    {
        u8g2_SetFlipMode(&u8g2, mode);
    }

    void setContrast(uint8_t value)
    {
        u8g2_SetContrast(&u8g2, value);
    }

    void setDisplayRotation(const u8g2_cb_t *u8g2_cb)
    {
        u8g2_SetDisplayRotation(&u8g2, u8g2_cb);
    }


    bool Init()
    {
        if (U8G2_I2C_HANDLE->Instance == I2C_SOFT)
            Soft_I2C_Init();

        /* note: call to u8x8_utf8_init is not required here, this is done in the setup procedures before */
        setPowerSave(1);
        initDisplay();
        clearDisplay();
        clearBuffer();
        setPowerSave(0);

        return 1;
    }

    void beginSimple()
    {
        /* does not clear the display and does not wake up the display */
        /* user is responsible for calling clearDisplay() and setPowerSave(0) */
        initDisplay();
    }

    /* u8g2  */

#ifdef U8G2_WITH_CLIP_WINDOW_SUPPORT

    void setMaxClipWindow()
    { u8g2_SetMaxClipWindow(&u8g2); }

    void setClipWindow(u8g2_uint_t clip_x0, u8g2_uint_t clip_y0, u8g2_uint_t clip_x1, u8g2_uint_t clip_y1)
    {
        u8g2_SetClipWindow(&u8g2, clip_x0, clip_y0, clip_x1, clip_y1);
    }

#endif /* U8G2_WITH_CLIP_WINDOW_SUPPORT */


    u8g2_uint_t getDisplayHeight(void)
    { return u8g2_GetDisplayHeight(&u8g2); }

    u8g2_uint_t getDisplayWidth(void)
    { return u8g2_GetDisplayWidth(&u8g2); }


    /* u8g2_buffer.c */
    void sendBuffer(void)
    { u8g2_SendBuffer(&u8g2); }

    void clearBuffer(void)
    { u8g2_ClearBuffer(&u8g2); }

    void firstPage(void)
    { u8g2_FirstPage(&u8g2); }

    uint8_t nextPage(void)
    { return u8g2_NextPage(&u8g2); }

    uint8_t *getBufferPtr(void)
    { return u8g2_GetBufferPtr(&u8g2); }

    uint8_t getBufferTileHeight(void)
    { return u8g2_GetBufferTileHeight(&u8g2); }

    uint8_t getBufferTileWidth(void)
    { return u8g2_GetBufferTileWidth(&u8g2); }

    uint8_t getPageCurrTileRow(void)
    { return u8g2_GetBufferCurrTileRow(&u8g2); }    // obsolete
    void setPageCurrTileRow(uint8_t row)
    { u8g2_SetBufferCurrTileRow(&u8g2, row); }    // obsolete
    uint8_t getBufferCurrTileRow(void)
    { return u8g2_GetBufferCurrTileRow(&u8g2); }

    void setBufferCurrTileRow(uint8_t row)
    { u8g2_SetBufferCurrTileRow(&u8g2, row); }

    // this should be renamed to setBufferAutoClear
    void setAutoPageClear(uint8_t mode)
    { u8g2_SetAutoPageClear(&u8g2, mode); }

    void updateDisplayArea(uint8_t tx, uint8_t ty, uint8_t tw, uint8_t th)
    { u8g2_UpdateDisplayArea(&u8g2, tx, ty, tw, th); }

    void updateDisplay(void)
    { u8g2_UpdateDisplay(&u8g2); }

    void refreshDisplay(void)
    { u8x8_RefreshDisplay(u8g2_GetU8x8(&u8g2)); }


    /* clib/u8g2.hvline.c */
    void setDrawColor(uint8_t color_index)
    { u8g2_SetDrawColor(&u8g2, color_index); }

    uint8_t getDrawColor(void)
    { return u8g2_GetDrawColor(&u8g2); }

    void drawPixel(u8g2_uint_t x, u8g2_uint_t y)
    { u8g2_DrawPixel(&u8g2, x, y); }

    void drawHLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w)
    { u8g2_DrawHLine(&u8g2, x, y, w); }

    void drawVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t h)
    { u8g2_DrawVLine(&u8g2, x, y, h); }

    void drawHVLine(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t len, uint8_t dir)
    {
        u8g2_DrawHVLine(&u8g2, x, y, len, dir);
    }

    /* u8g2_box.c */
    void drawFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
    { u8g2_DrawFrame(&u8g2, x, y, w, h); }

    void drawRFrame(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
    { u8g2_DrawRFrame(&u8g2, x, y, w, h, r); }

    void drawBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h)
    { u8g2_DrawBox(&u8g2, x, y, w, h); }

    void drawRBox(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, u8g2_uint_t r)
    { u8g2_DrawRBox(&u8g2, x, y, w, h, r); }

    /* u8g2_circle.c */
    void drawCircle(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t opt = U8G2_DRAW_ALL)
    { u8g2_DrawCircle(&u8g2, x0, y0, rad, opt); }

    void drawDisc(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rad, uint8_t opt = U8G2_DRAW_ALL)
    { u8g2_DrawDisc(&u8g2, x0, y0, rad, opt); }

    void drawEllipse(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t opt = U8G2_DRAW_ALL)
    { u8g2_DrawEllipse(&u8g2, x0, y0, rx, ry, opt); }

    void drawFilledEllipse(u8g2_uint_t x0, u8g2_uint_t y0, u8g2_uint_t rx, u8g2_uint_t ry, uint8_t opt = U8G2_DRAW_ALL)
    { u8g2_DrawFilledEllipse(&u8g2, x0, y0, rx, ry, opt); }

    /* u8g2_line.c */
    void drawLine(u8g2_uint_t x1, u8g2_uint_t y1, u8g2_uint_t x2, u8g2_uint_t y2)
    { u8g2_DrawLine(&u8g2, x1, y1, x2, y2); }

    /* u8g2_bitmap.c */
    void setBitmapMode(uint8_t is_transparent)
    { u8g2_SetBitmapMode(&u8g2, is_transparent); }

    void drawBitmap(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t cnt, u8g2_uint_t h, const uint8_t *bitmap)
    { u8g2_DrawBitmap(&u8g2, x, y, cnt, h, bitmap); }

    void drawXBM(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap)
    { u8g2_DrawXBM(&u8g2, x, y, w, h, bitmap); }

    void drawXBMP(u8g2_uint_t x, u8g2_uint_t y, u8g2_uint_t w, u8g2_uint_t h, const uint8_t *bitmap)
    { u8g2_DrawXBMP(&u8g2, x, y, w, h, bitmap); }


    /* u8g2_polygon.c */
    void drawTriangle(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2)
    { u8g2_DrawTriangle(&u8g2, x0, y0, x1, y1, x2, y2); }

    /* u8log_u8g2.c */
    void drawLog(u8g2_uint_t x, u8g2_uint_t y, class U8G2LOG &u8g2log);

    /* u8g2_font.c */

    void setFont(const uint8_t *font)
    { u8g2_SetFont(&u8g2, font); }

    void setFontMode(uint8_t is_transparent)
    { u8g2_SetFontMode(&u8g2, is_transparent); }

    void setFontDirection(uint8_t dir)
    { u8g2_SetFontDirection(&u8g2, dir); }

    int8_t getAscent(void)
    { return u8g2_GetAscent(&u8g2); }

    int8_t getDescent(void)
    { return u8g2_GetDescent(&u8g2); }

    void setFontPosBaseline(void)
    { u8g2_SetFontPosBaseline(&u8g2); }

    void setFontPosBottom(void)
    { u8g2_SetFontPosBottom(&u8g2); }

    void setFontPosTop(void)
    { u8g2_SetFontPosTop(&u8g2); }

    void setFontPosCenter(void)
    { u8g2_SetFontPosCenter(&u8g2); }

    void setFontRefHeightText(void)
    { u8g2_SetFontRefHeightText(&u8g2); }

    void setFontRefHeightExtendedText(void)
    { u8g2_SetFontRefHeightExtendedText(&u8g2); }

    void setFontRefHeightAll(void)
    { u8g2_SetFontRefHeightAll(&u8g2); }


/*
uint8_t u8g2_IsGlyph(u8g2_t *u8g2, uint16_t requested_encoding);
int8_t u8g2_GetGlyphWidth(u8g2_t *u8g2, uint16_t requested_encoding);
u8g2_uint_t u8g2_GetStrWidth(u8g2_t *u8g2, const char *s);
u8g2_uint_t u8g2_GetUTF8Width(u8g2_t *u8g2, const char *str);
*/

    u8g2_uint_t drawGlyph(u8g2_uint_t x, u8g2_uint_t y, uint16_t encoding)
    { return u8g2_DrawGlyph(&u8g2, x, y, encoding); }

    u8g2_uint_t drawStr(u8g2_uint_t x, u8g2_uint_t y, const char *s)
    { return u8g2_DrawStr(&u8g2, x, y, s); }

    u8g2_uint_t drawUTF8(u8g2_uint_t x, u8g2_uint_t y, const char *s)
    { return u8g2_DrawUTF8(&u8g2, x, y, s); }

    u8g2_uint_t drawExtUTF8(u8g2_uint_t x, u8g2_uint_t y, uint8_t to_left, const uint16_t *kerning_table, const char *s)
    { return u8g2_DrawExtUTF8(&u8g2, x, y, to_left, kerning_table, s); }


    u8g2_uint_t getStrWidth(const char *s)
    { return u8g2_GetStrWidth(&u8g2, s); }

    u8g2_uint_t getUTF8Width(const char *s)
    { return u8g2_GetUTF8Width(&u8g2, s); }

    // not required any more, enable UTF8 for print 
    //void printUTF8(const char *s) { tx += u8g2_DrawUTF8(&u8g2, tx, ty, s); }

    /* screenshot functions for full buffer mode */
    /* vertical top lsb memory architecture */
    void writeBufferPBM(Print &p);

    void writeBufferXBM(Print &p);
    /* horizontal right lsb memory architecture */
    /* SH1122, LD7032, ST7920, ST7986, LC7981, T6963, SED1330, RA8835, MAX7219, LS0 */
    void writeBufferPBM2(Print &p);

    void writeBufferXBM2(Print &p);

    /* virtual function for print base class */
    size_t write(uint8_t v)
    {
        uint16_t e = cpp_next_cb(&(u8g2.u8x8), v);

        if (e < 0x0fffe)
        {
            u8g2_uint_t delta = u8g2_DrawGlyph(&u8g2, tx, ty, e);

#ifdef U8G2_WITH_FONT_ROTATION
            switch (u8g2.font_decode.dir)
            {
                case 0:
                    tx += delta;
                    break;
                case 1:
                    ty += delta;
                    break;
                case 2:
                    tx -= delta;
                    break;
                case 3:
                    ty -= delta;
                    break;
            }

            // requires 10 bytes more on avr
            //tx = u8g2_add_vector_x(tx, delta, 0, u8g2.font_decode.dir);
            //ty = u8g2_add_vector_y(ty, delta, 0, u8g2.font_decode.dir);

#else
            tx += delta;
#endif

        }
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size)
    {
        size_t cnt = 0;
        while (size > 0)
        {
            cnt += write(*buffer++);
            size--;
        }
        return cnt;
    }


    /* user interface */
/*
uint8_t u8g2_UserInterfaceSelectionList(u8g2_t *u8g2, const char *title, uint8_t start_pos, const char *sl);
uint8_t u8g2_UserInterfaceMessage(u8g2_t *u8g2, const char *title1, const char *title2, const char *title3, const char *buttons);
uint8_t u8g2_UserInterfaceInputValue(u8g2_t *u8g2, const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits, const char *post);
*/

    uint8_t userInterfaceSelectionList(const char *title, uint8_t start_pos, const char *sl)
    {
        return u8g2_UserInterfaceSelectionList(&u8g2, title, start_pos, sl);
    }

    uint8_t userInterfaceMessage(const char *title1, const char *title2, const char *title3, const char *buttons)
    {
        return u8g2_UserInterfaceMessage(&u8g2, title1, title2, title3, buttons);
    }

    uint8_t
    userInterfaceInputValue(const char *title, const char *pre, uint8_t *value, uint8_t lo, uint8_t hi, uint8_t digits,
                            const char *post)
    {
        return u8g2_UserInterfaceInputValue(&u8g2, title, pre, value, lo, hi, digits, post);
    }


    /* LiquidCrystal compatible functions */
    void home(void)
    {
        tx = 0;
        ty = 0;
        u8x8_utf8_init(u8g2_GetU8x8(&u8g2));
    }

    void clear(void)
    {
        home();
        clearDisplay();
        clearBuffer();
    }

    void noDisplay(void)
    { u8g2_SetPowerSave(&u8g2, 1); }

    void display(void)
    { u8g2_SetPowerSave(&u8g2, 0); }

    void setCursor(u8g2_uint_t x, u8g2_uint_t y)
    {
        tx = x;
        ty = y;
    }

    /* u8glib compatible functions */
    void sleepOn(void)
    { u8g2_SetPowerSave(&u8g2, 1); }

    void sleepOff(void)
    { u8g2_SetPowerSave(&u8g2, 0); }

    void setColorIndex(uint8_t color_index)
    { u8g2_SetDrawColor(&u8g2, color_index); }

    uint8_t getColorIndex(void)
    { return u8g2_GetDrawColor(&u8g2); }

    int8_t getFontAscent(void)
    { return u8g2_GetAscent(&u8g2); }

    int8_t getFontDescent(void)
    { return u8g2_GetDescent(&u8g2); }

    int8_t getMaxCharHeight(void)
    { return u8g2_GetMaxCharHeight(&u8g2); }

    int8_t getMaxCharWidth(void)
    { return u8g2_GetMaxCharWidth(&u8g2); }

    u8g2_uint_t getHeight()
    { return u8g2_GetDisplayHeight(&u8g2); }

    u8g2_uint_t getWidth()
    { return u8g2_GetDisplayWidth(&u8g2); }
};

void u8g2_print_callback(const char *s);  /* U8g2lib.cpp */


class U8G2LOG : public Print
{

public:
    u8log_t u8log;

    /* the constructor does nothing, use begin() instead */
    U8G2LOG(void)
    {}

    /* connect to u8g2, draw to u8g2 whenever required */
    bool begin(class U8G2 &u8g2, uint8_t width, uint8_t height, uint8_t *buf)
    {
        u8log_Init(&u8log, width, height, buf);
        u8log_SetCallback(&u8log, u8log_u8g2_cb, u8g2.getU8g2());
        return true;
    }

    /* disconnected version, manual redraw required */
    bool begin(uint8_t width, uint8_t height, uint8_t *buf)
    {
        u8log_Init(&u8log, width, height, buf);
        return true;
    }

    void setLineHeightOffset(int8_t line_height_offset)
    {
        u8log_SetLineHeightOffset(&u8log, line_height_offset);
    }

    void setRedrawMode(uint8_t is_redraw_line_for_each_char)
    {
        u8log_SetRedrawMode(&u8log, is_redraw_line_for_each_char);
    }

    /* virtual function for print base class */
    size_t write(uint8_t v)
    {
        u8log_WriteChar(&u8log, v);
        return 1;
    }

    size_t write(const uint8_t *buffer, size_t size)
    {
        size_t cnt = 0;
        while (size > 0)
        {
            cnt += write(*buffer++);
            size--;
        }
        return cnt;
    }

    void writeString(const char *s)
    { u8log_WriteString(&u8log, s); }

    void writeChar(uint8_t c)
    { u8log_WriteChar(&u8log, c); }

    void writeHex8(uint8_t b)
    { u8log_WriteHex8(&u8log, b); }

    void writeHex16(uint16_t v)
    { u8log_WriteHex16(&u8log, v); }

    void writeHex32(uint32_t v)
    { u8log_WriteHex32(&u8log, v); }

    void writeDec8(uint8_t v, uint8_t d)
    { u8log_WriteDec8(&u8log, v, d); }

    void writeDec16(uint8_t v, uint8_t d)
    { u8log_WriteDec16(&u8log, v, d); }
};

/* u8log_u8g2.c */
inline void U8G2::drawLog(u8g2_uint_t x, u8g2_uint_t y, class U8G2LOG &u8g2log)
{
    u8g2_DrawLog(&u8g2, x, y, &(u8g2log.u8log));
}


class U8G2_BITMAP : public U8G2
{
public:
    U8G2_BITMAP(uint16_t pixel_width, uint16_t pixel_height, const u8g2_cb_t *rotation)
    {
        u8g2_SetupBitmap(getU8g2(), rotation, pixel_width, pixel_height);
    }

    // This completely resets various settings, such as the
    // font, so be sure to re-initialize things
    void changeSize(uint16_t pixel_width, uint16_t pixel_height)
    {
        u8g2_SetupBitmap(getU8g2(), getU8g2()->cb, pixel_width, pixel_height);
    }
};

class SSD1306 : public U8G2
{
private:

public:
    explicit SSD1306(I2C_HandleTypeDef *_hi2c, const u8g2_cb_t *rotation = U8G2_R3) : U8G2()
    {
        U8G2_I2C_HANDLE = _hi2c;
        u8g2_Setup_ssd1306_i2c_128x80_noname_f(&u8g2, rotation, u8x8_byte_stm32_hw_i2c, u8x8_stm32_gpio_and_delay);
    }
};

#endif /* _U8G2LIB_HH */

