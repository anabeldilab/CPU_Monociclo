/*
 * asm.c
 * Ensamblador configurable v0.1
 * Diseño de Procesadores
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*Var globales*/

//Definición del lenguaje ensamblador

#define INSTSIZE 32        //Tamaño en bits de la instrucción

//Nemónico de cada instrucción

const char* mnemonics[] = { "li", "addi", "subi", "andi", "ori", "noti", "c2i", "mov", "not", "add", "sub", "and", "or", "c2", "c2", "load", "loadr", "store", "j", "jrel", "jz", "jnz", "jcall", "jret", "reti", "nop"};

//Opcode de cada instrucción

const char* opcodes[] = { "0000", "0010", "0011", "0100", "0101", "0110", "0111", "1000", "1001", "1010", "1011", "1100", "1101", "1110", "1111", "00010000", "00010001", "00010010" , "00010011", "00010100", "00010101", "00010110", "00010111", "00011000", "00011001", "00011111"};

// Operandos

#define MAXNUMOPER 3      //Número máximo de operandos posibles en una instrucción

// Codificación de los operandos de cada instrucción (C: cte datos, D: cte de dirección de código, R: campo de registro)

const char* operands[] = { "RC", "RRC", "RRC", "RRC", "RRC", "RC", "RC", "RR", "RR", "RRR", "RRR", "RRR", "RRR", "RR", "RR", "RD", "RRD", "RD", "D", "S", "D", "D", "S", "", "", ""};

//Tamaños de operandos
#define CONSTANTSIZE 16    //Tamaño en bits de una constante C (o dirección de datos si así se considera)
#define REGFIELDSIZE 4     //Tamaño en bits de un campo de registro R
#define DESTDIRSIZE  10    //Tamaño en bits de un campo de dirección de código D
#define SALRELSIZE 10      // Tamaño en bits de un salto S

#define NUMINS (sizeof(mnemonics)/sizeof(mnemonics[0]))     //Número de instrucciones deducido de la matriz de nemónicos

//Posiciones (bit más significativo) de los operandos en la instrucción (de INSTSIZE-1 a 1), 0 significa no usado (no hay operandos de sólo 1 bit)
const int posoper[NUMINS][MAXNUMOPER] = { {3, 27, 0},
                                          {3, 7, 27},
                                          {3, 7, 27},
                                          {3, 7, 27},
                                          {3, 7, 27},
                                          {3, 27, 0},
                                          {3, 27, 0},
                                          {3, 11, 0},
                                          {3, 11, 0},
                                          {3, 11, 7},
                                          {3, 11, 7},
                                          {3, 11, 7},
                                          {3, 11, 7},
                                          {3, 11, 0},
                                          {3, 11, 0},
                                          {3, 23, 0},
                                          {3, 7, 23},
                                          {7, 23, 0},
                                          {9, 0, 0},
                                          {9, 0, 0},
                                          {9, 0, 0},
                                          {9, 0, 0},
                                          {9, 0, 0},
                                          {0, 0, 0},
                                          {0, 0, 0},
                                          {0, 0, 0} };


//*************************************************************************************************************************************************************************
// Normalmente no sería necesario tocar el código de más abajo para adaptar a un ensamblador nuevo, salvo modificaciones en codificación de parámetros como salto relativo
//*************************************************************************************************************************************************************************


#define MAXLINE 256      //Tamaño máximo de una línea de ensamblador en caracteres

constexpr auto MAXPROGRAMLEN = (1 << DESTDIRSIZE);        //Tamaño máximo de la memoria de programa en palabras
char progmem[MAXPROGRAMLEN][INSTSIZE + 1];
char instrucc[INSTSIZE + 1];

//Contador de posición de memoria ($)
int counter = 0;


//Tabla de Referencias simple, cada una un elemento del array
constexpr auto MAXSYMBOLLEN = 50;    //Tamaño máximo de un símbolo (Etiqueta o Constante) en caracteres
#define MAXSYMBREFS 1000
struct SymbRef {
    char Symbol[MAXSYMBOLLEN + 1];
    int  LineRef;
    int  PosRef;
    int  BitPos;
    int  Size;
    int  Relativo;
};
struct SymbRef tablaR[MAXSYMBREFS];
int numRefs = 0;
int sal_rel = 0;

//Tabla de símbolos
#define MAXSYMBOLS 1000
struct SymbEnt {
    char Symbol[MAXSYMBOLLEN + 1];
    int Value;
    int LineDef;
};
struct SymbEnt tablaS[MAXSYMBOLS];
int numSymb = 0;


// Se calcula el numero de bits del operando buscado: C, R, D o S
int operNumBits(int idopcode, int idxOper) {
    char codeOperand = operands[idopcode][idxOper];
    switch (codeOperand) {
    case 'C':
        return CONSTANTSIZE;   //Tamaño en bits de una Constante
        break;
    case 'R':
        return REGFIELDSIZE;    //Tamaño en bits necesario para codificar un Registro
        break;
    case 'D':
        return DESTDIRSIZE;   //Tamaño en bits necesario para codificar una Etiqueta (Destino de un salto)
        break;
    case 'S':
        return SALRELSIZE;   // Tamaño en bits necesario para codificar un salto relativo (constante inmediata)
    default:
        return -1;
    }
}


// Pasamos de texto a binario (?)
void convBin(unsigned int number, char* destStr, size_t numchars) {
    unsigned int numero = number;
    for (int i = 0; i < numchars; i++) {
        if (numero % 2) {
            *(destStr + numchars - 1 - i) = '1';
        }
        else {
            *(destStr + numchars - 1 - i) = '0';
        }
        numero >>= 1;
    }
}

int pow(int n, int p) {
    int acc = 1;
    for (int i = 0; i < p; i++)
        acc *= n;
    return acc;
}

int convertBinaryToDecimal(int n) {
    int decimalNumber = 0, i = 0, remainder;
    while (n != 0) {
        remainder = n % 10;
        n /= 10;
        decimalNumber += remainder*pow(2,i);
        ++i;
    }
    return decimalNumber;
}

// Encuentra strings con nelementos contados (?)
int findStr(const char* str, const char** liststr, int nelem) {
    int i;
    for (i = 0; i < nelem; i++) {
        if (strcmp(liststr[i], str) == 0) {
            return i;
        }
    }
    return -1;
}

// Añadiendo un simbolo que a saber
void addSymbRef(const char* sym, int line, int pos, int bitpos, int numbits) {
    if (numRefs < MAXSYMBREFS) {
        strncpy(tablaR[numRefs].Symbol, sym, MAXSYMBOLLEN+1);
        tablaR[numRefs].LineRef = line;
        tablaR[numRefs].PosRef = pos;
        tablaR[numRefs].BitPos = bitpos;
        tablaR[numRefs].Size = numbits;
        tablaR[numRefs].Relativo = sal_rel;
        //printf("Insert. ref. a símbolo: '%s' ocurrida en la línea fuente %u (instrucción en dir %u) en idx %u de tabla de referencias\n", sym, line, pos, numRefs);
        numRefs++;
    }
    else {
        printf("Tabla de Referencias llena intentando añadir: '%s'\n", sym);
    }
}

void addSymbol(const char* sym, int value, int srcline) {
    if (numSymb < MAXSYMBOLS) {
        strncpy(tablaS[numSymb].Symbol, sym, MAXSYMBOLLEN+1);
        tablaS[numSymb].Value = value;
        tablaS[numSymb].LineDef = srcline;
        //printf("Insertando símbolo: '%s' con valor %u en pos %u de tabla de simbolos\n", sym, value, numSymb);
        numSymb++;
    }
    else {
        printf("Tabla de símbolos llena intentando añadir: '%s'\n", sym);
    }
}

int getSymbolIdx(int lineadef) {
    int idx = -1;
    for (int i = 0; i < numSymb; i++) {
        if (tablaS[i].LineDef == lineadef) {
            idx = i;
            break;
        }
    }
    return idx;
}

int getSymbolValue(const char* sym) {
    int value = -1;
    unsigned sym_size = 0;
    for (int i = 0; i < numSymb; i++) {
        sym_size = strlen(sym) > strlen(tablaS[i].Symbol) ? strlen(sym) : strlen(tablaS[i].Symbol);  // para que pueda existir una etiqueta ref que sea sufijo de otra 
        if (!strncmp(tablaS[i].Symbol, sym, sym_size)) {
            value = tablaS[i].Value;
            break;
        }
    }
    //printf("Buscando valor del simbolo <%s> y obtenido el valor %d\n", sym, value);
    return value;
}

void setSymbolValue(int idx, int value) {
    tablaS[idx].Value = value;
    printf("Asignando al símbolo con ind=%u '%s' el valor %u\n",idx, tablaS[idx].Symbol, value);
}

//Comentarios iniciados con "#", los elimina hasta el final de la línea
int eatComment(FILE* file) {
    //Devuelve true si no se ha acabado el fichero procesando el comentario 
    int c;
    c = fgetc(file);
    if (c == EOF) {
        return 0;
    }
    if (c != '#') {
        ungetc(c, file); //Por si es llamado sin determinar si era comentario
        return 1;
    }
    else {//Procesa el resto del comentario hasta fin de linea
        do {
            c = fgetc(file);
            if (c == EOF) {
                return 0;
            }
        } while (c != '\n'); //Se asume que no se abre en modo binario (Windows compatible)
    }
    return 1;
}


void processMnemonic(FILE* file, char* line, int numread, bool *code, int srcline, int counter) {
    int id = findStr(line, mnemonics, NUMINS);
    if (id == -1) { //No es nemónico, posible pseudoint o error
        *code = false;
        if ((strncmp("equ", line, numread) == 0) || (strncmp("EQU", line, numread) == 0)) { //Caso 'equ' (única pseudo ins por el momento)
            int cte, num;
            num = fscanf(file, " %i", &cte);
            if (num == 1) {
                int idx = getSymbolIdx(srcline);
                if (idx < 0) {
                    printf("No hay símbolo definido para el 'equ' en la línea: %u\n", srcline);
                }
                else {
                    setSymbolValue(idx, cte);
                }
            }
            else {
                printf("Operando incorrecto para el 'equ' en la línea: %u\n", srcline);
            }
        }
        else {
            printf("Nemónico no reconocido en la línea: %u\n", srcline);
        }
    }
    else { //Es nemonico
        *code = true; //Se va a emitir código
        //Lo guardamos en la linea de código máquina actual instrucc
        memcpy(instrucc, opcodes[id], strlen(opcodes[id]));
        //Veamos el número de operandos esperado
        size_t numoper = strlen(operands[id]);
        if (numoper == 0) { //se acabó de momento
            return;
        }
        //Procesamos operandos
        long int posfile; //Posicion del fichero de entrada
        char fmtStr[MAXLINE + 1] = ""; //Cadena de formato scanf de operandos
        char fmtStrSym[MAXLINE + 1] = ""; //Igual para operandos con símbolos
        int num; //Valor devuelto pos scanf como numero de operandos reconocidos
        int ops[MAXNUMOPER] = { 0, 0, 0 }; //Operandos posibles, máximo tres operandos reales de los cuales solo uno puede ser un simbolo que se resolverá o no ahora 
        char simb[MAXSYMBOLLEN + 1] = ""; //símbolo
        strcat(fmtStr, " "); //Permitimos ws iniciales
        strcat(fmtStrSym, " "); //Permitimos ws iniciales
        for (int i = 0; i < numoper; i++) { //Procesamos cada tipo de operando​
            switch (operands[id][i]) { 
            case 'R':
                strcat(fmtStr, "%*[Rr]%2u"); //Registros, intentar leer dos caracteres de num. registro máximo
                if (i != (numoper - 1)) strcat(fmtStr, " , "); //Si no somos el último operando, consumir ws y coma enmedio
                num = fscanf(file, fmtStr, &(ops[i]));
                if (num != 1) {
                    printf("Error buscando operando %d tipo registro con formato '%s' en línea %d\n", i + 1, fmtStr, srcline);
                    exit(1);
                }
                break;
            case 'S':
                sal_rel = 1;
            case 'C':
            case 'D':
                //Podría ser simbólico, intentamos primero con ctes
                posfile = ftell(file); //guardar posición del fichero de entrada
                strcat(fmtStr, "%i"); //Permite leer en hex, octal o decimal con notación del C,valores negativos en decimal (se codificarán en complemento a 2)
                if (i != (numoper - 1)) strcat(fmtStr, " , "); //Si no somos el último operando, consumir ws y coma enmedio
                num = fscanf(file, fmtStr, &(ops[i]));
                if (num != 1) { //No se pudo leer bien el operando
                    fseek(file, posfile, SEEK_SET); //restauramos pos en fichero
                    strcat(fmtStrSym, "%[^ ,\n\t]"); //Leer símbolo como cadena a ver 
                    if (i != (numoper - 1)) strcat(fmtStrSym, "%*[ ,\t]"); //si no somos ultimo operando quita espacios, tabs y coma siguientes
                    num = fscanf(file, fmtStrSym, simb);
                    if (num == 1) { //Ha habido suerte, ahora recuperar el valor del símbolo
                        int val = getSymbolValue(simb);
                        if (val != -1) { //Encontrado
                            if (sal_rel) ops[i] = val - counter;
                            else ops[i] = val;
                        }
                        else { //No encontrado, meter en tabla de referencias para resolverlas al final
                            addSymbRef(simb, srcline, counter, posoper[id][i], operNumBits(id, i));
                        }
                    }
                    else {
                            printf("Error buscando operando %d cte con formato '%s' en la línea %d\n", i + 1, fmtStrSym, srcline);
                            exit(1);
                    }
                }
                sal_rel = 0;
                break;
            default:
                break;
            }
            if (num != 1) {
                printf("Error buscando operando %d en la línea %d\n", i + 1, srcline);
                exit(1);
            }
            //Ahora convertimos el operando cte, de registro o simbolico resuelto en instrucc (el no resuelto estará a cero)
            convBin(ops[i], instrucc + (INSTSIZE - 1) - posoper[id][i], operNumBits(id, i));

            //Inicializa cadenas de formato de siguientes operandos
            *fmtStr = '\0';
            *fmtStrSym = '\0';
        }
        //Ahora tenemos la instrucción completa salvo los símbolos que no han podido ser resueltos (quedan a cero)
    }

}

int readToWhitespace(FILE* file, char* cadena, int maxchars) {
    int c;
    int numread = 0;
    char* ptchar = cadena;
    do {
        c = fgetc(file);
        //printf("L:%c\n", c);
        if (c == EOF) break;
        if (isspace(c)) { //Devuelve el espacio leido al stream y sale
            ungetc(c, file);
            break;
        }
        /* un caracter valido */
        *ptchar++ = (unsigned char)tolower(c); //guarda en lowercase
        numread++;
    } while (numread <= maxchars);
    return numread;
}

int processLine(FILE* file, bool *code, int *currentline, int counter) {
    //devuelve 2 para fin de procesado de linea normal, 1 para fin de fichero con linea procesada lista, 0 para fin de fichero sin linea procesada
    //Los ws deben ser consumidos antes, esperamos entrar con caracter no ws
    char line[MAXLINE + 1];
    int c;
    int numread = 0;
    char* ptchar = line;
    bool isSymbol = false;
    bool isMnemonic = false; //Suposiciones iniciales
    
    do {
        c = fgetc(file);
        //printf("L:%c\n", c);
        if (c == EOF) { //Caso de error o de fin de fichero, línea finalizada en todo caso
            if (isMnemonic) { //Si estabamos procesando la cadena de un nemonico sin operandos
                *ptchar = '\0'; //Acaba la cadena en line
                processMnemonic(file, line, numread, code, *currentline, counter); //Procesalo y finaliza linea
                return 1;
            }
            else {
                return 0;
            }
        }
        if (c == '#') { //Comentario, no hay nada más hasta final de línea, puede acabar nemonico sin operandos
            if (isMnemonic) { //Si estabamos procesando la cadena de un nemonico
                *ptchar = '\0'; //Acaba la cadena en line
                processMnemonic(file, line, numread, code, *currentline, counter); //Procesalo y sigue con el comentario
                ungetc(c, file);
                if (!eatComment(file)) { //EOF, salimos indicando que posiblemente hay ínea
                    return 1; //EOF con el nemonico posiblemente procesado
                }
                else {
                    return 2; //Sigue fichero normal, posiblemente linea procesada
                }
            }
            else {
                ungetc(c, file);
                if (!eatComment(file)) { //Los consume, incluyendo el fin de línea
                    return 0; //EOF con nada pendiente
                }
                else {
                    return 2; //Sigue fichero normal, posiblemente linea procesada
                }
            }
        }
        if ((c == ':') && (isSymbol == false)) { //fin de simbolo y no encontrado antes (no se chequea su cadena)
            if (numread > 0) {
                *ptchar = '\0'; //Acaba la cadena
                addSymbol(line, counter, *currentline);
            }
            else {
                printf("Leída definición de símbolo vacío en línea: %d\n", *currentline);
            }
            isMnemonic = 0; //Permite leer futuro nemónico
            isSymbol = 1; //Impide volver a leer una etiq en esta línea
            ptchar = line; //prepara el buffer para recibir de nuevo
            numread = 0;
            continue; //no tener en cuenta el caracter actual ':' como valido
        }
        if (isspace(c)) {
            //Pueden ser espacios entre etiq y nemonico, previos a un nemonico solo, o espacios en linea sin elem importantes
            if (c == '\n') {
                if (isMnemonic) { //Si estabamos procesando la cadena de un nemonico sin operandos
                    *ptchar = '\0'; //Acaba la cadena en line
                    processMnemonic(file, line, numread, code, *currentline, counter); //Procesalo y termina linea
                }
                *currentline = *currentline + 1;
                break;
            }
            if (isMnemonic) { //Estamos leyendo una cadena y encontramos un ws, ya solo puede ser fin de mnemonico
                *ptchar = '\0'; //Acaba la cadena en line
                processMnemonic(file, line, numread, code, *currentline, counter); //Procesalo y a la vuelta vemos el resto de la linea
                isMnemonic = 0; //impide seguir considerando caracteres
                continue;
            }
            else {  //solo hemos leido ws de momento
                continue; //no guardamos nada ni cambiamos estado
            }
        }
        /* un caracter valido parte de etiq o nemonico*/
        if (!isMnemonic) {
            isMnemonic = true;
        }
        //Hemos empezado la cadena o seguimos añadiendo caracteres, bien por nemonico real o etiq supuesta nemonico
        *ptchar++ = (unsigned char)tolower(c); //guarda en lowercase caracteres sin espacio
        numread++;
    } while (numread <= MAXLINE); //Esto es dudoso, ya que aqui no se procesa una línea entera, da un poco igual
    return 1;
}

// Se come los espacios y los comentarios
int eatWhitespaceAndComments(FILE* file, int* linecount) {
    //Devuelve true si hemos avanzado en el fichero hasta un caracter no ws que no esté dentro de un comentario
    int c;
    do {
        c = fgetc(file);
        if (c == EOF) {
            return 0;
        }
        if (!isspace(c)) { //Devuelve el caracter leido al stream
            ungetc(c, file);
            if (c == '#') {
                if (!eatComment(file)) { //Elimina el comentario (hasta final de la línea)
                    return 0;
                }
                else {
                    (*linecount)++;
                }
            }
            else {
                return 1;
            }
        }
        else {
            if (c == '\n') {
                (*linecount)++;
            }
        }
    } while (1);
}

void ensambla(char* srcfilename, char* dstfilename)
{
    FILE *infile, *outfile;
    int counter = 0;
    int currentline = 1;
    bool codEmitido = false;

    if ((infile = fopen(srcfilename, "r")) == NULL) //Se abre en modo texto
    {
        printf("ERROR: src file '%s' open failed\n", srcfilename);
        exit(1);
    }

    //Inicializa toda la memoria de programa a '0':
    memset(progmem, '0', MAXPROGRAMLEN * (INSTSIZE + 1));
    for (int i = 0; i < MAXPROGRAMLEN; i++) {
        *((char *)progmem + i * (long long)(INSTSIZE + 1) + INSTSIZE) = '\0'; //Cada instrucción como una cadena con "0"s acabada en '\0'
    }

    //Ahora por cada línea de código
    int res;
    int seguirflag = 1;
    do {
        if (eatWhitespaceAndComments(infile, &currentline)) { //quita espacios y comentarios previos actualizando el numero de linea
            memset(instrucc, '0', INSTSIZE); //Preparar el buffer de la instrucción todo a "0"
            instrucc[INSTSIZE] = '\0';
            codEmitido = false;
            res = processLine(infile, &codEmitido, &currentline, counter);
            //printf("I: %s\n", instrucc);
            if (codEmitido && (counter < MAXPROGRAMLEN)) {
                //printf("Copiando la cadena de instrucc %s de tamaño %zu sobre la cadena de contenido ->%s<-\n", instrucc, strlen(instrucc), (char *)progmem[counter]);
                strncpy(progmem[counter], instrucc, INSTSIZE+1);
                //printf("Programa en dir %u es instrucc %s\n",counter, progmem[counter]);
                counter++;
            }
        }
        else {
            break;
        }
    } while (seguirflag);
    //Resolver los símbolos pendientes
    for (int i=0; i < numRefs; i++) {
        //buscamos el símbolo
        int value = getSymbolValue(tablaR[i].Symbol);
        if (value == -1) {
            printf("Símbolo %s que aparece en la línea %u no resuelto\n", tablaR[i].Symbol, tablaR[i].LineRef);
            continue;
        }
        if (tablaR[i].Relativo) {
            convBin(value - tablaR[i].PosRef, progmem[tablaR[i].PosRef] + (INSTSIZE - 1) - tablaR[i].BitPos, tablaR[i].Size);
        } else {
            convBin(value, progmem[tablaR[i].PosRef] + (INSTSIZE - 1) - tablaR[i].BitPos, tablaR[i].Size);
        }
    }
    for (int i = 0; i < 8; i++)
        convBin(convertBinaryToDecimal(atoi(opcodes[24])), progmem[512 + i] + (INSTSIZE - 1) - 31, 8); // opcode de reti en decimal opcodes[24]


    if ((outfile = fopen(dstfilename, "w")) == NULL) //Se abre en modo texto
    {
        printf("ERROR: dest file open failed\n");
        exit(1);
    }
    else {
        if (outfile != NULL) {
            for (int i = 0; i < MAXPROGRAMLEN; i++) {
                fprintf(outfile, "%s\n", progmem[i]);
            }
            fclose(outfile);
        }
    }
}



int main(int argc, char* argv[]){
    if (argc != 2) {
        char *srcfilename = new char[sizeof(argv[1])+1];
        srcfilename = argv[1];
        char *dstfilename = new char[sizeof(argv[2])+1];
        dstfilename = argv[2];
        ensambla(srcfilename, dstfilename);
    } else {
        printf("Número de argumentos invalido.\n");
        return 1;
    }
    return 0;
}
