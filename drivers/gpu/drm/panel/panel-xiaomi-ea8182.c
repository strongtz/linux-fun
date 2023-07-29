// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2023 Xilin Wu <wuxilin123@gmail.com>
 *
 * Based on panel-visionox-r66451
 */

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_probe_helper.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>
#include <drm/display/drm_dsc.h>
#include <drm/display/drm_dsc_helper.h>

#include <video/mipi_display.h>

struct xiaomi_ea8182 {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	struct regulator_bulk_data supplies[2];
	bool prepared, enabled;
};

static inline struct xiaomi_ea8182 *to_xiaomi_ea8182(struct drm_panel *panel)
{
	return container_of(panel, struct xiaomi_ea8182, panel);
}

static void xiaomi_ea8182_reset(struct xiaomi_ea8182 *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(10000, 10100);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 10100);
}

static int xiaomi_ea8182_on(struct xiaomi_ea8182 *ctx)
{
	struct mipi_dsi_device *dsi = ctx->dsi;
    struct device *dev = &dsi->dev;
    int ret;

	// dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	mipi_dsi_dcs_write_seq(dsi, 0x9d, 0x01);
    // DSC PPS
	// mipi_dsi_dcs_write_seq(dsi, 0x9e,
	// 		  0x11, 0x00, 0x00, 0xa9, 0x30, 0x80, 0x0c, 0x80, 0x05,
	// 		  0xa0, 0x00, 0x32, 0x02, 0xd0, 0x02, 0xd0, 0x02, 0x00,
	// 		  0x02, 0x68, 0x00, 0x20, 0x05, 0x8c, 0x00, 0x0a, 0x00,
	// 		  0x0c, 0x01, 0xf6, 0x01, 0x87, 0x18, 0x00, 0x10, 0xf0,
	// 		  0x03, 0x0c, 0x20, 0x00, 0x06, 0x0b, 0x0b, 0x33, 0x0e,
	// 		  0x1c, 0x2a, 0x38, 0x46, 0x54, 0x62, 0x69, 0x70, 0x77,
	// 		  0x79, 0x7b, 0x7d, 0x7e, 0x01, 0x02, 0x01, 0x00, 0x09,
	// 		  0x40, 0x09, 0xbe, 0x19, 0xfc, 0x19, 0xfa, 0x19, 0xf8,
	// 		  0x1a, 0x38, 0x1a, 0x78, 0x1a, 0xb6, 0x2a, 0xf6, 0x2b,
	// 		  0x34, 0x2b, 0x74, 0x3b, 0x74, 0x6b, 0xf4, 0x00, 0x00,
	// 		  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 		  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 		  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 		  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 		  0x00, 0x00);

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	usleep_range(10000, 11000);

	ret = mipi_dsi_dcs_set_tear_on(dsi, MIPI_DSI_DCS_TEAR_MODE_VBLANK);
	if (ret < 0) {
		dev_err(dev, "Failed to set tear on: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_column_address(dsi, 0x0000, 0x059f);
	if (ret < 0) {
		dev_err(dev, "Failed to set column address: %d\n", ret);
		return ret;
	}

	ret = mipi_dsi_dcs_set_page_address(dsi, 0x0000, 0x0c7f);
	if (ret < 0) {
		dev_err(dev, "Failed to set page address: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0xb4, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0xf7, 0x07);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x02);
	mipi_dsi_dcs_write_seq(dsi, 0xf4, 0x00, 0xc0, 0x7f, 0x3d);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x09);
	mipi_dsi_dcs_write_seq(dsi, 0xf4, 0x07);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x11);
	mipi_dsi_dcs_write_seq(dsi, 0xf4, 0x19);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x04);
	mipi_dsi_dcs_write_seq(dsi, 0xee, 0x80);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0xc0);
	mipi_dsi_dcs_write_seq(dsi, 0xd6, 0x40);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb3, 0x00, 0x00, 0xd8, 0xd8, 0xff, 0xff, 0x40);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xe7, 0x81);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x09);
	mipi_dsi_dcs_write_seq(dsi, 0xe7, 0x03);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x0f);
	mipi_dsi_dcs_write_seq(dsi, 0xe1, 0x10);
	mipi_dsi_dcs_write_seq(dsi, 0xcf, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x76);
	mipi_dsi_dcs_write_seq(dsi, 0xd1, 0x01);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x22);
	mipi_dsi_dcs_write_seq(dsi, 0xe2, 0x01, 0x02, 0x25);
	mipi_dsi_dcs_write_seq(dsi, 0xf7, 0x07);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
	msleep(90);
    /* Dimming Setting*/
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
    /* Global para */
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x0d);
    /* Dimming Speed Setting : 0x20 : 32Frames*/
	mipi_dsi_dcs_write_seq(dsi, 0xb4, 0x20);
    /* Global para */
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x0c);
    /* 0x20 : ELVSS DIM OFF */
	mipi_dsi_dcs_write_seq(dsi, 0xb4, 0x20);
    /* 0x20 Normal transition(60Hz) */
	mipi_dsi_dcs_write_seq(dsi, MIPI_DCS_WRITE_CONTROL_DISPLAY, 0x20);
	ret = mipi_dsi_dcs_set_display_brightness(dsi, 0x0000);
	if (ret < 0) {
		dev_err(dev, "Failed to set display brightness: %d\n", ret);
		return ret;
	}

	mipi_dsi_dcs_write_seq(dsi, 0xf7, 0x07);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);
    /* OFC (MIPI 1360Mbps/Lane, OSC 165Mhz)*/
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x07);
	mipi_dsi_dcs_write_seq(dsi, 0xeb, 0x03, 0x88, 0xf8);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);

	// ret = mipi_dsi_dcs_set_display_on(dsi);
	// if (ret < 0) {
	// 	dev_err(dev, "Failed to set display on: %d\n", ret);
	// 	return ret;
	// }

	return 0;
}

static int xiaomi_ea8182_off(struct xiaomi_ea8182 *ctx)
{
	ctx->dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
	return 0;
}

static int xiaomi_ea8182_prepare(struct drm_panel *panel)
{
	struct xiaomi_ea8182 *ctx = to_xiaomi_ea8182(panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	if (ctx->prepared)
		return 0;

	ret = regulator_bulk_enable(ARRAY_SIZE(ctx->supplies),
				    ctx->supplies);
	if (ret < 0)
		return ret;

	xiaomi_ea8182_reset(ctx);

	ret = xiaomi_ea8182_on(ctx);
	if (ret < 0) {
		dev_err(dev, "Failed to initialize panel: %d\n", ret);
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);
		return ret;
	}

	mipi_dsi_compression_mode(ctx->dsi, true);

	// dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;

	ctx->prepared = true;
	return 0;
}

static int xiaomi_ea8182_unprepare(struct drm_panel *panel)
{
	struct xiaomi_ea8182 *ctx = to_xiaomi_ea8182(panel);
	struct device *dev = &ctx->dsi->dev;
	int ret;

	if (!ctx->prepared)
		return 0;

	ret = xiaomi_ea8182_off(ctx);
	if (ret < 0)
		dev_err(dev, "Failed to un-initialize panel: %d\n", ret);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	regulator_bulk_disable(ARRAY_SIZE(ctx->supplies), ctx->supplies);

	ctx->prepared = false;
	return 0;
}

static const struct drm_display_mode xiaomi_ea8182_60hz_mode = {
	.clock = (1440 + 64 + 120 + 64) * (3200 + 632 + 680 + 680) * 60 / 1000, // 604569600 / 1000
	.hdisplay = 1440,
	.hsync_start = 1440 + 64,
	.hsync_end = 1440 + 64 + 120,
	.htotal = 1440 + 64 + 120 + 64, // 1536
	.vdisplay = 3200,
	.vsync_start = 3200 + 632,
	.vsync_end = 3200 + 632 + 680,
	.vtotal = 3200 + 632 + 680 + 680, // 3280
	.width_mm = 0,
	.height_mm = 0,
	.type = DRM_MODE_TYPE_DRIVER,
};

static const struct drm_display_mode xiaomi_ea8182_90hz_mode = {
	.clock = (1440 + 64 + 80 + 64) * (3200 + 160 + 160 + 160) * 90 / 1000, // 604569600 / 1000
	.hdisplay = 1440,
	.hsync_start = 1440 + 64,
	.hsync_end = 1440 + 64 + 80,
	.htotal = 1440 + 64 + 80 + 64, // 1536
	.vdisplay = 3200,
	.vsync_start = 3200 + 160,
	.vsync_end = 3200 + 160 + 160,
	.vtotal = 3200 + 160 + 160 + 160, // 3280
	.width_mm = 0,
	.height_mm = 0,
	.type = DRM_MODE_TYPE_DRIVER,
};

static const struct drm_display_mode xiaomi_ea8182_120hz_mode = {
	.clock = (1440 + 32 + 32 + 32) * (3200 + 24 + 24 + 32) * 120 / 1000, // 604569600 / 1000
	.hdisplay = 1440,
	.hsync_start = 1440 + 32,
	.hsync_end = 1440 + 32 + 32,
	.htotal = 1440 + 32 + 32 + 32, // 1536
	.vdisplay = 3200,
	.vsync_start = 3200 + 24,
	.vsync_end = 3200 + 24 + 24,
	.vtotal = 3200 + 24 + 24 + 32, // 3280
	.width_mm = 0,
	.height_mm = 0,
	.type = DRM_MODE_TYPE_DRIVER,
};

static int xiaomi_ea8182_enable(struct drm_panel *panel)
{
	struct xiaomi_ea8182 *ctx = to_xiaomi_ea8182(panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct drm_dsc_picture_parameter_set pps;
	int ret;

	if (ctx->enabled)
		return 0;

	if (!dsi->dsc) {
		dev_err(&dsi->dev, "DSC not attached to DSI\n");
		return -ENODEV;
	}

	// dsi->mode_flags |= MIPI_DSI_MODE_LPM;

	drm_dsc_pps_payload_pack(&pps, dsi->dsc);
	ret = mipi_dsi_picture_parameter_set(dsi, &pps);
	if (ret) {
		dev_err(&dsi->dev, "Failed to set PPS\n");
		return ret;
	}

	ret = mipi_dsi_dcs_exit_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(&dsi->dev, "Failed to exit sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	ret = mipi_dsi_dcs_set_display_on(dsi);
	if (ret < 0) {
		dev_err(&dsi->dev, "Failed on set display on: %d\n", ret);
		return ret;
	}

	usleep_range(17000, 18000);

	/* 120hz Frequency Select*/
	mipi_dsi_dcs_write_seq(dsi, 0x60, 0x08);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0x5a, 0x5a);
	mipi_dsi_dcs_write_seq(dsi, 0xb0, 0x55);
	mipi_dsi_dcs_write_seq(dsi, 0xd1, 0x0e);
	mipi_dsi_dcs_write_seq(dsi, 0xf7, 0x07);
	mipi_dsi_dcs_write_seq(dsi, 0xfc, 0xa5, 0xa5);
	mipi_dsi_dcs_write_seq(dsi, 0xf0, 0xa5, 0xa5);

	msleep(20);
	// dsi->mode_flags &= ~MIPI_DSI_MODE_LPM;
    // Check if this is required

	ctx->enabled = true;

	return 0;
}

static int xiaomi_ea8182_disable(struct drm_panel *panel)
{
	struct xiaomi_ea8182 *ctx = to_xiaomi_ea8182(panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct device *dev = &dsi->dev;
	int ret;

	ctx->enabled = false;

	ret = mipi_dsi_dcs_set_display_off(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to set display off: %d\n", ret);
		return ret;
	}
	msleep(20);

	ret = mipi_dsi_dcs_enter_sleep_mode(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to enter sleep mode: %d\n", ret);
		return ret;
	}
	msleep(120);

	return 0;
}

static int xiaomi_ea8182_get_modes(struct drm_panel *panel,
				    struct drm_connector *connector)
{
	drm_connector_helper_get_modes_fixed(connector, &xiaomi_ea8182_90hz_mode);
	return 1;
}

static const struct drm_panel_funcs xiaomi_ea8182_funcs = {
	.prepare = xiaomi_ea8182_prepare,
	.unprepare = xiaomi_ea8182_unprepare,
	.get_modes = xiaomi_ea8182_get_modes,
	.enable = xiaomi_ea8182_enable,
	.disable = xiaomi_ea8182_disable,
};

static int xiaomi_ea8182_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);

	return mipi_dsi_dcs_set_display_brightness(dsi, brightness);
}

static const struct backlight_ops xiaomi_ea8182_bl_ops = {
	.update_status = xiaomi_ea8182_bl_update_status,
};

static struct backlight_device *
xiaomi_ea8182_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 255,
		.max_brightness = 4095,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &xiaomi_ea8182_bl_ops, &props);
}

static int xiaomi_ea8182_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct xiaomi_ea8182 *ctx;
	struct drm_dsc_config *dsc;
	int ret = 0;

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	dsc = devm_kzalloc(dev, sizeof(*dsc), GFP_KERNEL);
	if (!dsc)
		return -ENOMEM;

	/* Set DSC params */
	dsc->dsc_version_major = 0x1;
	dsc->dsc_version_minor = 0x1;

	dsc->slice_height = 50;
	dsc->slice_width = 720;
	dsc->slice_count = 2;
    // Check this!
	dsc->bits_per_component = 8;
	dsc->bits_per_pixel = 8 << 4;
	dsc->block_pred_enable = true;

	dsi->dsc = dsc;

	ctx->supplies[0].supply = "vddio";
	ctx->supplies[1].supply = "vdd";

	ret = devm_regulator_bulk_get(&dsi->dev, ARRAY_SIZE(ctx->supplies),
			ctx->supplies);

	if (ret < 0)
		return ret;

    // Check this
	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio), "Failed to get reset-gpios\n");

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_MODE_LPM | MIPI_DSI_CLOCK_NON_CONTINUOUS;
    ctx->panel.prepare_prev_first = true;

	drm_panel_init(&ctx->panel, dev, &xiaomi_ea8182_funcs, DRM_MODE_CONNECTOR_DSI);
	ctx->panel.backlight = xiaomi_ea8182_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				"Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
	}

	return ret;
}

static void xiaomi_ea8182_remove(struct mipi_dsi_device *dsi)
{
	struct xiaomi_ea8182 *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id xiaomi_ea8182_of_match[] = {
	{.compatible = "xiaomi,ea8182"},
	{ /*sentinel*/ }
};
MODULE_DEVICE_TABLE(of, xiaomi_ea8182_of_match);

static struct mipi_dsi_driver xiaomi_ea8182_driver = {
	.probe = xiaomi_ea8182_probe,
	.remove = xiaomi_ea8182_remove,
	.driver = {
		.name = "panel-xiaomi-ea8182",
		.of_match_table = xiaomi_ea8182_of_match,
	},
};

module_mipi_dsi_driver(xiaomi_ea8182_driver);

MODULE_AUTHOR("Xilin Wu <wuxilin123@gmail.com>");
MODULE_DESCRIPTION("Panel driver for the xiaomi ea8182 AMOLED DSI panel");
MODULE_LICENSE("GPL");
