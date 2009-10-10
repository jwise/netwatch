/* reg-82801b.h
 * Register macros for Intel 82801B
 * NetWatch system management mode administration console
 *
 * Copyright (c) 2008 Jacob Potter and Joshua Wise.  All rights reserved.
 * This program is free software; you can redistribute and/or modify it under
 * the terms found in the file LICENSE in the root of this source tree.
 *
 */

#ifndef _REG_82801B_H
#define _REG_82801B_H

#define ICH2_PCI_BRIDGE_BUS		0
#define ICH2_PCI_BRIDGE_DEV		30
#define ICH2_PCI_BRIDGE_FN		0

#define ICH2_NIC_BUS			1
#define ICH2_NIC_DEV			8
#define ICH2_NIC_FN			0

#define ICH2_LPC_BUS			0
#define ICH2_LPC_DEV			31
#define ICH2_LPC_FN			0

#define ICH2_LPC_PCI_PMBASE		0x40
#define ICH2_PMBASE_MASK		0xFF80
#define ICH2_LPC_PCI_ACPI_CTRL		0x44
#define ICH2_LPC_PCI_GPIOBASE		0x58
#define ICH2_LPC_PCI_GPIO_CNTL		0x5C
#define ICH2_LPC_PCI_GEN_PMCON1		0xA0
#define ICH2_LPC_PCI_GEN_PMCON2		0xA2
#define ICH2_LPC_PCI_GEN_PMCON3		0xA4
#define ICH2_LPC_PCI_GPI_ROUT		0xB8
#define ICH2_LPC_PCI_TRP_FWD_EN		0xC0
#define ICH2_LPC_PCI_MON4_TRP_RNG	0xC4
#define ICH2_LPC_PCI_MON5_TRP_RNG	0xC6
#define ICH2_LPC_PCI_MON6_TRP_RNG	0xC8
#define ICH2_LPC_PCI_MON7_TRP_RNG	0xCA
#define ICH2_LPC_PCI_MON_TRP_MSK	0xCC

#define ICH2_PMBASE_PM1_STS		0x00
#define ICH2_PM1_STS_WAK_STS		(1 << 15)
#define ICH2_PM1_STS_PRBTNOR_STS	(1 << 11)
#define ICH2_PM1_STS_RTC_STS		(1 << 10)
#define ICH2_PM1_STS_PWRBTN_STS		(1 << 8)
#define ICH2_PM1_STS_GBL_STS		(1 << 5)
#define ICH2_PM1_STS_BM_STS		(1 << 4)
#define ICH2_PM1_STS_TMROF_STS		(1 << 0)

#define ICH2_PMBASE_PM1_EN		0x02
#define ICH2_PM1_EN_RTC_EN		(1 << 10)
#define ICH2_PM1_EN_PWRBTN_EN		(1 << 8)
#define ICH2_PM1_EN_GBL_EN		(1 << 5)
#define ICH2_PM1_EN_TMROF_EN		(1 << 0

#define ICH2_PMBASE_PM1_TMR		0x08
#define ICH2_PM1_TMR_FREQ		3579545		/* This will be the encryption key for a question on the test. */

#define ICH2_PMBASE_SMI_EN		0x30
#define ICH2_SMI_EN_PERIODIC_EN		(1 << 14)
#define ICH2_SMI_EN_TCO_EN		(1 << 13)
#define ICH2_SMI_EN_MCSMI_EN		(1 << 11)
#define ICH2_SMI_EN_BIOS_RLS		(1 << 7)
#define ICH2_SMI_EN_SWSMI_TMR_EN	(1 << 6)
#define ICH2_SMI_EN_APMC_EN		(1 << 5)
#define ICH2_SMI_EN_SLP_SMI_EN		(1 << 4)
#define ICH2_SMI_EN_LEGACY_USB_EN	(1 << 3)
#define ICH2_SMI_EN_BIOS_EN		(1 << 2)
#define ICH2_SMI_EN_EOS			(1 << 1)
#define ICH2_SMI_EN_GBL_SMI_EN		(1 << 0)

#define ICH2_PMBASE_SMI_STS		0x34
#define ICH2_SMI_STS_SMBUS_SMI_STS	(1 << 16)
#define ICH2_SMI_STS_SERIRQ_SMI_STS	(1 << 15)
#define ICH2_SMI_STS_PERIODIC_STS	(1 << 14)
#define ICH2_SMI_STS_TCO_STS		(1 << 13)
#define ICH2_SMI_STS_DEVMON_STS		(1 << 12)
#define ICH2_SMI_STS_MCSMI_STS		(1 << 11)
#define ICH2_SMI_STS_GPE1_STS		(1 << 10)
#define ICH2_SMI_STS_GPE0_STS		(1 << 9)
#define ICH2_SMI_STS_PM1_STS_REG	(1 << 8)
#define ICH2_SMI_STS_SWSMI_TMR_STS	(1 << 6)
#define ICH2_SMI_STS_APM_STS		(1 << 5)
#define ICH2_SMI_STS_SLP_SMI_STS	(1 << 4)
#define ICH2_SMI_STS_LEGACY_USB_STS	(1 << 3)
#define ICH2_SMI_STS_BIOS_STS		(1 << 2)

#define ICH2_PMBASE_MON_SMI		0x40
#define ICH2_MON_SMI_DEV7_TRAP_STS	(1 << 15)
#define ICH2_MON_SMI_DEV6_TRAP_STS	(1 << 14)
#define ICH2_MON_SMI_DEV5_TRAP_STS	(1 << 13)
#define ICH2_MON_SMI_DEV4_TRAP_STS	(1 << 12)
#define ICH2_MON_SMI_DEV7_TRAP_EN	(1 << 11)
#define ICH2_MON_SMI_DEV6_TRAP_EN	(1 << 10)
#define ICH2_MON_SMI_DEV5_TRAP_EN	(1 << 9)
#define ICH2_MON_SMI_DEV4_TRAP_EN	(1 << 8)

#define ICH2_PMBASE_DEVACT_STS		0x44
#define ICH2_DEVACT_STS_ADLIB_ACT_STS	(1 << 13)
#define ICH2_DEVACT_STS_KBC_ACT_STS	(1 << 12)
#define ICH2_DEVACT_STS_MIDI_ACT_STS	(1 << 11)
#define ICH2_DEVACT_STS_AUDIO_ACT_STS	(1 << 10)
#define ICH2_DEVACT_STS_PIRQDH_ACT_STS	(1 << 9)
#define ICH2_DEVACT_STS_PIRQCG_ACT_STS	(1 << 8)
#define ICH2_DEVACT_STS_PIRQBF_ACT_STS	(1 << 7)
#define ICH2_DEVACT_STS_PIRQAE_ACT_STS	(1 << 6)
#define ICH2_DEVACT_STS_LEG_ACT_STS	(1 << 5)
#define ICH2_DEVACT_STS_IDES1_ACT_STS	(1 << 3)
#define ICH2_DEVACT_STS_IDES0_ACT_STS	(1 << 2)
#define ICH2_DEVACT_STS_IDEP1_ACT_STS	(1 << 1)
#define ICH2_DEVACT_STS_IDEP0_ACT_STS	(1 << 0)

#define ICH2_PMBASE_DEVTRAP_EN		0x48
#define ICH2_DEVTRAP_EN_ADLIB_TRP_EN	(1 << 13)
#define ICH2_DEVTRAP_EN_KBC_TRP_EN	(1 << 12)
#define ICH2_DEVTRAP_EN_MIDI_TRP_EN	(1 << 11)
#define ICH2_DEVTRAP_EN_AUDIO_TRP_EN	(1 << 10)
#define ICH2_DEVTRAP_EN_LEG_TRP_EN	(1 << 5)
#define ICH2_DEVTRAP_EN_IDES1_TRP_EN	(1 << 3)
#define ICH2_DEVTRAP_EN_IDES0_TRP_EN	(1 << 2)
#define ICH2_DEVTRAP_EN_IDEP1_TRP_EN	(1 << 1)
#define ICH2_DEVTRAP_EN_IDEP0_TRP_EN	(1 << 0)

#define ICH2_IDE_BUS			0
#define ICH2_IDE_DEV			31
#define ICH2_IDE_FN			1

#define ICH2_USB0_BUS			0
#define ICH2_USB0_DEV			31
#define ICH2_USB0_FN			2

#define ICH2_USB1_BUS			0
#define ICH2_USB1_DEV			31
#define ICH2_USB1_FN			4

#define ICH2_USB_LEGKEY			0xC0

#define ICH2_SMBUS_BUS			0
#define ICH2_SMBUS_DEV			31
#define ICH2_SMBUS_FN			3

#define ICH2_AC97AUD_BUS		0
#define ICH2_AC97AUD_DEV		31
#define ICH2_AC97AUD_FN			5

#define ICH2_AC97MOD_BUS		0
#define ICH2_AC97MOD_DEV		31
#define ICH2_AC97MOD_FN			6

#endif
