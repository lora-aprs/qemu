#ifndef ESP32_I2C_H
#define ESP32_I2C_H

#include "hw/sysbus.h"
#include "hw/i2c/bitbang_i2c.h"

#define TYPE_ESP32_I2C "esp32.i2c"
#define Esp32_I2C(obj) OBJECT_CHECK(Esp32I2CState, (obj), TYPE_ESP32_I2C)

typedef struct Esp32I2CState {
   SysBusDevice parent_obj;

   MemoryRegion iomem;
   qemu_irq irq;
   I2CBus *bus;
   
   bitbang_i2c_interface bitbang;
} Esp32I2CState;

#endif /* ESP32_I2C_H */
