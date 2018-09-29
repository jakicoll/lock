#include "globals.h"

#include "epaper.h"

const epd_pinmap epd_pins = {.cs = 14, .dc = 25, .rst = 18, .busy = 0};

#define COLORED 0
#define UNCOLORED 1

Epd epd;
QRCode qrcode;
unsigned char image[5025];
Paint paint(image, 200, 200);

void wakeEpaper() {
  if (epd.Init(lut_full_update) != 0) {
    ESP_LOGE(TAG, "error=\"e-paper init failed\"");
    return;
  }
}

static void epaper_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                         const lv_color_t *color_p) {
  int32_t x;
  int32_t y;
  for (y = y1; y <= y2; y++) {
    for (x = x1; x <= x2; x++) {
      if (lv_color_to1(*color_p) == 0) {
        paint.DrawPixel(x, y, COLORED);
      }
      color_p++;
    }
  }

  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(),
                     paint.GetHeight());
  lv_flush_ready();
}

void setupEpaper() {
  wakeEpaper();

  epd.ClearFrameMemory(0xFF);
  epd.DisplayFrame();

  epd.SetFrameMemory(LOGO_IMAGE_DATA);
  epd.DisplayFrame();

  delay(1000);
  paint.Clear(UNCOLORED);

  renderQR("https://radforschung.org");

  lv_init();

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.disp_flush = epaper_flush;
  lv_disp_drv_register(&disp_drv);

  lv_obj_t *label1 = lv_label_create(lv_scr_act(), NULL);
  lv_label_set_text(label1, "radforschung.org");
  lv_obj_align(label1, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, -5);

  lv_tick_inc(1);
  lv_task_handler();

  epd.DisplayFrame();

  epd.Sleep();
}

void renderQR(char *text) {
  const int qrversion = 4;
  uint8_t qrcodeData[qrcode_getBufferSize(qrversion)];
  qrcode_initText(&qrcode, qrcodeData, qrversion, ECC_MEDIUM, text);

  const int LEFT_PADDING = 20;
  const int TOP_PADDING = 10;
  const int SIZE_MODIFIER = 5;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      if (qrcode_getModule(&qrcode, x, y)) {
        paint.DrawFilledRectangle(
            LEFT_PADDING + x * SIZE_MODIFIER - SIZE_MODIFIER + 1,
            TOP_PADDING + y * SIZE_MODIFIER - SIZE_MODIFIER + 1,
            LEFT_PADDING + x * SIZE_MODIFIER, TOP_PADDING + y * SIZE_MODIFIER,
            COLORED);
      }
    }
  }

  epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(),
                     paint.GetHeight());
  epd.DisplayFrame();
}