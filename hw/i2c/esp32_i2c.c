#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/i2c/esp32_i2c.h"
#include "hw/irq.h"

enum {
    I2C_SCL_LOW_PERIOD_REG   = 0x0000,
    I2C_CTR_REG              = 0x0004, // r, ww, struct
    I2C_SR_REG               = 0x0008, // r, wr, struct
    I2C_TO_REG               = 0x000c,
    I2C_SLAVE_ADDR_REG       = 0x0010,
    I2C_RXFIFO_ST_REG        = 0x0014,
    I2C_FIFO_CONF_REG        = 0x0018, // r, ww, struct
    I2C_INT_RAW_REG          = 0x0020, // r,   , struct
    I2C_INT_CLR_REG          = 0x0024, // r, ww
    I2C_INT_ENA_REG          = 0x0028, // r, wr, struct
    I2C_INT_STATUS_REG       = 0x002c, // r,   , struct
    I2C_SDA_HOLD_REG         = 0x0030,
    I2C_SDA_SAMPLE_REG       = 0x0034,
    I2C_SCL_HIGH_PERIOD_REG  = 0x0038,
    I2C_SCL_START_HOLD_REG   = 0x0040,
    I2C_SCL_RSTART_SETUP_REG = 0x0044,
    I2C_SCL_STOP_HOLD_REG    = 0x0048,
    I2C_SCL_STOP_SETUP_REG   = 0x004c,
    I2C_SCL_FILTER_CFG_REG   = 0x0050,
    I2C_SDA_FILTER_CFG_REG   = 0x0054,
    I2C_COMD0_REG            = 0x0058, // r, ww, struct
    I2C_COMD1_REG            = 0x005c,
    I2C_COMD2_REG            = 0x0060,
    I2C_COMD3_REG            = 0x0064,
    I2C_COMD4_REG            = 0x0068,
    I2C_COMD5_REG            = 0x006c,
    I2C_COMD6_REG            = 0x0070,
    I2C_COMD7_REG            = 0x0074,
    I2C_COMD8_REG            = 0x0078,
    I2C_COMD9_REG            = 0x007c,
    I2C_COMD10_REG           = 0x0080,
    I2C_COMD11_REG           = 0x0084,
    I2C_COMD12_REG           = 0x0088,
    I2C_COMD13_REG           = 0x008c,
    I2C_COMD14_REG           = 0x0090,
    I2C_COMD15_REG           = 0x0094,
    I2C_FIFO_START           = 0x0100
};

#define ESP32_I2C_MEM_SIZE 0x1000

static void esp32_i2c_reset(DeviceState *s)
{
}

static uint64_t esp32_i2c_readb(void * opaque, hwaddr addr, unsigned int size)
{
    Esp32I2CState * s = (Esp32I2CState *)opaque;

    switch(addr)
    {
    case I2C_CTR_REG:
        return s->i2c_ctr_reg.val;
    case I2C_SR_REG:
        return s->i2c_sr_reg.val;
    case I2C_FIFO_CONF_REG:
        return s->i2c_fifo_conf_reg.val;
    case I2C_INT_RAW_REG:
        {
            int temp = s->i2c_int_raw_reg.val;
            s->i2c_int_raw_reg.val = 0;
            return temp;
        }
    case I2C_INT_CLR_REG:
        return 0;
    case I2C_INT_ENA_REG:
        return s->i2c_int_ena_reg.val;
    case I2C_INT_STATUS_REG:
        return s->i2c_int_status_reg.val;
    case I2C_COMD0_REG:
    case I2C_COMD1_REG:
    case I2C_COMD2_REG:
    case I2C_COMD3_REG:
    case I2C_COMD4_REG:
    case I2C_COMD5_REG:
    case I2C_COMD6_REG:
    case I2C_COMD7_REG:
    case I2C_COMD8_REG:
    case I2C_COMD9_REG:
    case I2C_COMD10_REG:
    case I2C_COMD11_REG:
    case I2C_COMD12_REG:
    case I2C_COMD13_REG:
    case I2C_COMD14_REG:
    case I2C_COMD15_REG:
        return s->i2c_comd_reg[(addr - I2C_COMD0_REG * 4) / 4].val;
    default:
        break;
    }
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

    memory_region_init_io(&s->iomem, OBJECT(s), &esp32_i2c_ops, s, TYPE_ESP32_I2C, ESP32_I2C_MEM_SIZE);
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
