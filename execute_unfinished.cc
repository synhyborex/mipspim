#include "mipsim.hpp"
#include <iostream> //NOT PERMANENT

Stats stats;
Caches caches(0);
unsigned int jumpTo = 0;
bool jump_flag = false;
bool offset_flag = false;

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
  if(jump_flag){ //jump to address
    pc = jumpTo;
    jump_flag = false;
    cout << "pc is " << pc << endl;
  }
  else if(offset_flag){
    pc = pc + jumpTo;
    offset_flag = false;
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
      rf.write(rt.rd, rf[rt.rt] << rt.sa);
      break;
    case SP_SRL:
      rf.write(rt.rd, rf[rt.rt] >> rt.sa);
      break;
    case SP_SRA:
      rf.write(rt.rd, rf[rt.rt] / (rt.sa*2));
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
      cout<< "Rf[rt.rs] is" << rf[rt.rs] <<endl;
      cout<< "Rf[rt.rt] is" << rf[rt.rt] <<endl;
      if((signed)rf[rt.rs] < (signed)rf[rt.rt]) {
        cout<<"Determined rs is less than rt (1)" << endl;
        rf.write(rt.rd,1);
      }
      else {
         cout<<"Determined rs is not less than rt (0)" << endl;
         rf.write(rt.rd,0);
      }
      break;
    case SP_JR:
      jumpTo = signExtend16to32ui(rf[rt.rs]);
      jump_flag = true;
      //pc = signExtend16to32ui(rf[rt.rs]);
      break;
    case SP_JALR:
      jumpTo = signExtend16to32ui(rf[rt.rs]);
      jump_flag = true;
      rf.write(rt.rd,pc+4);
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
  case OP_J:
    jumpTo = (pc & 0xf0000000 | rj.target << 2);
    jump_flag = true;
    break;
  case OP_JAL:
    jumpTo = (pc & 0xf0000000 | rj.target << 2);
    jump_flag = true;
    rf.write(31,pc+4);
    break;
  case OP_BEQ:
    if(rf[ri.rs] == rf[ri.rt]){
      jumpTo = signExtend16to32ui(ri.imm) << 2;
      offset_flag = true;
      break;
    }
  case OP_BNE:
    if(rf[ri.rt] != rf[ri.rs]){
      jumpTo = signExtend16to32ui(ri.imm) << 2;
      offset_flag = true;
      //break; Add Break, weird error case still
    }
  case OP_BLEZ:
    if((signed)rf[ri.rs] <= 0){
      jumpTo = signExtend16to32ui(ri.imm) << 2;
      offset_flag = true;
      break;
    }
  case OP_SLTI: case OP_SLTIU:
    if((signed)rf[ri.rs] < (signed)signExtend16to32ui(ri.imm)){
       cout << rf[ri.rs] << " less than " << ri.imm;
       rf.write(ri.rt, 1);
    }
    else {
       cout << rf[ri.rs] << " not less than " << ri.imm;
       rf.write(ri.rt, 0);
    }
    break;
  case OP_ORI:
    rf.write(ri.rt, rf[ri.rs] | signExtend16to32ui(ri.imm));
    break;
  case OP_SB:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    dmem.write(addr, 0xFF & rf[ri.rt]);
    break;
  case OP_LBU:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    rf.write(ri.rt, 0xF000 & dmem[addr]);
    //rf.write(ri.rt, dmem[addr].data_ubyte4(0));
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
  case OP_LB:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    rf.write(ri.rt, (signed)(0xF000 & dmem[addr]));
    break;
  case OP_LUI:
    rf.write(ri.rt, signExtend16to32ui(ri.imm) << 16);
    break;
  default:
    cout << "Unsupported instruction: ";
    instr.printI(instr);
    exit(1);
    break;
  }
}
