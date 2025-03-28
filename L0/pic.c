#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include "picture/flamingos.h"

int main(void)
{
    static AM_GPU_CONFIG_T cfg;
    static AM_INPUT_KEYBRD_T kfg;
    static AM_GPU_FBDRAW_T gcfg;
    ioe_init();
    ioe_read(AM_GPU_CONFIG, &cfg);
    int screenWidth = cfg.width;
    int screenHeight = cfg.height;

    // printf("%p\n",main);
    uint32_t *p = (uint32_t *)malloc(sizeof(uint32_t) * screenWidth * screenHeight);

    for (size_t i = 0; i < screenHeight; i++)
    {
        int imgI = i * imgHeight / screenHeight;
        for (size_t j = 0; j < screenWidth; j++)
        {
            int imgJ = j * imgWidth / screenWidth;
            p[i * screenWidth + j] = imageData[imgI * imgWidth + imgJ];
        }
    }

    gcfg.x = 0;
    gcfg.y = 0;
    gcfg.pixels = p;
    gcfg.w = screenWidth;
    gcfg.h = screenHeight;
    gcfg.sync = true;

    ioe_write(AM_GPU_FBDRAW, &gcfg);

    while (kfg.keycode != AM_KEY_ESCAPE)
    {
        ioe_read(AM_INPUT_KEYBRD, &kfg);
    }

    return 0;
}
