/**
 * Copyright (C) 2021 Longcheer Inc. All rights reserved.
 * 2021-09-11 File created.
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#ifdef CONFIG_OF
#include <linux/of.h>
#include <linux/of_gpio.h>
#endif
#include <sound/core.h>
#include <sound/soc.h>
#include <asoc/msm-cdc-pinctrl.h>

#define LCT_PINCTRL_DRV_VERSION  "V1.0.0"

struct lct_pinctrl_dev
{
	struct device_node *reset_gpio_p;
};
typedef struct lct_pinctrl_dev lct_pinctrl_dev_t;

static DEFINE_MUTEX(lct_pinctrl_mutex);
static lct_pinctrl_dev_t *g_lct_pinctrl_dev = NULL;

static int lct_pinctrl_pinctrl_init(struct platform_device *pdev, lct_pinctrl_dev_t *lct_pinctrl_dev)
{
	if (!lct_pinctrl_dev)
		return -EINVAL;

	lct_pinctrl_dev->reset_gpio_p = of_parse_phandle(pdev->dev.of_node,"audio-reset-pin", 0);
	if (!lct_pinctrl_dev->reset_gpio_p) {
		dev_dbg(&pdev->dev, "%s: property %s not detected in node %s\n",
			__func__, "audio-reset-pin",
			pdev->dev.of_node->full_name);
	}

	return 0;
}

static int lct_pinctrl_ctrl_pin(lct_pinctrl_dev_t *lct_pinctrl_dev, int on)
{
	int ret;
	if (lct_pinctrl_dev == NULL)
		return -EINVAL;

	on = !!on;
	if (lct_pinctrl_dev->reset_gpio_p) {
		if (on)
			msm_cdc_pinctrl_select_active_state(
					lct_pinctrl_dev->reset_gpio_p);
		else
			msm_cdc_pinctrl_select_sleep_state(
					lct_pinctrl_dev->reset_gpio_p);
		pr_debug("%s: on=%d \n", __func__, on);
		ret = 0;
	}
    return 0;
}

int lct_pinctrl_rst_pin_set(int enable)
{
	int ret;

	pr_debug("%s external speaker PA\n", enable ? "Enable" : "Disable");

	mutex_lock(&lct_pinctrl_mutex);
	if (enable) {
		ret = lct_pinctrl_ctrl_pin(g_lct_pinctrl_dev, enable);
	} else {
		ret = lct_pinctrl_ctrl_pin(g_lct_pinctrl_dev, enable);
	}
	mutex_unlock(&lct_pinctrl_mutex);

	return ret;
}
EXPORT_SYMBOL_GPL(lct_pinctrl_rst_pin_set);

static int lct_pinctrl_dev_probe(struct platform_device *pdev)
{
	lct_pinctrl_dev_t *lct_pinctrl_dev;
	int ret = 0;

	pr_debug("lct_pinctrl version: %s\n", LCT_PINCTRL_DRV_VERSION);
	lct_pinctrl_dev = devm_kzalloc(&pdev->dev, sizeof(lct_pinctrl_dev_t), GFP_KERNEL);
	if (lct_pinctrl_dev == NULL) {
		pr_err("allocate memery failed\n");
		return -ENOMEM;
	}

	ret |= lct_pinctrl_pinctrl_init(pdev, lct_pinctrl_dev);
	if (ret) {
        pr_err("init fail\n");
		return ret;
    }

	platform_set_drvdata(pdev, lct_pinctrl_dev);
	g_lct_pinctrl_dev = lct_pinctrl_dev;

	return ret;
}

static int lct_pinctrl_dev_remove(struct platform_device *pdev)
{
	lct_pinctrl_dev_t *lct_pinctrl_dev = platform_get_drvdata(pdev);

	if (lct_pinctrl_dev) {
		devm_kfree(&pdev->dev, lct_pinctrl_dev);
	}

	return 0;
}

#ifdef CONFIG_OF
static const struct of_device_id lct_pinctrl_of_match[] =
{
	{.compatible = "longcheer,lct_pinctrl"},
	{},
};
MODULE_DEVICE_TABLE(of, lct_pinctrl_of_match);
#endif

static struct platform_driver lct_pinctrl_dev_driver =
{
	.probe = lct_pinctrl_dev_probe,
	.remove = lct_pinctrl_dev_remove,
	.driver = {
		.name = "lct_pinctrl",
#ifdef CONFIG_OF
		.of_match_table = lct_pinctrl_of_match,
#endif
	}
};

#if !defined(module_platform_driver)
static int __init lct_pinctrl_dev_init(void)
{
	int ret;

	ret = platform_driver_register(&lct_pinctrl_dev_driver);
	if (ret)
		pr_err("register driver failed, ret: %d\n", ret);

	return ret;
}

static void __exit lct_pinctrl_dev_exit(void)
{
	platform_driver_unregister(&lct_pinctrl_dev_driver);
}

module_init(lct_pinctrl_dev_init);
module_exit(lct_pinctrl_dev_exit);
#else
module_platform_driver(lct_pinctrl_dev_driver);
#endif

MODULE_AUTHOR("LCT SW <support@longcheer.com>");
MODULE_DESCRIPTION("Platfrom Controller driver");
MODULE_VERSION(LCT_PINCTRL_DRV_VERSION);
MODULE_LICENSE("GPL");

