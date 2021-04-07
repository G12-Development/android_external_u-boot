/*
 * drivers/display/lcd/lcd_extern/lcd_extern.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the named License,
 * or any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _LCD_EXTERN_H_
#define _LCD_EXTERN_H_
#ifdef CONFIG_SYS_I2C_AML
#include <aml_i2c.h>
#else
#include <i2c.h>
#endif

#define EXTPR(fmt, args...)     printf("lcd extern: "fmt"", ## args)
#define EXTERR(fmt, args...)    printf("lcd extern: error: "fmt"", ## args)

#define LCD_EXTERN_DRIVER		"lcd_extern"

struct aml_lcd_extern_pinmux_s {
	unsigned reg;
	unsigned mux;
};

extern void udelay(unsigned long usec);
extern void mdelay(unsigned long msec);

extern unsigned char aml_lcd_extern_i2c_bus_get_sys(unsigned char i2c_bus);

#ifdef CONFIG_OF_LIBFDT
extern char *aml_lcd_extern_get_dts_prop(int nodeoffset, char *propname);
extern int aml_lcd_extern_get_dts_child(int index);
#endif

extern int aml_lcd_extern_get_gpio(unsigned char index);
extern int aml_lcd_extern_set_gpio(unsigned char index, int value);
extern void aml_lcd_extern_pinmux_set(int status);

extern int aml_lcd_extern_default_probe(struct aml_lcd_extern_driver_s *ext_drv);

#ifdef CONFIG_AML_LCD_EXTERN_I2C_T5800Q
extern int aml_lcd_extern_i2c_T5800Q_get_default_index(void);
extern int aml_lcd_extern_i2c_T5800Q_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_TC101
extern int aml_lcd_extern_i2c_tc101_get_default_index(void);
extern int aml_lcd_extern_i2c_tc101_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_ANX6345
extern int aml_lcd_extern_i2c_anx6345_get_default_index(void);
extern int aml_lcd_extern_i2c_anx6345_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_DLPC3439
extern int aml_lcd_extern_i2c_DLPC3439_get_default_index(void);
extern int aml_lcd_extern_i2c_DLPC3439_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_I2C_RT6947
extern int aml_lcd_extern_i2c_RT6947_get_default_index(void);
extern int aml_lcd_extern_i2c_RT6947_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif

#ifdef CONFIG_AML_LCD_EXTERN_SPI_LD070WS2
extern int aml_lcd_extern_spi_LD070WS2_get_default_index(void);
extern int aml_lcd_extern_spi_LD070WS2_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif

#ifdef CONFIG_AML_LCD_EXTERN_MIPI_N070ICN
extern int aml_lcd_extern_mipi_N070ICN_get_default_index(void);
extern int aml_lcd_extern_mipi_N070ICN_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_KD080D13
extern int aml_lcd_extern_mipi_KD080D13_get_default_index(void);
extern int aml_lcd_extern_mipi_KD080D13_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_TV070WSM
extern int aml_lcd_extern_mipi_TV070WSM_get_default_index(void);
extern int aml_lcd_extern_mipi_TV070WSM_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif

#ifdef CONFIG_AML_LCD_EXTERN_MIPI_ST7701
extern int aml_lcd_extern_mipi_st7701_get_default_index(void);
extern int aml_lcd_extern_mipi_st7701_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_P070ACB
extern int aml_lcd_extern_mipi_p070acb_get_default_index(void);
extern int aml_lcd_extern_mipi_p070acb_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
#ifdef CONFIG_AML_LCD_EXTERN_MIPI_TL050FHV02CT
extern int aml_lcd_extern_mipi_tl050fhv02ct_get_default_index(void);
extern int aml_lcd_extern_mipi_tl050fhv02ct_probe(struct aml_lcd_extern_driver_s *ext_drv);
#endif
extern int aml_lcd_extern_mipi_default_probe(struct aml_lcd_extern_driver_s *ext_drv);

#endif

