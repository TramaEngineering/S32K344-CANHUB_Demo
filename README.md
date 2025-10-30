# S32K344-CANHUB_Demo
### Enable/disable VLAN 
1) In peripheral "gPTP_s32K3xx" and check the VLAN option with the PCP at 7 (max priority Network control)
2) In enet.c redefine the macro VLAN_ACTIVE (1U)