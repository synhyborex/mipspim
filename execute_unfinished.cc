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

//R-TYPE Breakdown
bool R_FULL(Data32 instr) {
  GenericType rgf(instr);

  if(rgf.func == SP_ADDU || rgf.func == SP_ADD || rgf.func == SP_SUBU || rgf.func == SP_SUB ||
  rgf.func == SP_AND || rgf.func == SP_OR || rgf.func == SP_XOR || rgf.func == SP_NOR || 
  rgf.func == SP_SLT) {
     return true;
  } 
  return false;
}

bool R_SA(Data32 instr) {
  GenericType rgf(instr);
  if(rgf.op == 0) {
     if(rgf.func == SP_SLL || rgf.func == SP_SRL || rgf.func == SP_SRA) {
        return true;
     }
  }
  return false;
}

bool R_JR(Data32 instr) {
   GenericType rgf(instr);
   if(rgf.op == 0) {
      if(rgf.func == SP_JR) {
         return true;
      }
   }
   return false;
}

bool R_JALR(Data32 instr) {
   GenericType rgf(instr);
   if(rgf.op == 0) {
      if(rgf.func == SP_JALR) {
         return true;
      }
   }
   return false;
}

//I-Type Breakdown
bool I_BRANCH(Data32 instr) {
   GenericType rgf(instr);

   if(rgf.op == OP_BEQ || rgf.op == OP_BNE || rgf.op == OP_BLEZ) {
      return true;
   }
   return false;
}

bool I_S(Data32 instr) {
   GenericType rgf(instr);

   if(rgf.op == OP_SW || rgf.op == OP_SB) {
      return true;
   } 
   return false;
}

bool I_L(Data32 instr) {
   GenericType rgf(instr);

   if(rgf.op == OP_LBU || rgf.op == OP_LB || rgf.op == OP_LW) {
      return true;
   }
   return false;
}

bool I_IMM(Data32 instr) {
   GenericType rgf(instr);
   
   if(rgf.op == OP_ADDIU || rgf.op == OP_ADDI || rgf.op == OP_SLTIU || rgf.op == OP_SLTI || 
   rgf.op == OP_ORI) {
      return true;
   }
   return false;
}

bool J_J(Data32 instr) {
   GenericType rgf(instr);
   
   if(rgf.op == OP_J) {
      return true;
   }
   return false;
}

bool J_JAL(Data32 instr) {
   GenericType rgf(instr);
   
   if(rgf.op == OP_JAL) {
      return true;
   }
   return false;
}

void execute() {
  Data32 instr = imem[pc];
  Data32 futureInstr = imem[pc + 4];
  Data32 superFutureInstr = imem[pc + 8];
  //Current Instruction
  GenericType rg(instr);
  RType rt(instr);
  IType ri(instr);
  JType rj(instr);
  //Future Instruction
  GenericType rg2(futureInstr);
  RType rt2(futureInstr);
  IType ri2(futureInstr);
  JType rj2(futureInstr);
  //Super Future Instruction
  GenericType rg3(superFutureInstr);
  RType rt3(superFutureInstr);
  IType ri3(superFutureInstr);
  JType rj3(superFutureInstr);

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
/*
   cout << "Instruction: ";

   if(rt.func == SP_SLL && rt.op == 0) {
      cout << "NOP" << endl;
   }
   if(rg.op == OP_J) {
      cout << "J" << endl;
   }
   if(rg.op == OP_LW) {
      cout << "LW" << endl;
   }
   if(rg.op == OP_ADDIU) {
      cout << "ADDIU" << endl;
   }
   if(rg.func == SP_ADDU) {
      cout << "ADDU" << endl;
   }
   if(rg.op == OP_SLTI) {
      cout << "SLTI" << endl;
   }
   if(rg.op == OP_SW) {
      cout << "SW" << endl;
   }
   if(rg.op == OP_BNE) {
      cout << "BNE" << endl;
   }
   else {
      cout << endl;
   }
   if(rt.func == SP_SLL && rt.op == 0) {
      cout << "NOP" << endl;
   }
*/

 //Check for Load Use Hazard
 //If instruction is a Load
   if(I_L(instr)) {
      //If No Op
      if(rt2.func == SP_SLL && rt2.op == 0) {
         stats.loadHasLoadUseStall++;
      }
      //If next instruction uses loaded data
      if(R_FULL(futureInstr)) {
         if(ri.rt == rt2.rs || ri.rt == rt2.rt) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(R_SA(futureInstr)) {
         if(ri.rt == rt2.rt) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(R_JR(futureInstr)) {
         if(ri.rt == rt2.rs) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(R_JALR(futureInstr)) {
         if(ri.rt == rt2.rs) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(I_BRANCH(futureInstr)) {
         if(ri.rt == ri2.rs || ri.rt == ri2.rt) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(I_S(futureInstr)) {
         if(ri.rt == ri2.rs || ri.rt == ri2.rs) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(I_L(futureInstr)) {
         if(ri.rt == ri2.rs) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
      if(I_IMM(futureInstr)) {
         if(ri.rt == ri2.rs) {
            stats.loadHasLoadUseHazard++;
         }
         else {
            stats.loadHasNoLoadUseHazard++;
         }
      }
   }

 //Check for forwarding
 //If current and next instruction is valid.
   if(!(rt.func == SP_SLL && rt.op == 0) && !(rt2.func == SP_SLL && rt2.op == 0)){
      //R_FULL
      if(R_FULL(instr) && R_FULL(futureInstr)) {
         if(rt.rd == rt2.rs || rt.rd == rt2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_FULL(instr) && R_SA(futureInstr)) {
         if(rt.rd == rt2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_FULL(instr) && R_JR(futureInstr)) {
         if(rt.rd == rt2.rs) {
            stats.forwardsFromExec++;
         }  
      }
      else if(R_FULL(instr) && R_JALR(futureInstr)) {
         if(rt.rd == rt2.rs) {
            stats.forwardsFromExec++;
         }  
      }
      else if(R_FULL(instr) && I_BRANCH(futureInstr)) { 
         if(rt.rd == ri2.rs || rt.rd == ri2.rt) {
            stats.forwardsFromExec++;
         }  
      }
      else if(R_FULL(instr) && I_S(futureInstr)) {
         if(rt.rd == ri2.rs || rt.rd == ri2.rt) {
            stats.forwardsFromExec++;
         }  
      }
      else if(R_FULL(instr) && I_L(futureInstr)) {
         if(rt.rd == ri2.rs) {
            stats.forwardsFromExec++;
         }  

      }
      else if(R_FULL(instr) && I_IMM(futureInstr)) {
         if(rt.rd == ri2.rs) {
            stats.forwardsFromExec++;
         }
      }

      //R_SA
      else if(R_SA(instr) && R_FULL(futureInstr)) {
         if(rt.rd == rt2.rt || rt.rd == rt2.rs) {
            stats.forwardsFromExec++;
         }      
      }
      else if(R_SA(instr) && R_SA(futureInstr)) {
         if(rt.rd == rt2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_SA(instr) && R_JR(futureInstr)) {
         if(rt.rd == rt2.rs) {
            stats.forwardsFromExec++;
         }      
      }
      else if(R_SA(instr) && R_JALR(futureInstr)) {
         if(rt.rd == rt2.rs) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_SA(instr) && I_BRANCH(futureInstr)) {
         if(rt.rd == ri2.rs || rt.rd == ri2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_SA(instr) && I_S(futureInstr)) {
         if(rt.rd == ri2.rs || rt.rd == ri2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_SA(instr) && I_L(futureInstr)) {
         if(rt.rd == ri2.rs) {
            stats.forwardsFromExec++;
         }
      }
      else if(R_SA(instr) && I_IMM(futureInstr)) {
         if(rt.rd == ri2.rs) {
            stats.forwardsFromExec++;
         }
      }

      //I_L
      else if(I_L(instr) && R_FULL(futureInstr)) {
         if(ri.rt == rt2.rs || ri.rt == rt2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_L(instr) && R_SA(futureInstr)) {
         if(ri.rt == rt2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_L(instr) && R_JR(futureInstr)) {
         if(ri.rt == rt2.rs) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_L(instr) && R_JALR(futureInstr)) {
         if(ri.rt == rt2.rs) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_L(instr) && I_BRANCH(futureInstr)) {
         if(ri.rt == ri2.rs || ri.rt == ri2.rt) {
            stats.forwardsFromExec++;
            cout << "L Branch" << stats.forwardsFromExec << endl;
         }
      }
      else if(I_L(instr) && I_S(futureInstr)) {
         if(ri.rt == ri2.rs || ri.rt == ri2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_L(instr) && I_L(futureInstr)) {
         if(ri.rt == ri2.rs) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_L(instr) && I_IMM(futureInstr)) {
         if(ri.rt == ri2.rs) {
            stats.forwardsFromExec++;
         } 
      }

      //I_IMM
      else if(I_IMM(instr) && R_FULL(futureInstr)) {
         if(ri.rt == rt2.rs || ri.rt == rt2.rt) {
            stats.forwardsFromExec++;
         }    
      }
      else if(I_IMM(instr) && R_SA(futureInstr)) {
         if(ri.rt == rt2.rt) {
            stats.forwardsFromExec++;
         }    
      }
      else if(I_IMM(instr) && R_JR(futureInstr)) {
         if(ri.rt == rt2.rs) {
            stats.forwardsFromExec++;
         }    
      }
      else if(I_IMM(instr) && R_JALR(futureInstr)) {
         if(ri.rt == rt2.rs) {
            stats.forwardsFromExec++;
         }    
      }
      else if(I_IMM(instr) && I_BRANCH(futureInstr)) {
         if(ri.rt == ri2.rs || ri.rt == ri2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_IMM(instr) && I_S(futureInstr)) {
         if(ri.rt == ri2.rs || ri.rt == ri2.rt) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_IMM(instr) && I_L(futureInstr)) {
         if(ri.rt == ri2.rs) {
            stats.forwardsFromExec++;
         }
      }
      else if(I_IMM(instr) && I_IMM(futureInstr)) {
         if(ri.rt == ri2.rs) {
            stats.forwardsFromExec++;
         }
      }
   }

 //Check for forwarding from mem
 //If current and next instruction is valid.
   if(!(rt.func == SP_SLL && rt.op == 0) && !(rt3.func == SP_SLL && rt3.op == 0)){
      //R_FULL
      if(R_FULL(instr) && R_FULL(superFutureInstr)) {
         if(rt.rd == rt3.rs || rt.rd == rt3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_FULL(instr) && R_SA(superFutureInstr)) {
         if(rt.rd == rt3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_FULL(instr) && R_JR(superFutureInstr)) {
         if(rt.rd == rt3.rs) {
            stats.forwardsFromMem++;
         }  
      }
      else if(R_FULL(instr) && R_JALR(superFutureInstr)) {
         if(rt.rd == rt3.rs) {
            stats.forwardsFromMem++;
         }  
      }
      else if(R_FULL(instr) && I_BRANCH(superFutureInstr)) { 
         if(rt.rd == ri3.rs || rt.rd == ri3.rt) {
            stats.forwardsFromMem++;
         }  
      }
      else if(R_FULL(instr) && I_S(superFutureInstr)) {
         if(rt.rd == ri3.rs || rt.rd == ri3.rt) {
            stats.forwardsFromMem++;
         }  
      }
      else if(R_FULL(instr) && I_L(superFutureInstr)) {
         if(rt.rd == ri3.rs) {
            stats.forwardsFromMem++;
         }  

      }
      else if(R_FULL(instr) && I_IMM(superFutureInstr)) {
         if(rt.rd == ri3.rs) {
            stats.forwardsFromMem++;
         }
      }

      //R_SA
      else if(R_SA(instr) && R_FULL(superFutureInstr)) {
         if(rt.rd == rt3.rt || rt.rd == rt3.rs) {
            stats.forwardsFromMem++;
         }      
      }
      else if(R_SA(instr) && R_SA(superFutureInstr)) {
         if(rt.rd == rt3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_SA(instr) && R_JR(superFutureInstr)) {
         if(rt.rd == rt3.rs) {
            stats.forwardsFromMem++;
         }      
      }
      else if(R_SA(instr) && R_JALR(superFutureInstr)) {
         if(rt.rd == rt3.rs) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_SA(instr) && I_BRANCH(superFutureInstr)) {
         if(rt.rd == ri3.rs || rt.rd == ri3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_SA(instr) && I_S(superFutureInstr)) {
         if(rt.rd == ri3.rs || rt.rd == ri3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_SA(instr) && I_L(superFutureInstr)) {
         if(rt.rd == ri3.rs) {
            stats.forwardsFromMem++;
         }
      }
      else if(R_SA(instr) && I_IMM(superFutureInstr)) {
         if(rt.rd == ri3.rs) {
            stats.forwardsFromMem++;
         }
      }

      //I_L
      else if(I_L(instr) && R_FULL(superFutureInstr)) {
         if(ri.rt == rt3.rs || ri.rt == rt3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && R_SA(superFutureInstr)) {
         if(ri.rt == rt3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && R_JR(superFutureInstr)) {
         if(ri.rt == rt3.rs) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && R_JALR(superFutureInstr)) {
         if(ri.rt == rt3.rs) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && I_BRANCH(superFutureInstr)) {
         if(ri.rt == ri3.rs || ri.rt == ri3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && I_S(superFutureInstr)) {
         if(ri.rt == ri3.rs || ri.rt == ri3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && I_L(superFutureInstr)) {
         if(ri.rt == ri3.rs) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_L(instr) && I_IMM(superFutureInstr)) {
         if(ri.rt == ri3.rs) {
            stats.forwardsFromMem++;
         } 
      }

      //I_IMM
      else if(I_IMM(instr) && R_FULL(superFutureInstr)) {
         if(ri.rt == rt3.rs || ri.rt == rt3.rt) {
            stats.forwardsFromMem++;
         }    
      }
      else if(I_IMM(instr) && R_SA(superFutureInstr)) {
         if(ri.rt == rt3.rt) {
            stats.forwardsFromMem++;
         }    
      }
      else if(I_IMM(instr) && R_JR(superFutureInstr)) {
         if(ri.rt == rt3.rs) {
            stats.forwardsFromMem++;
         }    
      }
      else if(I_IMM(instr) && R_JALR(superFutureInstr)) {
         if(ri.rt == rt3.rs) {
            stats.forwardsFromMem++;
         }    
      }
      else if(I_IMM(instr) && I_BRANCH(superFutureInstr)) {
         if(ri.rt == ri3.rs || ri.rt == ri3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_IMM(instr) && I_S(superFutureInstr)) {
         if(ri.rt == ri3.rs || ri.rt == ri3.rt) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_IMM(instr) && I_L(superFutureInstr)) {
         if(ri.rt == ri3.rs) {
            stats.forwardsFromMem++;
         }
      }
      else if(I_IMM(instr) && I_IMM(superFutureInstr)) {
         if(ri.rt == ri3.rs) {
            stats.forwardsFromMem++;
         }
      }
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
      if(rt.rs == rt.rt == rt.rd == rt.sa) { //no-op
       // cout << "no ops in SLL" << endl;
        stats.instrs--;
      }
      else{ //valid sll
        stats.numRType++;
      }
      stats.numRegReads++;
      stats.numRegWrites++;
      break;
    case SP_SRL:
      rf.write(rt.rd, static_cast<unsigned int>(rf[rt.rt]) >> rt.sa);
      stats.numRType++;
      stats.numRegReads++;
      stats.numRegWrites++;
      break;
    case SP_SRA:
      rf.write(rt.rd, static_cast<int>(rf[rt.rt]) >> rt.sa);
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
      rf.write(rt.rd, rf[rt.rs] < rf[rt.rt] ? 1 : 0);
      stats.numRType++;
      stats.numRegReads += 2;
      stats.numRegWrites++;
      break;
    case SP_JR:
      jumpTo = static_cast<unsigned int>(rf[rt.rs]);
      jump_flag = true;
      stats.numRType++;
      stats.numRegReads++;
      break;
    case SP_JALR:
      jumpTo = static_cast<unsigned int>(rf[rt.rs]);
      jump_flag = true;
      rf.write(31,pc + 4);
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
    if(rf[ri.rs] <= 0){
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
    rf.write(ri.rt, rf[ri.rs] < signExtend16to32ui(ri.imm) ? 1 : 0);
    stats.numIType++;
    stats.numRegReads++;
    stats.numRegWrites++;
    break;
  case OP_ORI:
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
    //rf.write(ri.rt, rf[ri.rt] | (ri.imm << 16));
    //rf.write(ri.rt, rf[ri.rt] & 0xFFFF0000); //clear lower bits
    rf.write(ri.rt, signExtend16to32ui(ri.imm) << 16);
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
