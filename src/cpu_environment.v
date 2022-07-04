`timescale 1 ns / 10 ps

module cpu_environment(input wire clk, reset,
                       input wire [9:0] switches,
                       input wire [3:0] buttons,
                       output wire [9:0] led_r,
                       output wire [7:0] led_g,
                       output wire [17:0] addresses,
                       output wire [4:0] control_mem,
                       inout wire [15:0] data);

  wire [7:0] interruptions;
  wire [9:0] program_counter, prueba;
  wire oe;

  // instancia entrada salida
  i_o_manager in_out(clk, reset, oe, addresses[15:0], buttons, switches, led_g, control_mem, data);

  // instancia del procesador
  cpu cpumono(clk, reset, interruptions, oe, addresses[15:0], program_counter, prueba, data);

  // timer
  timer #(30, 8) timer_interrupt(clk, reset, interruptions);
// 135135135 medio segundo con 27MHz

  assign addresses[17:16] = 2'b00;
  
  assign led_r = program_counter;


endmodule