
opcode_ED_in_b_mc   equ #40
opcode_ED_in_h_mc   equ #50
opcode_ED_in_b_mc   equ #60
opcode_ED_in_f_mc   equ #70 ; undocumented in f,(c)
opcode_ED_ldi       equ #A0
opcode_ED_ldir      equ #B0

opcode_ED_out_mc_b  equ #41
opcode_ED_out_mc_d  equ #51
opcode_ED_out_mc_h  equ #61
opcode_ED_out_mc_0  equ #71 ; undocumented out (c),0
opcode_ED_cpi       equ #A1
opcode_ED_cpir      equ #B1

opcode_ED_sbc_hl_bc equ #42
opcode_ED_sbc_hl_de equ #52
opcode_ED_sbc_hl_hl equ #62
opcode_ED_sbc_hl_sp equ #72
opcode_ED_ini       equ #A2
opcode_ED_inir      equ #B2

opcode_ED_ld_mnn_bc equ #43
opcode_ED_ld_mnn_bc equ #53
opcode_ED_ld_mnn_hl_shadow equ #63 ; regular opcode is not this one
opcode_ED_ld_mnn_bc equ #73
opcode_ED_outi      equ #A3
opcode_ED_otir      equ #B3

opcode_ED_neg       equ #44
opcode_ED_retn      equ #45
opcode_ED_im0       equ #46
opcode_ED_im1       equ #56

opcode_ED_ld_i_a    equ #47
opcode_ED_ld_a_i    equ #57
opcode_ED_rrd       equ #67

opcode_ED_in_c_mc   equ #48
opcode_ED_in_e_mc   equ #58
opcode_ED_in_l_mc   equ #68
opcode_ED_in_a_mc   equ #78
opcode_ED_ldd       equ #A8
opcode_ED_lddr      equ #B8

opcode_ED_out_mc_c  equ #49
opcode_ED_out_mc_e  equ #59
opcode_ED_out_mc_l  equ #69
opcode_ED_out_mc_a  equ #79
opcode_ED_cpd       equ #A9
opcode_ED_cpdr      equ #B9

opcode_ED_adc_hl_bc equ #4A
opcode_ED_adc_hl_de equ #5A
opcode_ED_adc_hl_hl equ #6A
opcode_ED_adc_hl_sp equ #7A
opcode_ED_ind       equ #AA
opcode_ED_indr      equ #BA

opcode_ED_ld_bc_mnn equ #4B
opcode_ED_ld_de_mnn equ #5B
opcode_ED_ld_hl_mnn_shadow equ #6B
opcode_ED_ld_sp_mnn equ #7B
opcode_ED_outd      equ #AB
opcode_ED_otdr      equ #BB


opcode_ED_reti      equ #4D
opcode_ED_im1       equ #5E

opcode_ED_ld_r_a    equ #4F
opcode_ED_ld_a_r    equ #5F
opcode_ED_rld       equ #6F

