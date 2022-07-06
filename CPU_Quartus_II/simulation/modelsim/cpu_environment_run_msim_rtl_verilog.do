transcript on
if {[file exists rtl_work]} {
	vdel -lib rtl_work -all
}
vlib rtl_work
vmap work rtl_work

vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/timer.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/stack.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/inter_manager.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/i_o_manager.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/dp.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/cu.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/cpu_environment.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/cpu.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/alu.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/memprog.v}
vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Monociclo/src/components.v}

vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Quartus_II/../CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Quartus_II/../CPU_Monociclo/src/cpu_environment_tb.v}

vsim -t 1ps -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cycloneii_ver -L rtl_work -L work -voptargs="+acc"  cpu_tb

add wave *
view structure
view signals
run -all
