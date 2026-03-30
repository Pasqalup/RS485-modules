all : flash

TARGET:=slave
# CFLAGS += -lm
#define FUNCONF_SYSTICK_USE_HCLK 1
TARGET_MCU?=CH32V003
include ../../ch32fun/ch32fun.mk

flash : cv_flash
clean : cv_clean

