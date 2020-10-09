#ifndef ESP32_I2C_H
#define ESP32_I2C_H

#include "hw/sysbus.h"
#include "qemu/fifo8.h"
#include "hw/i2c/bitbang_i2c.h"

#define TYPE_ESP32_I2C "esp32.i2c"
#define Esp32_I2C(obj) OBJECT_CHECK(Esp32I2CState, (obj), TYPE_ESP32_I2C)

// I2C_CTR_REG
typedef union {
    struct {
        uint32_t I2C_SDA_FORCE_OUT    : 1;
        uint32_t I2C_SCL_FORCE_OUT    : 1;
        uint32_t I2C_SAMPLE_SCL_LEVEL : 1;
        uint32_t                      : 1;
        uint32_t I2C_MS_MODE          : 1;
        uint32_t I2C_TRANS_START      : 1;
        uint32_t I2C_TX_LSB_FIRST     : 1;
        uint32_t I2C_RX_LSB_FIRST     : 1;
        uint32_t                      : 2;
        uint32_t I2C_FSM_RESET        : 1;
        uint32_t                      : 21;
    };
    uint32_t val;
} i2c_ctr_reg_t;

// I2C_SR_REG
enum i2c_scl_state_last_t
{
    I2C_SCL_STATE_LAST_IDLE          = 0,
    I2C_SCL_STATE_LAST_START         = 1,
    I2C_SCL_STATE_LAST_NEGATIVE_EDGE = 3,
    I2C_SCL_STATE_LAST_LOW           = 4,
    I2C_SCL_STATE_LAST_POSITIVE_EDGE = 4,
    I2C_SCL_STATE_LAST_HIGH          = 5,
    I2C_SCL_STATE_LAST_STOP          = 6,
};

enum i2c_scl_main_state_last_t
{
    I2C_SCL_MAIN_STATE_LAST_IDLE          = 0,
    I2C_SCL_MAIN_STATE_LAST_ADDRESS_SHIFT = 1,
    I2C_SCL_MAIN_STATE_LAST_ACK_ADRESS    = 2,
    I2C_SCL_MAIN_STATE_LAST_RS_DATA       = 3,
    I2C_SCL_MAIN_STATE_LAST_TX_DATA       = 4,
    I2C_SCL_MAIN_STATE_LAST_SEND_ACK      = 5,
    I2C_SCL_MAIN_STATE_LAST_WAIT_ACK      = 6,
};

enum i2c_bus_busy_t
{
    I2C_BUS_IDLE = 0,
    I2C_BUS_BUSY = 1,
};

enum i2c_slave_rw_t
{
    I2C_SLAVE_RW_MASTER_WRITE = 0,
    I2C_SLAVE_RW_MASTER_READ  = 1,
};

typedef union {
    struct {
        uint32_t                       I2C_ACK_REC             : 1;
        enum i2c_slave_rw_t            I2C_SLAVE_RW            : 1;
        uint32_t                       I2C_TIME_OUT            : 1;
        uint32_t                       I2C_ARB_LOST            : 1;
        enum i2c_bus_busy_t            I2C_BUS_BUSY            : 1;
        uint32_t                       I2C_SLAVE_ADDRESSED     : 1;
        uint32_t                       I2C_BYTE_TRANS          : 1;
        uint32_t                                               : 1;
        uint32_t                       I2C_RXFIFO_CNT          : 6;
        uint32_t                                               : 4;
        uint32_t                       I2C_TXFIFO_CNT          : 6;
        enum i2c_scl_main_state_last_t I2C_SCL_MAIN_STATE_LAST : 3;
        uint32_t                                               : 1;
        enum i2c_scl_state_last_t      I2C_SCL_STATE_LAST      : 3;
        uint32_t                                               : 1;
    };
    uint32_t val;
} i2c_sr_reg_t;

// I2C_FIFO_CONF_REG
typedef union {
    struct {
        uint32_t I2C_RX_FIFO_FULL_THRHD  : 5;
        uint32_t I2C_TX_FIFO_EMPTY_THRHD : 5;
        uint32_t I2C_NONFIFO_EN          : 1;
        uint32_t I2C_FIFO_ADDR_CFG_EN    : 1;
        uint32_t I2C_RX_FIFO_RESET       : 1;
        uint32_t I2C_TX_FIFO_RESET       : 1;
        uint32_t I2C_NONFIFO_RX_THRES    : 6;
        uint32_t I2C_NONFIFO_TX_THRES    : 6;
        uint32_t                         : 6;
    };
    uint32_t val;
} i2c_fifo_conf_reg_t;

// I2C_FIFO_DATA
typedef union {
    struct {
        uint32_t I2C_FIFO_DATA : 8;
        uint32_t               : 24;
    };
    uint32_t val;
} i2c_fifo_data_t;

// I2C_INT_RAW_REG, I2C_INT_ENA_REG, I2C_INT_STATUS_REG
typedef union {
    struct {
        uint32_t I2C_RX_FIFO_FULL             : 1;
        uint32_t I2C_TX_FIFO_EMPTY            : 1;
        uint32_t I2C_RX_FIFO_OVF              : 1;
        uint32_t I2C_END_DETECT               : 1;
        uint32_t I2C_SLAVE_TRAN_COMP          : 1;
        uint32_t I2C_ARBITRATION_LOST         : 1;
        uint32_t I2C_MASTER_TRAN_COMP         : 1;
        uint32_t I2C_TRANS_COMPLETE           : 1;
        uint32_t I2C_TIME_OUT                 : 1;
        uint32_t I2C_TRANS_START              : 1;
        uint32_t I2C_ACK_ERR                  : 1;
        uint32_t I2C_RX_REC_FULL              : 1;
        uint32_t I2C_TX_SEND_EMPTY            : 1;
        uint32_t I2C_SCL_ST_TO                : 1;
        uint32_t I2C_SCL_MAIN_ST_TO           : 1;
        uint32_t I2C_DET_START                : 1;
        uint32_t                              : 16;
    };
    uint32_t val;
} i2c_int_reg_t;

// I2C_COMDx_REG
enum i2c_command_op_code_t
{
    I2C_COMMAND_OP_CODE_RSTART = 0,
    I2C_COMMAND_OP_CODE_WRITE  = 1,
    I2C_COMMAND_OP_CODE_READ   = 2,
    I2C_COMMAND_OP_CODE_STOP   = 3,
    I2C_COMMAND_OP_CODE_END    = 4,
};

typedef union {
    struct {
        uint32_t                   I2C_COMMAND_BYTE_NUM     : 8;
        uint32_t                   I2C_COMMAND_ACK_CHECK_EN : 1;
        uint32_t                   I2C_COMMAND_ACK_EXP      : 1;
        uint32_t                   I2C_COMMAND_ACK_VALUE    : 1;
        enum i2c_command_op_code_t I2C_COMMAND_OP_CODE      : 3;
        uint32_t                                            : 17;
        uint32_t                   I2C_COMMAND_DONE         : 1;
    };
    uint32_t val;
} i2c_comd_reg_t;

#define I2C_COMD_REG_COUNT 16

typedef struct Esp32I2CState {
    SysBusDevice parent_obj;

    MemoryRegion iomem;
    qemu_irq irq;
    I2CBus *bus;

    bitbang_i2c_interface bitbang;

    i2c_ctr_reg_t i2c_ctr_reg;
    i2c_sr_reg_t i2c_sr_reg;
    i2c_fifo_conf_reg_t i2c_fifo_conf_reg;
    i2c_int_reg_t i2c_int_raw_reg;
    i2c_int_reg_t i2c_int_ena_reg;
    i2c_int_reg_t i2c_int_status_reg;
    i2c_comd_reg_t i2c_comd_reg[I2C_COMD_REG_COUNT];

    Fifo8 rx_fifo;
    Fifo8 tx_fifo;
} Esp32I2CState;

#endif /* ESP32_I2C_H */
