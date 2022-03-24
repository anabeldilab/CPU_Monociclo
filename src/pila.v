`timescale 1 ns / 10 ps

module pila(input wire clk, reset, push, pop, interrupt, input wire[9:0] pc_addr, output wire [9:0] out, output wire underflow, overflow);

  reg[9:0] mempila[0:15]; //memoria de 16 de tamaño con 10 bits de ancho
  reg[16:0] sp;  // Puede tener un bit más de control

  initial
    sp = 16'b0;

  always @(posedge clk, reset)
  begin
    if (push)
    begin
      sp <= sp + 16'b1;
      mempila[sp + 16'b1] <= pc_addr;
    end
    else if (pop)
    begin
      if (sp[15:0] ==  15'b0)
        sp[16] <= 1'b1;
      else
        sp <= sp - 16'b1;
    end
    else if (reset)
      sp <= 16'b0;
  end
  assign out = interrupt ? mempila[sp] : mempila[sp] + 10'b1;

  assign underflow = reset | !((sp[16] == 1'b1) & (sp[15:0] == 15'b0)) ? 1'b0 : 1'b1;
  assign overflow = reset | !((sp[16] == 1'b1) & (sp[15:0] == 15'b111111111111111)) ? 1'b0 : 1'b1;

endmodule