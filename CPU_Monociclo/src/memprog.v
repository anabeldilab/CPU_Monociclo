`timescale 1 ns / 10 ps

//Memoria de programa, se inicializa y no se modifica
module memprog(input  wire        clk,
               input  wire [9:0]  a,
               output wire [31:0] rd);

  reg [31:0] mem[0:1023]; //memoria de 1024 palabras de 32 bits de ancho

  initial
  begin
    $readmemb(/* "C:/Users/alu01/Desktop/cuatri/DDP/CPU_Monociclo/src/ */"progfile.mem",mem); // inicializa la memoria del fichero en texto binario
  end
  
  assign rd = mem[a];

endmodule


