#include "mipsim.hpp"
#include <iostream> //NOT PERMANENT

Stats stats;
Caches caches(0);
unsigned int jumpTo = 0;
bool jump_flag = false;
bool offset_flag = false;
bool lastInstrBranch = false;

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
  stats.cycles++;
  if(jump_flag){ //jump to address
    //check jump delay slot
    if(rg.op){
      stats.hasUsefulJumpDelaySlot++;
    }
    else stats.hasUselessJumpDelaySlot++;

    pc = jumpTo;
    jump_flag = false;
  }
  else if(offset_flag){ //branched true
    pc = pc + jumpTo;
    offset_flag = false;
  }
  else pc = pctarget;

  //check branch delay slots
  if(lastInstrBranch){
    if(rg.op){
      stats.hasUsefulBranchDelaySlot++;
    }
    else stats.hasUselessBranchDelaySlot++;
    lastInstrBranch = false;
  }

  switch(rg.op) {
  case OP_SPECIAL:
    switch(rg.func) {
    case SP_ADDU: case SP_ADD:
      rf.write(rt.rd, rf[rt.rs] + rf[rt.rt]);
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_SUBU: case SP_SUB:
      rf.write(rt.rd, rf[rt.rs] - rf[rt.rt]);
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_SLL:
      rf.write(rt.rd, rf[rt.rt] << rt.sa);
      if(rt.op) { //valid sll
        stats.numRType++;
      }
      else{ //no-op
        stats.instrs--;
      }
      stats.numRegReads++;
      stats.numRegWrites++;
      break;
    case SP_SRL:
      rf.write(rt.rd, rf[rt.rt] >> rt.sa);
      stats.numRType++;
      stats.numRegReads++;
      stats.numRegWrites++;
      break;
    case SP_SRA:
      rf.write(rt.rd, (signed)rf[rt.rt] >> rt.sa);
      stats.numRType++;
      stats.numRegReads++;
      stats.numRegWrites++;
      break;
    case SP_AND:
      rf.write(rt.rd, rf[rt.rs] & rf[rt.rt]);
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_OR:
      rf.write(rt.rd, rf[rt.rs] | rf[rt.rt]);
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_XOR:
      rf.write(rt.rd, rf[rt.rs] ^ rf[rt.rt]);
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_NOR:
      rf.write(rt.rd, !(rf[rt.rs] | rf[rt.rt]));
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_SLT:
      if((signed)rf[rt.rs] < (signed)rf[rt.rt]) {
        rf.write(rt.rd,1);
      }
      else {
         rf.write(rt.rd,0);
      }
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_JR:
      jumpTo = rf[rt.rs];
      jump_flag = true;
      stats.numRType++;
      stats.numRegReads++;
      break;
    case SP_JALR:
      jumpTo = rf[rt.rs];
      jump_flag = true;
      rf.write(rt.rd,pc + 4);
      stats.numRType++;
      stats.numRegReads++;
      stats.numRegWrites++;
      break;
    default:
      instr.printI(instr);
      exit(1);
      break;
    }
    break;

  case OP_ADDIU: case OP_ADDI:
    rf.write(ri.rt, rf[ri.rs] + signExtend16to32ui(ri.imm));
    stats.numIType++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_J:
    jumpTo = (pc & 0xF0000000) | (rj.target << 2);
    jump_flag = true;
    stats.numJType++;
    break;
  case OP_JAL:
    jumpTo = (pc & 0xF0000000) | (rj.target << 2);
    jump_flag = true;
    rf.write(31,pc + 4);
    stats.numJType++;
    stats.numRegWrites++;
    break;
  case OP_BEQ:
    if(rf[ri.rs] == rf[ri.rt]){
      jumpTo = signExtend16to32ui(ri.imm) << 2;
      offset_flag = true;
    }
    if(offset_flag){ //branch taken
      if(ri.imm > 0)
        stats.numForwardBranchesTaken++;
      else stats.numBackwardBranchesTaken++;
    }
    else{ //branch not taken
      if(ri.imm > 0)
        stats.numForwardBranchesNotTaken++;
      else stats.numBackwardBranchesNotTaken++;
    }
    lastInstrBranch = true;
    stats.numIType++;
    stats.numBranches++;
    stats.numRegReads += 2;
    break;
  case OP_BNE:
    if(rf[ri.rt] != rf[ri.rs]){
      jumpTo = signExtend16to32ui(ri.imm) << 2;
      offset_flag = true;
    }
    if(offset_flag){ //branch taken
      if(ri.imm > 0)
        stats.numForwardBranchesTaken++;
      else stats.numBackwardBranchesTaken++;
    }
    else{ //branch not taken
      if(ri.imm > 0)
        stats.numForwardBranchesNotTaken++;
      else stats.numBackwardBranchesNotTaken++;
    }
    lastInstrBranch = true;
    stats.numIType++;
    stats.numBranches++;
    stats.numRegReads += 2;
    break; 
  case OP_BLEZ:
    if((signed)rf[ri.rs] <= 0){
      jumpTo = signExtend16to32ui(ri.imm) << 2;
      offset_flag = true;
    }
    if(offset_flag){ //branch taken
      if(ri.imm > 0)
        stats.numForwardBranchesTaken++;
      else stats.numBackwardBranchesTaken++;
    }
    else{ //branch not taken
      if(ri.imm > 0)
        stats.numForwardBranchesNotTaken++;
      else stats.numBackwardBranchesNotTaken++;
    }
    lastInstrBranch = true;
    stats.numIType++;
    stats.numBranches++;
    stats.numRegReads += 2;
    break;
  case OP_SLTI: case OP_SLTIU:
    if((signed)rf[ri.rs] < (signed)signExtend16to32ui(ri.imm)){
       rf.write(ri.rt, 1);
    }
    else {
       rf.write(ri.rt, 0);
    }
    stats.numIType++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_ORI:
    //rf.write(ri.rt, rf[ri.rs] | ((signExtend16to32ui(ri.imm)<<16)>>16));
    rf.write(ri.rt, rf[ri.rs] | signExtend16to32ui(ri.imm));
    stats.numIType++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_SB:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    dmem.write(addr, (0xFF & rf[ri.rt]) << 24); //move it to msb
    stats.numIType++;
    stats.numMemWrites++;
    stats.numRegReads += 2;
    break;
  case OP_LBU:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    rf.write(ri.rt, dmem[addr].data_ubyte4(0));
    stats.numIType++;
    stats.numMemReads++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_SW:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    dmem.write(addr, rf[ri.rt]);
    stats.numIType++;
    stats.numMemWrites++;
    stats.numRegReads += 2;
    break;
  case OP_LW:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    rf.write(ri.rt, dmem[addr]);
    stats.numIType++;
    stats.numMemReads++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_LB:
    addr = rf[ri.rs] + signExtend16to32ui(ri.imm);
    caches.access(addr);
    rf.write(ri.rt, (signed)(dmem[addr].data_ubyte4(0)));
    stats.numIType++;
    stats.numMemReads++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_LUI:
    //rf.write(ri.rt, rf[ri.rt] & 0x0000FFFF); //clear upper bits
    rf.write(ri.rt, rf[ri.rt] | (ri.imm << 16));
    rf.write(ri.rt, rf[ri.rt] & 0xFFFF0000); //clear lower bits
    stats.numIType++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  default:
    cout << "Unsupported instruction: ";
    instr.printI(instr);
    exit(1);
    break;
  }
}