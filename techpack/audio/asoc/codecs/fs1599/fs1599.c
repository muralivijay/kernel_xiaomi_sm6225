/**
 * Copyright (C) Fourier Semiconductor Inc. 2016-2020. All rights reserved.
 * 2019-01-29 File created.
 */

#include "fsm_public.h"
#if defined(CONFIG_FSM_FS1599)

#define FS1599_STATUS         0xF000

#define FS1599_DEVID          0xF001
#define FS1599_REVID          0xF002

#define FS1599_ANASTAT        0xF003
#define FS1599_DIGSTAT        0xF004

#define FS1599_CHIPINI        0xF00E

#define FS1599_PWRCTRL        0xF010
#define FS1599_PWDN           0x0010
#define FS1599_I2CR           0x0110

#define FS1599_SYSCTRL        0xF011
#define FS1599_CPEN           0x0611
#define FS1599_AMPEN          0x0711


#define FS1599_GAINCTRL       0xF018

#define FS1599_ACCKEY         0xF01F

#define FS1599_LNMCTRL        0xF03F
#define FS1599_LNMMODE        0x0F3F

#define FS1599_ANACTRL        0xF0C0
#define FS1599_BSGCTRL        0xF0BC

#define FS1599_FW_NAME        "fs1599.fsm"


static int fs1599_i2c_reset(fsm_dev_t *fsm_dev)
{
	uint16_t val;
	int ret;
	int i;

	if (!fsm_dev) {
		return -EINVAL;
	}

	for (i = 0; i < FSM_I2C_RETRY; i++) {
		fsm_reg_write(fsm_dev, REG(FS1599_PWRCTRL), 0x0002); // reset nack
		fsm_reg_read(fsm_dev, REG(FS1599_PWRCTRL), NULL); // dummy read
		fsm_delay_ms(15); // 15ms
		ret = fsm_reg_write(fsm_dev, REG(FS1599_PWRCTRL), 0x0001);
		ret |= fsm_reg_read(fsm_dev, REG(FS1599_CHIPINI), &val);
		if ((val == 0x0003) || (val == 0x0300)) { // init finished
			break;
		}
	}
	if (i == FSM_I2C_RETRY) {
		pr_addr(err, "retry timeout");
		ret = -ETIMEDOUT;
	}

	return ret;
}

int fs1599_reg_init(fsm_dev_t *fsm_dev)
{
	fsm_config_t *cfg = fsm_get_config();
	int ret;

	if (fsm_dev == NULL || cfg == NULL) {
		return -EINVAL;
	}

	ret = fs1599_i2c_reset(fsm_dev);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_ACCKEY), 0x0091);
	ret |= fsm_write_reg_tbl(fsm_dev, FSM_SCENE_COMMON);
	ret |= fsm_write_reg_tbl(fsm_dev, cfg->next_scene);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_ACCKEY), 0x0000);
	if (ret) {
		pr_addr(err, "init fail:%d", ret);
		fsm_dev->state.dev_inited = false;
	} else {
		fsm_dev->cur_scene = cfg->next_scene;
	}

	return ret;
}

int fs1599_start_up(fsm_dev_t *fsm_dev)
{
	int ret;

	if (!fsm_dev) {
		return -EINVAL;
	}
	ret  = fsm_reg_write(fsm_dev, REG(FS1599_ACCKEY), 0xCA91);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_ANACTRL), 0x0010);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_PWRCTRL), 0x0000);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_SYSCTRL), 0x00C0);
	fsm_delay_ms(10);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_ANACTRL), 0x0000);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_ACCKEY), 0x0000);
	fsm_dev->amp_on = true;
	fsm_dev->errcode = ret;

	return ret;
}

int fs1599_set_mute(fsm_dev_t *fsm_dev, int mute)
{
	int ret = 0;

	if (!fsm_dev) {
		return -EINVAL;
	}
	if (mute) {
		ret = fsm_set_bf(fsm_dev, FS1599_LNMMODE, 0x0000);
		fsm_dev->cur_scene = 0;
	}

	return ret;
}

int fs1599_shut_down(fsm_dev_t *fsm_dev)
{
	int ret;

	if (fsm_dev == NULL) {
		return -EINVAL;
	}
	ret = fsm_reg_write(fsm_dev, REG(FS1599_SYSCTRL), 0x0000);
	fsm_delay_ms(20);
	ret |= fsm_reg_write(fsm_dev, REG(FS1599_PWRCTRL), 0x0001);
	fsm_dev->amp_on = false;

	return ret;
}

void fs1599_ops(fsm_dev_t *fsm_dev)
{
	fsm_config_t *cfg = fsm_get_config();

	if (!fsm_dev || !cfg) {
		return;
	}

	fsm_set_fw_name(FS1599_FW_NAME);
	fsm_dev->dev_ops.reg_init = fs1599_reg_init;
	fsm_dev->dev_ops.start_up = fs1599_start_up;
	fsm_dev->dev_ops.set_mute = fs1599_set_mute;
	fsm_dev->dev_ops.shut_down = fs1599_shut_down;
	cfg->nondsp_mode = true;
	cfg->store_otp = false;
}
#endif
