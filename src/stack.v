`timescale 1 ns / 10 ps

module stack(input  wire       clk, reset, push, pop, interrupt, 
             input  wire [9:0] pc_addr, 
             output wire [9:0] out, 
             output wire       underflow, overflow);

  reg[9:0] stackmem[0:15]; //memoria de 16 de tamaño con 10 bits de ancho
  reg[4:0] sp;  // Un bit más de control

  always @(posedge clk, posedge reset)
  begin
	 if (reset)
      sp <= 5'b0;
    else if (push)
    begin
      if (sp[3:0] == 4'b1111)
        sp[4] <= 1'b1;
      else
      begin
        sp <= sp + 4'b1;
        stackmem[sp + 4'b1] <= pc_addr;
      end
    end
    else if (pop)
    begin
      if (sp[3:0] ==  4'b0)
        sp[4] <= 1'b1;
      else
        sp <= sp - 4'b1;
    end
  end
  assign out = interrupt ? stackmem[sp] : stackmem[sp] + 10'b1;

  assign underflow = !((sp[4] == 1'b1) & (sp[3:0] == 4'b0)) ? 1'b0 : 1'b1;
  assign overflow = !((sp[4] == 1'b1) & (sp[3:0] == 4'b1111)) ? 1'b0 : 1'b1;

endmodule