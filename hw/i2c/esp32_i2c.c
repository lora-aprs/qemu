#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/i2c/esp32_i2c.h"
#include "hw/irq.h"

#define ESP32_I2C_MEM_SIZE 0x1000

static void esp32_i2c_reset(DeviceState *s)
{
}

static uint64_t esp32_i2c_readb(void *opaque, hwaddr addr, unsigned int size)
{
    return 0;
}

static void esp32_i2c_writeb(void *opaque, hwaddr addr, uint64_t value, unsigned int size)
{
}

static const MemoryRegionOps esp32_i2c_ops = {
    .read = esp32_i2c_readb,
    .write = esp32_i2c_writeb,
    .valid.min_access_size = 1,
    .valid.max_access_size = 4,
    .impl.min_access_size = 1,
    .impl.max_access_size = 1,
    .endianness = DEVICE_NATIVE_ENDIAN,
};

static void esp32_i2c_init(Object *o)
{
    Esp32I2CState *s = Esp32_I2C(o);

    memory_region_init_io(&s->iomem, OBJECT(s), &esp32_i2c_ops, s,
                          TYPE_ESP32_I2C, ESP32_I2C_MEM_SIZE);
    sysbus_init_mmio(SYS_BUS_DEVICE(s), &s->iomem);
    sysbus_init_irq(SYS_BUS_DEVICE(s), &s->irq);
    s->bus = i2c_init_bus(DEVICE(s), "i2c");
    bitbang_i2c_init(&s->bitbang, s->bus);
}

static void esp32_i2c_class_init(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);

    dc->reset = esp32_i2c_reset;
}

static const TypeInfo esp32_i2c_type_info = {
    .name = TYPE_ESP32_I2C,
    .parent = TYPE_SYS_BUS_DEVICE,
    .instance_size = sizeof(Esp32I2CState),
    .instance_init = esp32_i2c_init,
    .class_init = esp32_i2c_class_init,
};

static void esp32_i2c_register_types(void)
{
    type_register_static(&esp32_i2c_type_info);
}

type_init(esp32_i2c_register_types)
