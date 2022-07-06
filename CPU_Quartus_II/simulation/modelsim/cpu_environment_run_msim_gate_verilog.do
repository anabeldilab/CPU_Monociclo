transcript on
if {[file exists gate_work]} {
	vdel -lib gate_work -all
}
vlib gate_work
vmap work gate_work

vlog -vlog01compat -work work +incdir+. {cpu_environment_fast.vo}

vlog -vlog01compat -work work +incdir+C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Quartus_II/../CPU_Monociclo/src {C:/Users/alu01/Desktop/cuatri/DDP/CPU_proyecto/CPU_Quartus_II/../CPU_Monociclo/src/cpu_environment_tb.v}

vsim -t 1ps +transport_int_delays +transport_path_delays -L cycloneii_ver -L gate_work -L work -voptargs="+acc"  cpu_tb

add wave *
view structure
view signals
run -all
