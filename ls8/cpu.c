#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

#define DATA_LEN 6

void cpu_ram_write(struct cpu *cpu, unsigned char element, int index)
{
  if (index > 0 || index < 256)
  {
    printf("%s\n", "Index out of range.");
  }

  cpu->ram[index] = element;
};

/**
 * Load the binary bytes from a .ls8 source file into a RAM array
 */
void cpu_load(struct cpu *cpu, char *argv)
{
  // open file, read and load instructions into RAM
  FILE *fp;
  fp = fopen(argv, "r");
  char line[1024];
  int index = 0;

  while (fgets(line, sizeof(line), fp) != NULL)
  {
    char *endptr;
    unsigned char file_line = strtoul(line, &endptr, 2);

    if (endptr == line)
    {
      // if this matches, the line must be blank
      continue;
    };

    // load instructions and operands into instructions
    cpu->ram[index] = file_line;
    index++;
  };

  fclose(fp);
}

/**
 * ALU
 */
void alu(struct cpu *cpu, enum alu_op op, unsigned char regA, unsigned char regB)
{

  switch (op)
  {
  case ALU_MUL:
    cpu->registers[0] = cpu->registers[regA] * cpu->registers[regB];
    break;

  case ADD:
    // add the value in two registers and store the result in register A
    cpu->registers[regA] = cpu->registers[regA] + cpu->registers[regB];
    break;

  case ALU_CMP:
    // compare regA and regB
    // flag register, 0b00000LGE
    if (cpu->registers[regA] < cpu->registers[regB])
      cpu->fl = 0b00000100;
    else if (cpu->registers[regA] > cpu->registers[regB])
      cpu->fl = 0b00000010;
    else if (cpu->registers[regA] == cpu->registers[regB])
      cpu->fl = 0b00000001;
    break;
  }
}

/**
 * Run the CPU
 */
void cpu_run(struct cpu *cpu)
{
  int running = 1; // True until we get a HLT instruction
  unsigned char instruction;
  unsigned char operandA, operandB;
  cpu->registers[SP] = 243; // start stack at 0xF3
  int return_address;

  while (running)
  {
    // 1. Get the value of the current instruction (in address PC).
    instruction = cpu->ram[cpu->pc];
    // 2. Figure out how many operands this next instruction requires
    // 3. Get the appropriate value(s) of the operands following this instruction

    if (instruction >> 6 == 2)
    {
      operandA = cpu->ram[cpu->pc + 1];
      operandB = cpu->ram[cpu->pc + 2];
    }
    else if (instruction >> 6 == 1)
    {
      operandA = cpu->ram[cpu->pc + 1];
    }
    // 4. switch() over it to decide on a course of action, increment pc
    switch (instruction)
    {
    // general
    case LDI:
      cpu->registers[operandA] = operandB;
      cpu->pc += 3;
      break;

    case PRN:
      printf("%d\n", cpu->registers[operandA]);
      cpu->pc += 2;
      break;

    case CMP:
      alu(cpu, instruction, operandA, operandB);
      cpu->pc += 3;
      break;

    case HLT:
      running = 0;
      break;

    // stack
    case PUSH:
      cpu->registers[SP]--;
      // copy register value to stack
      cpu->ram[cpu->registers[SP]] = cpu->registers[operandA];
      cpu->pc += 2;
      break;

    case POP:
      // copy value from stack to register
      cpu->registers[operandA] = cpu->ram[cpu->registers[SP]];
      cpu->registers[SP]++;
      cpu->pc += 2;
      break;

    // subroutine
    case CALL:
      return_address = cpu->pc + 2;
      // push return address to stack
      cpu->registers[SP]--;
      cpu->ram[cpu->registers[SP]] = return_address;
      // move pc to subroutine from R0
      cpu->pc = cpu->registers[operandA];
      break;

    case RET:
      // get return address from stack
      return_address = cpu->ram[cpu->registers[SP]];
      // increment SP
      cpu->registers[SP]++;
      // set pc to return address
      cpu->pc = return_address;
      break;

    // mathematics
    case ADD:
      alu(cpu, instruction, operandA, operandB);
      cpu->pc += 3;
      break;

    case MUL:
      alu(cpu, instruction, operandA, operandB);
      cpu->pc += 3;
      break;

    default:
      printf("Unknown instruction %02x at address %02x\n", instruction, cpu->pc);
      exit(1);
    }
  }
}

/**
 * Initialize a CPU struct
 */
void cpu_init(struct cpu *cpu)
{
  // TODO: Initialize the PC and other special registers
  cpu->pc = 0;
  // 8 general-purpose 8-bit numeric registers R0-R7.
  memset(cpu->registers, 0, sizeof(unsigned char) * 8);
  // 8-bit addressing, so can address 256 bytes of RAM total
  memset(cpu->ram, 0, sizeof(unsigned char) * 256);
}

// RAM Methods
int cpu_ram_read(struct cpu *cpu, int index)
{
  if (index > 256 || index < 0)
  {
    printf("%s\n", "Index out of range.");
    return NULL;
  }

  printf("%d\n", cpu->ram[index]);
  return cpu->ram[index];
};
