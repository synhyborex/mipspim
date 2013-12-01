#include "mipsim.hpp"

template<>
void Memory<Data8, Data32>::write(const unsigned int addr, const Data32 data) {
  // data is in native format
  //   if little-endian, 3210 (MSB=3)
  // memory is in big-endian format (0123, MSB=0)
  int i;
  unsigned int myAddr = addr - base;
  if (size() < myAddr + 4) {
    m.resize(myAddr + 4, 0);
  }
  if (myAddr < lowest) lowest = myAddr;
  if (myAddr > highest) highest = myAddr;
  if (opts.writes) {
    cout << hex << addr << ": " << data << " ( ";
    for (i = 0 ; i < 4 ; i++) {
      cout << static_cast<unsigned int>(data.data_ubyte4(i)) << ' ';
    }
    cout << ")" << endl;
  }
  for (i = 0 ; i < 4 ; i++) {
    m[myAddr+i] = Data8(data.data_ubyte4(i));
  }
}

template<>
void Memory<Data32, Data32>::write(const unsigned int addr, const Data32 data) {
  unsigned int myAddr = addr - base;
  // cout << hex << addr << ": " << data << endl;
  m[myAddr] = data;
}

template<>
const Data32 Memory<Data8, Data32>::operator[](const unsigned int addr) const {
  unsigned int myAddr = addr - base;
  return Data32(m[myAddr], m[myAddr+1], m[myAddr+2], m[myAddr+3]);
}

template<>
const Data32 Memory<Data32, Data32>::operator[](const unsigned int addr) const {
  unsigned int myAddr = addr - base;
  return m[myAddr];
}

template<>
void Memory<Data8, Data32>::dump(DataType dt) const {
  Data32 d(0);
  unsigned int addr;
  for (addr = lowest ; addr <= highest + 3 ; addr++) {
    unsigned int byte = addr & 0x3;
    d.set_data_ubyte4(byte, m[addr]);
    // cout << hex << byte << " " << static_cast<unsigned int>(m[addr]) << endl;
    if ((byte == 0x3) && (dt == INSTRUCTIONS)) {
      cout << hex << d << endl;
      d.printI(d);
    } else if ((byte == 0x3) && (dt == DATA) && (d.data_uint() != 0)) {
      cout << addr - 3 + getBase() << ": " << d << endl;
    }
  }
}

template<>
void Memory<Data32, Data32>::dump(DataType dt) const {
  if (dt == DATA) {
    for_each(m.begin(), m.end(), Data32::printD);
  }
}

// This must be updated by the students. It should return true for a
// cache hit and false for a cache miss, and on a cache miss, should
// update the cache tags. The "entries" vector contains the cache
// tags, so if you want to put the tag "t" into cache block "b", then
// evaluate "entries[b] = t;". The locals you have available to help
// you make this decision are "blocksize" (in bytes) and "size" (total
// cache size in blocks). You should also update the "hits" and
// "misses" counters.
bool Cache::access(unsigned int address) { 
  //address is 32 bits
  //size is total size of cache
  //blocksize is line size
  //# lines = size / blocksize

  //number of bits in address for each
  unsigned int byteSelectBits = 0;
  unsigned int indexBits = 0;
  unsigned int tagBits = 0;

  //these will be the versions that get changed
  unsigned int shiftBlocksize = blocksize;
  unsigned int shiftSize = size/blocksize;

  //figure out how many bits we need in the address
  //byte select
  while(shiftBlocksize != 1){
    shiftBlocksize = shiftBlocksize >> 1;
    byteSelectBits++;
  }
  //index
  while(shiftSize != 1){
    shiftSize = shiftSize >> 1;
    indexBits++;
  }
  //tag
  tagBits = sizeof(address)*8 - byteSelectBits - indexBits;

  /*pull out the right bits*/
  unsigned int addr = address;
  //byte select
  addr >>= byteSelectBits; //we don't need the "data", shift it off

  //index
  unsigned int indexMask = 0; //the mask to pull out the index bits
  for(int i = 0; i < indexBits; i++){
    indexMask <<= 1;
    indexMask++; //add another hex 'F' to the mask
  }
  unsigned int cacheIndex = indexMask & addr; //the index in the cache
  /*cout << entries.size()+10 << endl;
  cout << "cache line size is " << blocksize << endl;
  cout << "address size: " << sizeof(address) << endl;
  cout << "byte select bits: " << byteSelectBits << endl;
  cout << "index bits: " << indexBits << endl;
  cout << "tag bits: " << tagBits << endl;
  cout << address << endl;
  cout << indexMask << endl;
  cout << addr << endl;
  cout << cacheIndex << endl;*/

  //tag
  addr >>= indexBits; //shift off index bits now, leaving only the tag
  //cout << addr << endl;

  //check if the cache contains the tag
  if(entries[cacheIndex] == addr){ //success
    hits++;
    return true;
  }
  else{ //failure
    misses++;
    entries[cacheIndex] = addr;
    return false;
  }

  //cout << "test is " << test2 << endl;
  /*cout << "\ncache size is " << size << endl;
  cout << "cache line size is " << blocksize << endl;
  cout << "byte select bits: " << byteSelectBits << endl;
  cout << "index bits: " << indexBits << endl;
  cout << "tag bits: " << tagBits << endl;
  cout << "address size: " << sizeof(address) << endl;
  cout << "test: " << 15 << endl;*/
  //return false;
}

void Stats::print() {
  cout << dec
       << "Total number of dynamic instructions: " << instrs << endl
       << "Total number of cycles: " << cycles << endl
       << "  RType: " << numRType << endl
       << "  IType: " << numIType << endl
       << "  JType: " << numJType << endl
       << "Number of Memory Reads: " << numMemReads << endl
       << "Number of Memory Writes: " << numMemWrites << endl
       << "Number of Register Reads: " << numRegReads << endl
       << "Number of Register Writes: " << numRegWrites << endl
       << "Number of Forwards: " << endl
       << "  From Ex Stage: " << forwardsFromExec << endl
       << "  From Mem Stage: " << forwardsFromMem << endl
       << "Branches: " << endl
       << "  Forward:" << endl
       << "    Taken: " << numForwardBranchesTaken << endl
       << "    Not taken: " << numForwardBranchesNotTaken << endl
       << "  Backward:" << endl
       << "    Taken: " << numBackwardBranchesTaken << endl
       << "    Not taken: " << numBackwardBranchesNotTaken << endl
       << "Branch delay slot: " << endl
       << "  Useful instruction: " << hasUsefulBranchDelaySlot << endl
       << "  Not useful instruction: " << hasUselessBranchDelaySlot << endl
       << "Jump delay slot: " << endl
       << "  Useful instruction: " << hasUsefulJumpDelaySlot << endl
       << "  Not useful instruction: " << hasUselessJumpDelaySlot << endl
       << "Load Use Hazard: " << endl
       << "  Has load use stall: " << loadHasLoadUseStall << endl
       << "  Has load use hazard: " << loadHasLoadUseHazard << endl
       << "  Has no load use hazard: " << loadHasNoLoadUseHazard << endl;
}

unsigned int swizzle(unsigned int d) {
  return (((d >> 24) & 0xff) |
          ((d >> 8)  & 0xff00) |
          ((d << 8)  & 0xff0000) |
          ((d << 24) & 0xff000000));
}
