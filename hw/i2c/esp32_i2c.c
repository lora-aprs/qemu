#include "qemu/osdep.h"
#include "qemu/log.h"
#include "qemu/module.h"
#include "hw/i2c/esp32_i2c.h"
#include "hw/irq.h"

enum {
    I2C_SCL_LOW_PERIOD_REG   = 0x0000,
    I2C_CTR_REG              = 0x0004,
    I2C_SR_REG               = 0x0008,
    I2C_TO_REG               = 0x000c,
    I2C_SLAVE_ADDR_REG       = 0x0010,
    I2C_RXFIFO_ST_REG        = 0x0014,
    I2C_FIFO_CONF_REG        = 0x0018,
    I2C_FIFO_DATA_REG        = 0x001c,
    I2C_INT_RAW_REG          = 0x0020,
    I2C_INT_CLR_REG          = 0x0024,
    I2C_INT_ENA_REG          = 0x0028,
    I2C_INT_STATUS_REG       = 0x002c,
    I2C_SDA_HOLD_REG         = 0x0030,
    I2C_SDA_SAMPLE_REG       = 0x0034,
    I2C_SCL_HIGH_PERIOD_REG  = 0x0038,
    I2C_SCL_START_HOLD_REG   = 0x0040,
    I2C_SCL_RSTART_SETUP_REG = 0x0044,
    I2C_SCL_STOP_HOLD_REG    = 0x0048,
    I2C_SCL_STOP_SETUP_REG   = 0x004c,
    I2C_SCL_FILTER_CFG_REG   = 0x0050,
    I2C_SDA_FILTER_CFG_REG   = 0x0054,
    I2C_COMD0_REG            = 0x0058,
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


//#define DEBUG_ESP32_I2C

#ifdef DEBUG_ESP32_I2C
#define DPRINTF(fmt, ...) \
do { printf("%s: " fmt , __func__, ## __VA_ARGS__); } while (0)
#define BADF(fmt, ...) \
do { fprintf(stderr, "%s: error: " fmt , __func__, ## __VA_ARGS__); exit(1);} while (0)
#else
#define DPRINTF(fmt, ...) do {} while(0)
#define BADF(fmt, ...) \
do { fprintf(stderr, "%s: error: " fmt , __func__, ## __VA_ARGS__);} while (0)
#endif

const char * I2C_REG_NAME[] =
{
    "I2C_SCL_LOW_PERIOD_REG",
    "I2C_CTR_REG",
    "I2C_SR_REG",
    "I2C_TO_REG",
    "I2C_SLAVE_ADDR_REG",
    "I2C_RXFIFO_ST_REG",
    "I2C_FIFO_CONF_REG",
    "I2C_FIFO_DATA_REG",
    "I2C_INT_RAW_REG",
    "I2C_INT_CLR_REG",
    "I2C_INT_ENA_REG",
    "I2C_INT_STATUS_REG",
    "I2C_SDA_HOLD_REG",
    "I2C_SDA_SAMPLE_REG",
    "I2C_SCL_HIGH_PERIOD_REG",
    "",
    "I2C_SCL_START_HOLD_REG",
    "I2C_SCL_RSTART_SETUP_REG",
    "I2C_SCL_STOP_HOLD_REG",
    "I2C_SCL_STOP_SETUP_REG",
    "I2C_SCL_FILTER_CFG_REG",
    "I2C_SDA_FILTER_CFG_REG",
    "I2C_COMD0_REG",
    "I2C_COMD1_REG",
    "I2C_COMD2_REG",
    "I2C_COMD3_REG",
    "I2C_COMD4_REG",
    "I2C_COMD5_REG",
    "I2C_COMD6_REG",
    "I2C_COMD7_REG",
    "I2C_COMD8_REG",
    "I2C_COMD9_REG",
    "I2C_COMD10_REG",
    "I2C_COMD11_REG",
    "I2C_COMD12_REG",
    "I2C_COMD13_REG",
    "I2C_COMD14_REG",
    "I2C_COMD15_REG",
    "scl_st_time_out",
    "scl_main_st_time_out",
    "scl_sp_conf"
};

#define ESP32_I2C_MEM_SIZE 0x1000
#define I2C_FIFO_LENGTH 32

static void esp32_i2c_reset(DeviceState * dev)
{
    Esp32I2CState * s = Esp32_I2C(dev);

    s->i2c_ctr_reg.val = 0;
    s->i2c_ctr_reg.I2C_SCL_FORCE_OUT = 1;
    s->i2c_ctr_reg.I2C_SDA_FORCE_OUT = 1;
    s->i2c_sr_reg.val = 0;
    s->i2c_fifo_conf_reg.val = 0;
    s->i2c_fifo_conf_reg.I2C_NONFIFO_RX_THRES = 0x15;
    s->i2c_fifo_conf_reg.I2C_NONFIFO_TX_THRES = 0x15;
    s->i2c_int_raw_reg.val = 0;
    s->i2c_int_ena_reg.val = 0;
    s->i2c_int_status_reg.val = 0;
    for(int i = 0; i < I2C_COMD_REG_COUNT; i++)
    {
        s->i2c_comd_reg[i].val = 0;
        s->i2c_comd_reg[i].I2C_COMMAND_DONE = 1;
    }

    fifo8_reset(&s->tx_fifo);
    fifo8_reset(&s->rx_fifo);
}

static uint64_t esp32_i2c_read(void * opaque, hwaddr addr, unsigned int size)
{
    DPRINTF("i2c reg read: %s (0x%x)\n", I2C_REG_NAME[addr/4], (int)addr);
    Esp32I2CState * s = Esp32_I2C(opaque);

    switch(addr)
    {
    case I2C_CTR_REG:
        DPRINTF("read: 0x%x\n", s->i2c_ctr_reg.val);
        return s->i2c_ctr_reg.val;
    case I2C_SR_REG:
        DPRINTF("read: 0x%x\n", s->i2c_sr_reg.val);
        return s->i2c_sr_reg.val;
    case I2C_FIFO_DATA_REG:
        return fifo8_pop(&s->rx_fifo);
    case I2C_FIFO_CONF_REG:
        DPRINTF("read: 0x%x\n", s->i2c_fifo_conf_reg.val);
        return s->i2c_fifo_conf_reg.val;
    case I2C_INT_RAW_REG:
        {
            DPRINTF("read: 0x%x\n", s->i2c_int_raw_reg.val);
            int temp = s->i2c_int_raw_reg.val;
            s->i2c_int_raw_reg.val = 0;
            return temp;
        }
    case I2C_INT_CLR_REG:
        return 0;
    case I2C_INT_ENA_REG:
        DPRINTF("read: 0x%x\n", s->i2c_int_ena_reg.val);
        return s->i2c_int_ena_reg.val;
    case I2C_INT_STATUS_REG:
        DPRINTF("read: 0x%x\n", s->i2c_int_status_reg.val);
        DPRINTF("read: 0x%x raw\n", s->i2c_int_raw_reg.val);
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
        DPRINTF("read: 0x%x\n", s->i2c_comd_reg[(addr - I2C_COMD0_REG) / 4].val);
        return s->i2c_comd_reg[(addr - I2C_COMD0_REG) / 4].val;
    default:
        DPRINTF("Unimplemented register %s (0x%x)\n", I2C_REG_NAME[addr/4], (int)addr);
        break;
    }
    return 0;
}

static void esp32_i2c_perform_write(Esp32I2CState * s)
{
    for(int i = 0; i < I2C_COMD_REG_COUNT; i++)
    {
        if(s->i2c_comd_reg[i].I2C_COMMAND_DONE)
        {
            continue;
        }
        char op_code = s->i2c_comd_reg[i].I2C_COMMAND_OP_CODE;
        DPRINTF("cmd i: %d, op: %d\n", i, op_code);

        switch (op_code)
        {
        case I2C_COMMAND_OP_CODE_RSTART:
            DPRINTF("I2C_COMMAND_OP_CODE_RSTART\n"); // just reset the bus
            s->i2c_sr_reg.I2C_BUS_BUSY = I2C_BUS_IDLE;
            i2c_end_transfer(s->bus);
            break;
        case I2C_COMMAND_OP_CODE_WRITE:
            {
                DPRINTF("I2C_COMMAND_OP_CODE_WRITE, leng: %d\n", s->i2c_comd_reg[i].I2C_COMMAND_BYTE_NUM);

                char leng = s->i2c_comd_reg[i].I2C_COMMAND_BYTE_NUM;

                if(s->i2c_sr_reg.I2C_BUS_BUSY == I2C_BUS_IDLE)
                {
                    if(fifo8_is_empty(&s->tx_fifo))
                    {
                        s->i2c_int_raw_reg.I2C_TX_FIFO_EMPTY = 1;
                        if(s->i2c_int_ena_reg.I2C_TX_FIFO_EMPTY)
                        {
                            s->i2c_int_status_reg.I2C_TX_FIFO_EMPTY = 1;
                            qemu_irq_raise(s->irq);
                        }
                        break;
                    }
                    uint8_t data = fifo8_pop(&s->tx_fifo);
                    uint8_t addr = data >> 1;
                    uint8_t read_write = data & 0x1;

                    DPRINTF("start_transfer: 0x%x, 0x%x\n", addr, read_write);
                    if(i2c_start_transfer(s->bus, addr, read_write) != 0)
                    {
                        DPRINTF("not found!\n");
                        s->i2c_sr_reg.I2C_ACK_REC = 0;
                        s->i2c_int_raw_reg.I2C_ACK_ERR = 1;
                        if(s->i2c_int_ena_reg.I2C_ACK_ERR)
                        {
                            s->i2c_int_status_reg.I2C_ACK_ERR = 1;
                            qemu_irq_raise(s->irq);
                        }
                        break;
                    }
                    DPRINTF("found\n");
                    s->i2c_sr_reg.I2C_BUS_BUSY = I2C_BUS_BUSY;
                    s->i2c_sr_reg.I2C_ACK_REC = 1;
                    s->i2c_sr_reg.I2C_BYTE_TRANS = 1;

                    if(s->i2c_comd_reg[i].I2C_COMMAND_ACK_CHECK_EN && s->i2c_comd_reg[i].I2C_COMMAND_ACK_EXP)
                    {
                        s->i2c_comd_reg[i].I2C_COMMAND_ACK_VALUE = 1;
                    }
                    else
                    {
                        s->i2c_comd_reg[i].I2C_COMMAND_ACK_VALUE = 0;
                    }

                    s->i2c_int_raw_reg.I2C_TRANS_START = 1;
                    if(s->i2c_int_ena_reg.I2C_TRANS_START)
                    {
                        s->i2c_int_status_reg.I2C_TRANS_START = 1;
                        qemu_irq_raise(s->irq);
                    }
                    leng--;
                }

                for(uint num_byte = 0; num_byte < leng; num_byte++)
                {
                    if(fifo8_is_empty(&s->tx_fifo))
                    {
                        s->i2c_int_raw_reg.I2C_TX_FIFO_EMPTY = 1;
                        if(s->i2c_int_ena_reg.I2C_TX_FIFO_EMPTY)
                        {
                            s->i2c_int_status_reg.I2C_TX_FIFO_EMPTY = 1;
                            qemu_irq_raise(s->irq);
                        }
                        break;
                    }
                    uint8_t data = fifo8_pop(&s->tx_fifo);
                    DPRINTF("i2c_send: 0x%x\n", data);
                    i2c_send(s->bus, data);
                    s->i2c_sr_reg.I2C_BYTE_TRANS = 1;
                }
                s->i2c_int_raw_reg.I2C_TX_SEND_EMPTY = 1;
                if(s->i2c_int_ena_reg.I2C_TX_SEND_EMPTY)
                {
                    s->i2c_int_status_reg.I2C_TX_SEND_EMPTY = 1;
                    qemu_irq_raise(s->irq);
                }
            }
            break;
        case I2C_COMMAND_OP_CODE_READ:
            {
                DPRINTF("I2C_COMMAND_OP_CODE_READ\n");
                if(fifo8_is_full(&s->rx_fifo))
                {
                    s->i2c_int_raw_reg.I2C_RX_FIFO_OVF = 1;
                    if(s->i2c_int_ena_reg.I2C_RX_FIFO_OVF)
                    {
                        s->i2c_int_status_reg.I2C_RX_FIFO_OVF = 1;
                        qemu_irq_raise(s->irq);
                    }
                    break;
                }
                uint8_t data = i2c_recv(s->bus);
                DPRINTF("i2c_recv: 0x%x\n", data);
                fifo8_push(&s->rx_fifo, data);
                s->i2c_sr_reg.I2C_RXFIFO_CNT++;
                if(fifo8_is_full(&s->rx_fifo))
                {
                    s->i2c_int_raw_reg.I2C_RX_FIFO_FULL = 1;
                    if(s->i2c_int_ena_reg.I2C_RX_FIFO_FULL)
                    {
                        s->i2c_int_status_reg.I2C_RX_FIFO_FULL = 1;
                        qemu_irq_raise(s->irq);
                    }
                }
            }
            break;
        case I2C_COMMAND_OP_CODE_STOP:
            DPRINTF("I2C_COMMAND_OP_CODE_STOP\n");
            s->i2c_sr_reg.I2C_BUS_BUSY = I2C_BUS_IDLE;
            i2c_end_transfer(s->bus);
            s->i2c_int_raw_reg.I2C_TRANS_COMPLETE = 1;
            if(s->i2c_int_ena_reg.I2C_TRANS_COMPLETE)
            {
                s->i2c_int_status_reg.I2C_TRANS_COMPLETE = 1;
                qemu_irq_raise(s->irq);
            }
            break;
        case I2C_COMMAND_OP_CODE_END:
            DPRINTF("I2C_COMMAND_OP_CODE_END\n");
            s->i2c_int_raw_reg.I2C_END_DETECT = 1;
            if(s->i2c_int_ena_reg.I2C_END_DETECT)
            {
                s->i2c_int_status_reg.I2C_END_DETECT = 1;
                qemu_irq_raise(s->irq);
            }
            break;
        default:
            break;
        }

        s->i2c_comd_reg[i].I2C_COMMAND_DONE = 1;
    }
}

static void esp32_i2c_write(void * opaque, hwaddr addr, uint64_t value, unsigned int size)
{
    DPRINTF("i2c reg write: %s (0x%x), value: 0x%x\n", I2C_REG_NAME[addr/4], (int)addr, (int)value);
    Esp32I2CState * s = Esp32_I2C(opaque);

    switch(addr)
    {
    case I2C_CTR_REG:
        s->i2c_ctr_reg.val = value;
        if(s->i2c_ctr_reg.I2C_MS_MODE == 1)
        {
            BADF("Slave mode not implemented!\n");
        }
        if(s->i2c_ctr_reg.I2C_TRANS_START)
        {
            esp32_i2c_perform_write(s);
        }
        s->i2c_ctr_reg.I2C_TRANS_START = 0;
        break;
    case I2C_SR_REG:
        s->i2c_sr_reg.val = value;
        break;
    case I2C_FIFO_DATA_REG:
        DPRINTF("---- FIFO DATA write: 0x%x ----\n", (int)value);
        fifo8_push(&s->tx_fifo, value);
        s->i2c_sr_reg.I2C_TXFIFO_CNT++;
        break;
    case I2C_FIFO_CONF_REG:
        s->i2c_fifo_conf_reg.val = value;
        if(s->i2c_fifo_conf_reg.I2C_RX_FIFO_RESET)
        {
            fifo8_reset(&s->rx_fifo);
        }
        if(s->i2c_fifo_conf_reg.I2C_TX_FIFO_RESET)
        {
            fifo8_reset(&s->tx_fifo);
        }
        break;
    case I2C_INT_CLR_REG:
        s->i2c_int_status_reg.val = 0;
        qemu_irq_lower(s->irq);
        break;
    case I2C_INT_ENA_REG:
        s->i2c_int_ena_reg.val = value;
        break;
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
        DPRINTF("comdXX: 0x%x, value: 0x%x\n", (int)(addr - I2C_COMD0_REG) / 4, (int)value);
        s->i2c_comd_reg[(addr - I2C_COMD0_REG) / 4].val = value;
        s->i2c_comd_reg[(addr - I2C_COMD0_REG) / 4].I2C_COMMAND_DONE = 0;
        break;
    default:
        DPRINTF("Unimplemented register %s (0x%x)\n", I2C_REG_NAME[addr/4], (int)addr);
        break;
    }
}

static const MemoryRegionOps esp32_i2c_ops = {
    .read = esp32_i2c_read,
    .write = esp32_i2c_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
};

static void esp32_i2c_init(Object * obj)
{
    Esp32I2CState *s = Esp32_I2C(obj);
    SysBusDevice *sbd = SYS_BUS_DEVICE(obj);

    memory_region_init_io(&s->iomem, obj, &esp32_i2c_ops, s, TYPE_ESP32_I2C, ESP32_I2C_MEM_SIZE);
    sysbus_init_mmio(sbd, &s->iomem);
    sysbus_init_irq(sbd, &s->irq);

    s->bus = i2c_init_bus(DEVICE(s), "i2c");
    bitbang_i2c_init(&s->bitbang, s->bus);

    fifo8_create(&s->tx_fifo, I2C_FIFO_LENGTH);
    fifo8_create(&s->rx_fifo, I2C_FIFO_LENGTH);
}

static void esp32_i2c_class_init(ObjectClass * klass, void * data)
{
    DeviceClass * dc = DEVICE_CLASS(klass);

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
