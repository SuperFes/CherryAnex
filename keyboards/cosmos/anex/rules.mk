# Add VIK configuration here (e.g. VIK_PMW3360_RIGHT=yes to use a trackball)

VIK_ENABLE = yes

include $(KEYBOARD_PATH_1)/vik/rules.mk

OLED_ENABLE = yes
OLED_DRIVER = ssd1306
OLED_TRANSPORT = i2c
