/*
 * Copyright (C) 2022 XiaoMi, Inc.
 */
#ifndef __WL2866D_H
#define __WL2866D_H

enum {
	OUT_DVDD1,
	OUT_DVDD2,
	OUT_AVDD1,
	OUT_AVDD2,
	VOL_ENABLE,
	VOL_DISABLE,
	DISCHARGE_ENABLE,
	DISCHARGE_DISABLE,
};

/* wl2866 init module */
int cam_wl2866_init_module(void);

/* wl2866 exit module */
void cam_wl2866_exit_module(void);

int wl2866d_camera_power_control(unsigned int out_iotype, int is_power_on);
#endif /* __WL2866D_H */
