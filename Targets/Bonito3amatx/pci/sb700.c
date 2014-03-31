#include "sb700.h"
#include "rs780_cmn.h"

#define NULL (void*)0

void set_sm_enable_bits(device_t sm_dev, u32 reg_pos, u32 mask, u32 val)
{
	u32 reg_old, reg;
	reg = reg_old = pci_read_config32(sm_dev, reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pci_write_config32(sm_dev, reg_pos, reg);
	}
}

static void pmio_write_index(u32 port_base, u8 reg, u8 value)
{
	OUTB(reg, port_base);
	OUTB(value, port_base + 1);
}

static u8 pmio_read_index(u32 port_base, u8 reg)
{
	OUTB(reg, port_base);
	return INB(port_base + 1);
}

void pm_iowrite(u8 reg, u8 value)
{
	printk_info("pm_iowrite\n");
	pmio_write_index(PM_INDEX, reg, value);
}

u8 pm_ioread(u8 reg)
{
	printk_info("pm_ioread\n");
	return pmio_read_index(PM_INDEX, reg);
}

void pm2_iowrite(u8 reg, u8 value)
{
	printk_info("pm2_iowrite\n");
	pmio_write_index(PM2_INDEX, reg, value);
}

u8 pm2_ioread(u8 reg)
{
	printk_info("pm2_ioread\n");
	return pmio_read_index(PM2_INDEX, reg);
}

static void set_pmio_enable_bits(device_t sm_dev, u32 reg_pos,
				 u32 mask, u32 val)
{
	u8 reg_old, reg;
	reg = reg_old = pm_ioread(reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pm_iowrite(reg_pos, reg);
	}
}

void pmio_enable_bits(u32 reg_pos,
				 u32 mask, u32 val)
{
	u8 reg_old, reg;
	reg = reg_old = pm_ioread(reg_pos);
	reg &= ~mask;
	reg |= val;
	if (reg != reg_old) {
		pm_iowrite(reg_pos, reg);
	}
}

void sb700_sata(int enabled)
{
	device_t sm_dev;
	int index = 8;

	sm_dev = _pci_make_tag(0, 0x14, 0);
	set_sm_enable_bits(sm_dev, 0xac, 1 << index, (enabled ? 1 : 0) << index);
}


void sb700_usb(device_t usb_dev, int enabled, int index)
{
	device_t sm_dev;
	//int index;
	//int function;

	//_pci_break_tag(usb_dev, NULL, NULL, &function);
	//index = (function + 1) % 6;

	sm_dev = _pci_make_tag(0, 0x14, 0);
	set_sm_enable_bits(sm_dev, 0x68, 1 << index, (enabled ? 1 : 0) << index);
}

void sb700_hda(int enabled)
{
        device_t sm_dev;
        int index = 3;

        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_pmio_enable_bits(sm_dev, 0x59, 1 << index, (enabled ? 1 : 0) << index);
}


void sb700_lpc(int enabled)
{
        device_t sm_dev;
        int index = 20;

        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_sm_enable_bits(sm_dev, 0x64, 1 << index, (enabled ? 1 : 0) << index);
}

void sb700_aci(device_t dev, int enabled)
{
        device_t sm_dev;
        int index;
	int function;

	_pci_break_tag(dev, NULL, NULL, &function);
        index = function - 5;
        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_pmio_enable_bits(sm_dev, 0x59, 1 << index, (enabled ? 1 : 0) << index);
}


void sb700_mci(device_t dev, int enabled)
{
        device_t sm_dev;
        int index;
	int function;

	_pci_break_tag(dev, NULL, NULL, &function);
        index = function - 5;
        sm_dev = _pci_make_tag(0, 0x14, 0);
        set_pmio_enable_bits(sm_dev, 0x59, 1 << index, (enabled ? 1 : 0) << index);
}



void sb700_enable()
{
/*
*	0:11.0  SATA	bit 8 of sm_dev 0xac : 1 - enable, default         + 32 * 3
*	0:12.0  OHCI0-USB1	bit 0 of sm_dev 0x68
*	0:12.1  OHCI1-USB1	bit 1 of sm_dev 0x68
*	0:12.2  EHCI-USB1	bit 2 of sm_dev 0x68
*	0:13.0  OHCI0-USB2	bit 4 of sm_dev 0x68
*	0:13.1  OHCI1-USB2	bit 5 of sm_dev 0x68
*	0:13.2  EHCI-USB2	bit 6 of sm_dev 0x68
*	0:14.5  OHCI0-USB3	bit 7 of sm_dev 0x68
*	0:14.0  SMBUS							0
*	0:14.1  IDE							1
*	0:14.2  HDA	bit 3 of pm_io 0x59 : 1 - enable, default	    + 32 * 4
*	0:14.3  LPC	bit 20 of sm_dev 0x64 : 0 - disable, default  + 32 * 1
*	0:14.4  PCI							4
*/
#ifdef ENABLE_SATA
	printk_info("enable_sata\n");
	sb700_sata(1);
#endif

	printk_info("enable usb0\n");
	sb700_usb(_pci_make_tag(0, 0x12, 0), 1, 0);

	printk_info("enable usb1\n");
	sb700_usb(_pci_make_tag(0, 0x12, 1), 1, 1);
#if  1
	//printk_info("enable usb2\n");
	//sb700_usb(_pci_make_tag(0, 0x12, 2), 1, 2);
	printk_info("enable usb4\n");
	sb700_usb(_pci_make_tag(0, 0x13, 0), 1, 4);
	printk_info("enable usb5\n");
	sb700_usb(_pci_make_tag(0, 0x13, 1), 1, 5);
	//printk_info("enable usb6\n");
	//sb700_usb(_pci_make_tag(0, 0x13, 2), 1, 6);
	printk_info("enable usb7\n");
	sb700_usb(_pci_make_tag(0, 0x14, 5), 1, 7);
#endif
	//printk_info("enable hda\n");
	//sb700_hda(1);
	printk_info("enable lpc\n");
	sb700_lpc(1);
	//sb700_aci(_pci_make_tag(0, 0x14, 5), 1);
	//sb700_mci(_pci_make_tag(0, 0x14, 6), 1);
}

#define SB700_ACPI_IO_BASE ( 0x800)
#define SB700_ACPI_IO_SIZE ( 0x100)

#define ACPI_PM_EVT_BLK		(SB700_ACPI_IO_BASE + 0x00) /* 4 bytes */
#define ACPI_PM1_CNT_BLK		(SB700_ACPI_IO_BASE + 0x04) /* 2 bytes */
#define ACPI_PMA_CNT_BLK		(SB700_ACPI_IO_BASE + 0x0F) /* 1 byte */
#define ACPI_PM_TMR_BLK		(SB700_ACPI_IO_BASE + 0x18) /* 4 bytes */
#define ACPI_GPE0_BLK			(SB700_ACPI_IO_BASE + 0x10) /* 8 bytes */
#define ACPI_END				(SB700_ACPI_IO_BASE + 0x80)
int sb700_acpi_init(void)
{
	unsigned int temp32;
	int loop;
	unsigned int PM_IO_BASE;
	device_t acpi_tag;
	acpi_tag = _pci_make_tag(0, 20, 0);
	PM_IO_BASE = _pci_conf_read(acpi_tag, 0x9c);
	/* pm1 base */
	pm_iowrite(0x22, ACPI_PM1_CNT_BLK & 0xff);
	pm_iowrite(0x23, ACPI_PM1_CNT_BLK >> 8);

	/* gpm base */
	pm_iowrite(0x28, ACPI_GPE0_BLK & 0xFF);
	pm_iowrite(0x29, ACPI_GPE0_BLK >> 8);

	/* gpm base */
	pm_iowrite(0x2e, ACPI_END & 0xFF);
	pm_iowrite(0x2f, ACPI_END >> 8);

	/* io decode */
	pm_iowrite(0x0E, 1<<3 | 0<<2); /* AcpiDecodeEnable, When set, SB uses
									* the contents of the PM registers at
									* index 20-2B to decode ACPI I/O address.
									* AcpiSmiEn & SmiCmdEn
									*/
	/* SLP_SMI_EN */
	pmio_enable_bits(0x04,0x1<<7,0x00);
	/* SlpS3ToLdtPwrGdEn */
	pmio_enable_bits(0x41,0x1<<3,0x1<<3);
	/* LongSlpS3 */
	pmio_enable_bits(0x8d,0x1<<5,0x1<<5);

	/* SCI_EN set P225 */
	OUTW(1, ACPI_PM1_CNT_BLK);
	OUTW(5<<10,ACPI_PM1_CNT_BLK);
	OUTW(1<<13,ACPI_PM1_CNT_BLK);
}
