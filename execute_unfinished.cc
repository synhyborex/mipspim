#include "mipsim.hpp"

Stats stats;
Caches caches(0);
unsigned int jumpTo = 0;
bool jump_flag = false;

unsigned int signExtend16to32ui(short i) {
  return static_cast<unsigned int>(static_cast<int>(i));
}

void execute() {
  Data32 instr = imem[pc];
  GenericType rg(instr);
  RType rt(instr);
  IType ri(instr);
  JType rj(instr);
  unsigned int pctarget = pc + 4;
  unsigned int addr;
  stats.instrs++;
  if(jump_flag){
    pc = jumpTo;
    jump_flag = false;
  }
  else pc = pctarget;
  switch(rg.op) {
  case OP_SPECIAL:
    switch(rg.func) {
    case SP_ADDU: case SP_ADD:
      rf.write(rt.rd, rf[rt.rs] + rf[rt.rt]);
      break;
    case SP_SUBU: case SP_SUB:
      rf.write(rt.rd, rf[rt.rs] - rf[rt.rt]);
      break;
    case SP_SLL:
      rf.write(rt.rd, rf[rt.rt] << rt.sa;
      break;
    case SP_SRL:
      rf.write(rt.rd, rf[rt.rt] >> rt.sa;
      break;
    case SP_SRA:
      rf.write(rt.rd, rf[rt.rt] / (rt.sa*2);
      break;
    case SP_AND:
      rf.write(rt.rd, rf[rt.rs] & rf[rt.rt]);
      break;
    case SP_OR:
      rf.write(rt.rd, rf[rt.rs] | rf[rt.rt]);
      break;
    case SP_XOR:
      rf.write(rt.rd, rf[rt.rs] ^ rf[rt.rt]);
      break;
    case SP_NOR:
      rf.write(rt.rd, !(rf[rt.rs] | rf[rt.rt]));
      break;
    case SP_SLT:
      if(rf[rt.rs] < rf[rt.rt])
        rf.write(rt.rd,1);
      else rf.write(rt.rd,0);
      break;
    case SP_JR:
      jumpTo = signExtend16to32ui(rf[rt.rs]);
      jump_flag = true;
      //pc = signExtend16to32ui(rf[rt.rs]);
      break;
    case SP_JALR:
      rf.write(rt.rd,pc);
      pc = signExtend16to32ui(rf[rt.rs]);
      break;
    default:
      cout << "Unsupported instruction: ";
      instr.printI(instr);
      exit(1);
      break;
    }
    break;
  case OP_ADDIU: case OP_ADDI:
    rf.write(ri.rt, rf[ri.rs] + signExtend16to32ui(ri.imm));
    break;
  case OP_SW:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    dmem.write(addr, rf[ri.rt]);
    break;
  case OP_LW:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    rf.write(ri.rt, dmem[addr]);
    break;
  default:
    cout << "Unsupported instruction: ";
    instr.printI(instr);
    exit(1);
    break;
  }
}