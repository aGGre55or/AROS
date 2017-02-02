/*
    Copyright � 1995-2017, The AROS Development Team. All rights reserved.
    $Id$
    
    http://download.intel.com/design/chipsets/datashts/29056601.pdf
*/

#include <aros/macros.h>
#include <aros/asmcall.h>
#include <proto/acpica.h>
#include <proto/exec.h>

#include <inttypes.h>
#include <string.h>

#include "kernel_base.h"
#include "kernel_debug.h"
#include "kernel_intern.h"

#include "acpi.h"
#include "apic.h"
#include "apic_ia32.h"
#include "ioapic.h"

#define D(x)
#define DINTR(x)

#define ACPI_MODPRIO_IOAPIC       50

/************************************************************************************************
                                    ACPI IO-APIC RELATED FUNCTIONS
 ************************************************************************************************/

const char *ACPI_TABLE_MADT_STR __attribute__((weak)) = "APIC";

#define IOREGSEL        0
#define IOREGWIN        0x10

/* descriptor for an ioapic routing table entry */
struct acpi_ioapic_route
{
    uint32_t    vect:8, dm:3, dstm:1, ds:1, pol:1, rirr:1, trig:1, mask:1, rsvd1:15;
    uint32_t    rsvd2:24, dst:8;
} __attribute__((packed));

static ULONG acpi_IOAPIC_ReadReg(APTR apic_base, UBYTE offset)
{
    *(ULONG volatile *)(apic_base + IOREGSEL) = offset;
    return *(ULONG volatile *)(apic_base + IOREGWIN);
}

static void acpi_IOAPIC_WriteReg(APTR apic_base, UBYTE offset, ULONG val)
{
    *(ULONG volatile *)(apic_base + IOREGSEL) = offset;
    *(ULONG volatile *)(apic_base + IOREGWIN) = val;
}

/* IO-APIC Interrupt Functions ... ***************************/

struct IOAPICInt_Private
{
    
};

void ioapic_ParseTableEntry(UQUAD *tblData)
{
    struct acpi_ioapic_route *tblEntry = (struct acpi_ioapic_route *)tblData;

    bug("%08x%08x", ((*tblData >> 32) & 0xFFFFFFFF), (*tblData & 0xFFFFFFFF));

    if (!tblEntry->mask)
    {
        bug(" Disabled.");
    }
    else
    {
        if (tblEntry->pol)
        {
            bug(" Active LOW,");
        }
        else
        {
            bug(" Active HIGH,");
        }
        
        if (tblEntry->trig)
        {
            bug(" LEVEL");
        }
        else
        {
            bug(" EDGE");
        }

        bug(" ->");

        if (tblEntry->dstm)
        {
            bug(" Logical %d:%d", ((tblEntry->dst >> 4) & 0xF), (tblEntry->dst & 0xF));
        }
        else
        {
            bug(" Physical %d", (tblEntry->dst & 0xF));
        }
    }
}

icid_t IOAPICInt_Register(struct KernelBase *KernelBase)
{
    ACPI_STATUS         status;
    ACPI_OBJECT         arg = { ACPI_TYPE_INTEGER };
    ACPI_OBJECT_LIST    arg_list = { 1, &arg };

    D(bug("[Kernel:IOAPIC] %s()\n", __func__));

#if (0)
    /* if we have been disabled, fail to register */
    if (IOAPICInt_IntrController.ic_Flags & ICF_DISABLED)
#endif
        return (icid_t)-1;

    /* Inform ACPI that we want to use IOAPIC mode... */
    arg.Integer.Value = 1;    // APIC IRQ model = IOAPIC
    status = AcpiEvaluateObject(NULL,
                                (char *)"\\_PIC",
                                &arg_list,
                                NULL);

    if (ACPI_FAILURE(status))
    {
        bug("[Kernel:IOAPIC] %s: Error evaluating _PIC: %s\n", __func__, AcpiFormatException(status));
        return (icid_t)-1;
    }

    D(bug("[Kernel:IOAPIC] %s: IOAPIC Enabled (status=%08x)\n", __func__, status));

    return (icid_t)IOAPICInt_IntrController.ic_Node.ln_Type;
}

BOOL IOAPICInt_Init(struct KernelBase *KernelBase, icid_t instanceCount)
{
    struct IOAPICCfgData *ioapicData;
    struct IOAPICData *ioapicPrivate = ((struct PlatformData *)KernelBase->kb_PlatformData)->kb_IOAPIC;
    int instance, irq = 0, ioapic_irqbase;

    D(bug("[Kernel:IOAPIC] %s(%d)\n", __func__, instanceCount));

    IOAPICInt_IntrController.ic_Private = ioapicPrivate;

    for (
            instance = 0;
            ((instance < instanceCount) && (ioapicData = &ioapicPrivate->ioapics[instance]));
            instance++
        )
    {
        D(
            bug("[Kernel:IOAPIC] %s: Init IOAPIC #%d [ID=%d] @ 0x%p\n", __func__, instance, ioapicData->ioapic_ID, ioapicData->ioapic_Base);
        )

        ioapic_irqbase = ioapicData->ioapic_GSI;

        D(bug("[Kernel:IOAPIC] %s: Configuring IRQs & Routing\n", __func__));

        if ((ioapicData->ioapic_RouteTable = AllocMem(ioapicData->ioapic_IRQCount * sizeof(UQUAD), MEMF_ANY)) != NULL)
        {
            D(bug("[Kernel:IOAPIC] %s: Routing Data @ 0x%p\n", __func__, ioapicData->ioapic_RouteTable));
            for (irq = ioapic_irqbase; (irq - ioapic_irqbase) < ioapicData->ioapic_IRQCount; irq++)
            {
                UBYTE ioapic_irq = irq - ioapic_irqbase;
                struct acpi_ioapic_route *irqRoute = (struct acpi_ioapic_route *)&ioapicData->ioapic_RouteTable[ioapic_irq];
                ULONG ioapicval;

                D(bug("[Kernel:IOAPIC] %s: route entry %d @ 0x%p\n", __func__, ioapic_irq, irqRoute));

                ioapicval = acpi_IOAPIC_ReadReg(
                    ioapicData->ioapic_Base,
                    IOAPICREG_REDTBLBASE + (ioapic_irq << 1));
                ioapicData->ioapic_RouteTable[ioapic_irq] = ((UQUAD)ioapicval << 32);
               
                ioapicval = acpi_IOAPIC_ReadReg(
                    ioapicData->ioapic_Base,
                    IOAPICREG_REDTBLBASE + (ioapic_irq << 1) + 1);
                ioapicData->ioapic_RouteTable[ioapic_irq] |= (UQUAD)ioapicval;

                /* mark the interrupts as active high, edge triggered
                 * and disabled by default */
                irqRoute->ds = 0;
                irqRoute->pol = 0;
                irqRoute->rirr = 0;
                irqRoute->trig = 0;
                /* setup delivery to apic #0 */
                irqRoute->vect = irq + HW_IRQ_BASE;
                irqRoute->dm = 0; // fixed
                irqRoute->dstm = 0; // physical
                irqRoute->mask = 0;
                irqRoute->dst = 0; // apic #0

                if (!krnInitInterrupt(KernelBase, irq, IOAPICInt_IntrController.ic_Node.ln_Type, instance))
                {
                    bug("[Kernel:IOAPIC] %s: failed to acquire IRQ #%d\n", __func__, irq);
                }
                D(
                    bug("[Kernel:IOAPIC]    %s:       ", __func__);
                    ioapic_ParseTableEntry((UQUAD *)&ioapicData->ioapic_RouteTable[ioapic_irq]);
                    bug("\n");
                );
                acpi_IOAPIC_WriteReg(ioapicData->ioapic_Base,
                    IOAPICREG_REDTBLBASE + (ioapic_irq << 1),
                    ((ioapicData->ioapic_RouteTable[ioapic_irq] >> 32) & 0xFFFFFFFF));
                acpi_IOAPIC_WriteReg(ioapicData->ioapic_Base,
                    IOAPICREG_REDTBLBASE + (ioapic_irq << 1 ) + 1,
                    (ioapicData->ioapic_RouteTable[ioapic_irq] & 0xFFFFFFFF));
            }
        }
    }

    return TRUE;
}

BOOL IOAPICInt_DisableIRQ(APTR icPrivate, icid_t icInstance, icid_t intNum)
{
    struct IOAPICData *ioapicPrivate = (struct IOAPICData *)icPrivate;
    struct IOAPICCfgData *ioapicData = &ioapicPrivate->ioapics[icInstance];
    UBYTE ioapic_irq = intNum - ioapicData->ioapic_GSI;
    struct acpi_ioapic_route *irqRoute = (struct acpi_ioapic_route *)&ioapicData->ioapic_RouteTable[ioapic_irq];

    D(bug("[Kernel:IOAPIC] %s()\n", __func__));

    irqRoute->mask = 0;

    acpi_IOAPIC_WriteReg(ioapicData->ioapic_Base,
        IOAPICREG_REDTBLBASE + (ioapic_irq << 1),
        ((ioapicData->ioapic_RouteTable[ioapic_irq] >> 32) & 0xFFFFFFFF));
    acpi_IOAPIC_WriteReg(ioapicData->ioapic_Base,
        IOAPICREG_REDTBLBASE + (ioapic_irq << 1 ) + 1,
        (ioapicData->ioapic_RouteTable[ioapic_irq] & 0xFFFFFFFF));

    return TRUE;
}

BOOL IOAPICInt_EnableIRQ(APTR icPrivate, icid_t icInstance, icid_t intNum)
{
    struct IOAPICData *ioapicPrivate = (struct IOAPICData *)icPrivate;
    struct IOAPICCfgData *ioapicData = &ioapicPrivate->ioapics[icInstance];
    UBYTE ioapic_irq = intNum - ioapicData->ioapic_GSI;
    struct acpi_ioapic_route *irqRoute = (struct acpi_ioapic_route *)&ioapicData->ioapic_RouteTable[ioapic_irq];
    IPTR _APICBase;
    apicid_t _APICID;

    D(bug("[Kernel:IOAPIC] %s()\n", __func__));

    _APICBase = core_APIC_GetBase();
    _APICID = core_APIC_GetID(_APICBase);

    irqRoute->ds = 0;
    irqRoute->pol = 0;
    irqRoute->rirr = 0;
    irqRoute->trig = 0;
    /* setup delivery to apic #0 */
    irqRoute->vect = intNum + HW_IRQ_BASE;
    irqRoute->dm = 0; // fixed
    irqRoute->dstm = 0; // physical
    irqRoute->mask = 1; // enable!!
    irqRoute->dst = _APICID;

    acpi_IOAPIC_WriteReg(ioapicData->ioapic_Base,
        IOAPICREG_REDTBLBASE + (ioapic_irq << 1),
        ((ioapicData->ioapic_RouteTable[ioapic_irq] >> 32) & 0xFFFFFFFF));
    acpi_IOAPIC_WriteReg(ioapicData->ioapic_Base,
        IOAPICREG_REDTBLBASE + (ioapic_irq << 1 ) + 1,
        (ioapicData->ioapic_RouteTable[ioapic_irq] & 0xFFFFFFFF));

    return TRUE;
}

BOOL IOAPICInt_AckIntr(APTR icPrivate, icid_t icInstance, icid_t intNum)
{
//    struct IOAPICData *ioapicPrivate = (struct IOAPICData *)icPrivate;

    D(bug("[Kernel:IOAPIC] %s()\n", __func__));

    return TRUE;
}

struct IntrController IOAPICInt_IntrController =
{
    {
        .ln_Name = "82093AA IO-APIC"
    },
    AROS_MAKE_ID('I','O','9','3'),
    0,
    NULL,
    IOAPICInt_Register,
    IOAPICInt_Init,
    IOAPICInt_EnableIRQ,
    IOAPICInt_DisableIRQ,
    IOAPICInt_AckIntr
};
 
 /********************************************************************/
 
 void acpi_IOAPIC_AllocPrivate(struct PlatformData *pdata)
{
    if (!pdata->kb_IOAPIC)
    {
        pdata->kb_IOAPIC = AllocMem(sizeof(struct IOAPICData) + pdata->kb_ACPI->acpi_ioapicCnt * sizeof(struct IOAPICCfgData), MEMF_CLEAR);
        D(bug("[Kernel:ACPI-IOAPIC] IO-APIC Private @ 0x%p, for %d IOAPIC's\n", pdata->kb_IOAPIC, pdata->kb_ACPI->acpi_ioapicCnt));
    }
}
 
/* Process the 'Interrupt Source' MADT Table */
AROS_UFH2(IPTR, ACPI_hook_Table_Int_Src_Parse,
	  AROS_UFHA(struct Hook *, table_hook, A0),
	  AROS_UFHA(ACPI_MADT_INTERRUPT_SOURCE *, intsrc, A2))
{
    AROS_USERFUNC_INIT

    DINTR(bug("[Kernel:ACPI-IOAPIC] ## %s()\n", __func__));
    DINTR(bug("[Kernel:ACPI-IOAPIC]    %s: %d:%d, GSI %d, Flags 0x%x\n", __func__, intsrc->Id, intsrc->Eid,
                intsrc->GlobalIrq, intsrc->IntiFlags));
    DINTR(
        if (intsrc->Type == 1)
        {
            bug("[Kernel:ACPI-IOAPIC]    %s: PMI, vector %d\n", __func__, intsrc->IoSapicVector);
        }
        else if(intsrc->Type == 2)
        {
            bug("[Kernel:ACPI-IOAPIC]    %s: INIT\n", __func__);
        }
        else if(intsrc->Type == 3)
        {
            bug("[Kernel:ACPI-IOAPIC]    %s: corrected\n", __func__);
        }
    )
    return TRUE;

    AROS_USERFUNC_EXIT
}

/* Process the 'Interrupt Source Overide' MADT Table */
AROS_UFH2(IPTR, ACPI_hook_Table_Int_Src_Ovr_Parse,
	  AROS_UFHA(struct Hook *, table_hook, A0),
	  AROS_UFHA(ACPI_MADT_INTERRUPT_OVERRIDE *, intsrc, A2))
{
    AROS_USERFUNC_INIT

    DINTR(bug("[Kernel:ACPI-IOAPIC] ## %s()\n", __func__));
    DINTR(bug("[Kernel:ACPI-IOAPIC]    %s: Bus %d, Source IRQ %d, GSI %d, Flags 0x%x\n", __func__, intsrc->Bus, intsrc->SourceIrq,
                intsrc->GlobalIrq, intsrc->IntiFlags));

    return TRUE;

    AROS_USERFUNC_EXIT
}

/* Process the 'Non-Maskable Interrupt Source' MADT Table */
AROS_UFH2(IPTR, ACPI_hook_Table_NMI_Src_Parse,
	  AROS_UFHA(struct Hook *, table_hook, A0),
	  AROS_UFHA(ACPI_MADT_NMI_SOURCE *, nmi_src, A2))
{
    AROS_USERFUNC_INIT

    DINTR(bug("[Kernel:ACPI-IOAPIC] ## %s()\n", __func__));
    DINTR(bug("[Kernel:ACPI-IOAPIC]    %s: GSI %d, Flags 0x%x\n", __func__, nmi_src->GlobalIrq, nmi_src->IntiFlags));

    /* FIXME: Uh... shouldn't we do something with this? */

    return TRUE;

    AROS_USERFUNC_EXIT
}

/* Process the 'IO-APIC' MADT Table */
AROS_UFH3(IPTR, ACPI_hook_Table_IOAPIC_Parse,
	  AROS_UFHA(struct Hook *, table_hook, A0),
	  AROS_UFHA(ACPI_MADT_IO_APIC *, ioapic, A2),
	  AROS_UFHA(struct ACPI_TABLESCAN_DATA *, tsdata, A1))
{
    AROS_USERFUNC_INIT

    struct PlatformData *pdata = tsdata->acpits_UserData;

    D(bug("[Kernel:ACPI-IOAPIC] ## %s()\n", __func__));

    if (!pdata->kb_IOAPIC)
    {
        acpi_IOAPIC_AllocPrivate(pdata);
    }

    if (pdata->kb_IOAPIC)
    {
        icintrid_t ioapicICInstID;
        ULONG ioapicval;
        int i;

        bug("[Kernel:ACPI-IOAPIC] Registering IO-APIC #%d [ID=%d] @ %p [GSI = %d]\n",
            pdata->kb_IOAPIC->ioapic_count, ioapic->Id, ioapic->Address, ioapic->GlobalIrqBase);

        if ((ioapicICInstID = krnAddInterruptController(KernelBase, &IOAPICInt_IntrController)) != (icintrid_t)-1)
        {
            struct IOAPICCfgData *ioapicData = (struct IOAPICCfgData *)&pdata->kb_IOAPIC->ioapics[pdata->kb_IOAPIC->ioapic_count];

            D(bug("[Kernel:ACPI-IOAPIC] IO-APIC IC ID #%d:%d\n", ICINTR_ICID(ioapicICInstID), ICINTR_INST(ioapicICInstID)));

            ioapicData->ioapic_Base = (APTR)((IPTR)ioapic->Address);
            ioapicData->ioapic_GSI = ioapic->GlobalIrqBase;

            ioapicval = acpi_IOAPIC_ReadReg(
                ioapicData->ioapic_Base,
                IOAPICREG_ID);
            ioapicData->ioapic_ID = ((ioapicval >> 24) & 0xF);

            D(bug("[Kernel:ACPI-IOAPIC]    %s:       #%d,",
                __func__, ioapicData->ioapic_ID));
            ioapicval = acpi_IOAPIC_ReadReg(
                ioapicData->ioapic_Base,
                IOAPICREG_VER);
            ioapicData->ioapic_IRQCount = ((ioapicval >> 16) & 0xFF) + 1;
            ioapicData->ioapic_Ver = (ioapicval & 0xFF);
            D(bug(" ver %d, max irqs = %d,",
                ioapicData->ioapic_Ver, ioapicData->ioapic_IRQCount));
            ioapicval = acpi_IOAPIC_ReadReg(
                ioapicData->ioapic_Base,
                IOAPICREG_ARB);
            D(bug("arb %d\n", ((ioapicval >> 24) & 0xF)));

            for (i = 0; i < (ioapicData->ioapic_IRQCount << 1); i += 2)
            {
                UQUAD tblraw = 0;

                ioapicval = acpi_IOAPIC_ReadReg(
                    ioapicData->ioapic_Base,
                    IOAPICREG_REDTBLBASE + i);
                tblraw = ((UQUAD)ioapicval << 32);
               
                ioapicval = acpi_IOAPIC_ReadReg(
                    ioapicData->ioapic_Base,
                    IOAPICREG_REDTBLBASE + i + 1);
                tblraw |= (UQUAD)ioapicval;

                D(
                    bug("[Kernel:ACPI-IOAPIC]    %s:       ", __func__);
                    ioapic_ParseTableEntry(&tblraw);
                    bug("\n");
                )
            }

            pdata->kb_IOAPIC->ioapic_count++;
        }
    }
    return TRUE;

    AROS_USERFUNC_EXIT
}

/*
 * Process the 'IO-APIC' MADT Table
 * This function counts the available IO-APICs.
 */
AROS_UFH3(static IPTR, ACPI_hook_Table_IOAPIC_Count,
	  AROS_UFHA(struct Hook *, table_hook, A0),
	  AROS_UFHA(ACPI_MADT_IO_APIC *, ioapic, A2),
	  AROS_UFHA(struct ACPI_TABLESCAN_DATA *, tsdata, A1))
{
    AROS_USERFUNC_INIT

    struct PlatformData *pdata = tsdata->acpits_UserData;
    struct ACPI_TABLE_HOOK *scanHook;

    D(bug("[Kernel:ACPI-IOAPIC] ## %s()\n", __func__));

    if (pdata->kb_ACPI->acpi_ioapicCnt == 0)
    {
        D(bug("[Kernel:ACPI-IOAPIC]    %s: Registering IO-APIC Table Parser...\n", __func__));

        scanHook = (struct ACPI_TABLE_HOOK *)AllocMem(sizeof(struct ACPI_TABLE_HOOK), MEMF_CLEAR);
        if (scanHook)
        {
            scanHook->acpith_Node.ln_Name = (char *)ACPI_TABLE_MADT_STR;
            scanHook->acpith_Node.ln_Pri = ACPI_MODPRIO_IOAPIC - 10;                            /* Queue 10 priority levels after the module parser */
            scanHook->acpith_Hook.h_Entry = (APTR)ACPI_hook_Table_IOAPIC_Parse;
            scanHook->acpith_HeaderLen = sizeof(ACPI_TABLE_MADT);
            scanHook->acpith_EntryType = ACPI_MADT_TYPE_IO_APIC;
            scanHook->acpith_UserData = pdata;
            Enqueue(&pdata->kb_ACPI->acpi_tablehooks, &scanHook->acpith_Node);
        }

        scanHook = (struct ACPI_TABLE_HOOK *)AllocMem(sizeof(struct ACPI_TABLE_HOOK), MEMF_CLEAR);
        if (scanHook)
        {
            DINTR(bug("[Kernel:ACPI-IOAPIC]    %s: Registering Interrupt Source Table Parser...\n", __func__));

            scanHook->acpith_Node.ln_Name = (char *)ACPI_TABLE_MADT_STR;
            scanHook->acpith_Node.ln_Pri = ACPI_MODPRIO_IOAPIC - 20;                            /* Queue 20 priority levels after the module parser */
            scanHook->acpith_Hook.h_Entry = (APTR)ACPI_hook_Table_Int_Src_Parse;
            scanHook->acpith_HeaderLen = sizeof(ACPI_TABLE_MADT);
            scanHook->acpith_EntryType = ACPI_MADT_TYPE_INTERRUPT_SOURCE;
            scanHook->acpith_UserData = pdata;
            Enqueue(&pdata->kb_ACPI->acpi_tablehooks, &scanHook->acpith_Node);
        }

        scanHook = (struct ACPI_TABLE_HOOK *)AllocMem(sizeof(struct ACPI_TABLE_HOOK), MEMF_CLEAR);
        if (scanHook)
        {
            DINTR(bug("[Kernel:ACPI-IOAPIC]    %s: Registering Interrupt Override Table Parser...\n", __func__));

            scanHook->acpith_Node.ln_Name = (char *)ACPI_TABLE_MADT_STR;
            scanHook->acpith_Node.ln_Pri = ACPI_MODPRIO_IOAPIC - 30;                            /* Queue 30 priority levels after the module parser */
            scanHook->acpith_Hook.h_Entry = (APTR)ACPI_hook_Table_Int_Src_Ovr_Parse;
            scanHook->acpith_HeaderLen = sizeof(ACPI_TABLE_MADT);
            scanHook->acpith_EntryType = ACPI_MADT_TYPE_INTERRUPT_OVERRIDE;
            scanHook->acpith_UserData = pdata;
            Enqueue(&pdata->kb_ACPI->acpi_tablehooks, &scanHook->acpith_Node);
        }

        scanHook = (struct ACPI_TABLE_HOOK *)AllocMem(sizeof(struct ACPI_TABLE_HOOK), MEMF_CLEAR);
        if (scanHook)
        {
            DINTR(bug("[Kernel:ACPI-IOAPIC]    %s: Registering NMI Source Table Parser...\n", __func__));

            scanHook->acpith_Node.ln_Name = (char *)ACPI_TABLE_MADT_STR;
            scanHook->acpith_Node.ln_Pri = ACPI_MODPRIO_IOAPIC - 40;                            /* Queue 40 priority levels after the module parser */
            scanHook->acpith_Hook.h_Entry = (APTR)ACPI_hook_Table_NMI_Src_Parse;
            scanHook->acpith_HeaderLen = sizeof(ACPI_TABLE_MADT);
            scanHook->acpith_EntryType = ACPI_MADT_TYPE_NMI_SOURCE;
            scanHook->acpith_UserData = pdata;
            Enqueue(&pdata->kb_ACPI->acpi_tablehooks, &scanHook->acpith_Node);
        }
    }
    pdata->kb_ACPI->acpi_ioapicCnt++;

    return TRUE;

    AROS_USERFUNC_EXIT
}


void ACPI_IOAPIC_SUPPORT(struct PlatformData *pdata)
{
    struct ACPI_TABLE_HOOK *scanHook;

//    if (cmdline && strstr(cmdline, "noioapic"))
//        return;

    scanHook = (struct ACPI_TABLE_HOOK *)AllocMem(sizeof(struct ACPI_TABLE_HOOK), MEMF_CLEAR);
    if (scanHook)
    {
        D(bug("[Kernel:ACPI-IOAPIC] %s: Registering IOAPIC Table Parser...\n", __func__));
        D(bug("[Kernel:ACPI-IOAPIC] %s: Table Hook @ 0x%p\n", __func__, scanHook));
        scanHook->acpith_Node.ln_Name = (char *)ACPI_TABLE_MADT_STR;
        scanHook->acpith_Node.ln_Pri = ACPI_MODPRIO_IOAPIC;
        scanHook->acpith_Hook.h_Entry = (APTR)ACPI_hook_Table_IOAPIC_Count;
        scanHook->acpith_HeaderLen = sizeof(ACPI_TABLE_MADT);
        scanHook->acpith_EntryType = ACPI_MADT_TYPE_IO_APIC;
        scanHook->acpith_UserData = pdata;
        Enqueue(&pdata->kb_ACPI->acpi_tablehooks, &scanHook->acpith_Node);
    }
    D(bug("[Kernel:ACPI-IOAPIC] %s: Registering done\n", __func__));
}

DECLARESET(KERNEL__ACPISUPPORT)
ADD2SET(ACPI_IOAPIC_SUPPORT, KERNEL__ACPISUPPORT, 0)