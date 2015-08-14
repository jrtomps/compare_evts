
#include <RingIOV11.h>
#include <V11/DataFormatV11.h>
#include <V11/CRingItem.h>
#include <V11/CRingItemFactory.h>
#include <FragmentIndex.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdint>
#include <memory>

using namespace std;
using namespace DAQ;
using namespace DAQ::V11;

int count=0;

void printItems(ostream& stream, uint16_t* pUBItem, uint16_t* pBItem)
{
      unique_ptr<CRingItem> priUB( CRingItemFactory::createRingItem(pUBItem));
      stream << "Unbuilt item" << endl;
      stream << priUB->toString() << "\n" << endl;

      unique_ptr<CRingItem> priB( CRingItemFactory::createRingItem(pBItem));
      stream << "Built item" << endl;
      stream << priB->toString() << "\n" << endl;
}
      
void compareEventItems(CRingItem& ubItem, CRingItem bItem)
{
  FragmentIndex index(reinterpret_cast<uint16_t*>(bItem.getBodyPointer()));
  uint16_t* pBItem = index.getFragment(0).s_itemhdr;
  uint16_t* pUBItem = reinterpret_cast<uint16_t*>(ubItem.getItemPointer());

  size_t builtSize = index.getFragment(0).s_size;
  size_t unbuiltSize = ubItem.size();
  if ( builtSize != unbuiltSize ) {
    cout << "index=" << setw(6) << count << " Differing sizes observed in the ring items" << endl;
    cout << "      " << setw(6) << " " << "ubSize=" << unbuiltSize << "bSize=" << builtSize << endl;
    printItems(cout, pUBItem, pBItem);
  } else {
    if (! std::equal(pUBItem, pUBItem+ubItem.size()/sizeof(uint16_t), pBItem) ) {
      cout << "index=" << count << " Diffent content observed" << endl;
      printItems(cout, pUBItem, pBItem);
    }
  }
}

void compareItems(CRingItem& ubItem, CRingItem bItem)
{
  uint16_t* pBItem  = reinterpret_cast<uint16_t*>(bItem.getItemPointer());
  uint16_t* pUBItem = reinterpret_cast<uint16_t*>(ubItem.getItemPointer());

  size_t builtSize = bItem.size();
  size_t unbuiltSize = ubItem.size();
  if ( builtSize != unbuiltSize ) {
    cout << "index=" << setw(6) << count << " Differing sizes observed in the ring items" << endl;
    cout << "      " << setw(6) << " " << "ubSize=" << unbuiltSize << "bSize=" << builtSize << endl;
    printItems(cout, pUBItem, pBItem);
  } else {
    if (! std::equal(pUBItem, pUBItem+ubItem.size()/sizeof(uint16_t), pBItem) ) {
      cout << "index=" << count << " Diffent content observed" << endl;

      printItems(cout, pUBItem, pBItem);
    }
  }
}

bool eofCondition(std::istream& unbuiltFile, std::istream& builtFile) 
{
  bool eofSet = false;

  // check for the stream state flags
  if (unbuiltFile.eof()) {
    if (! builtFile.eof()) {
      cout << "built file contains more data than unbuilt" << endl;
    }
    eofSet = true;
  }
 
  if (builtFile.eof()) {
    if (! unbuiltFile.eof()) {
      cout << "built file contains more data than unbuilt" << endl;
    } 
    eofSet = true;
  }
  
  return eofSet;
}

bool errorCondition(std::istream& unbuiltFile, std::istream& builtFile) 
{
  bool errorFound = false;

  if (unbuiltFile.rdstate()!=0) {
    cout << "Unbuilt file has error state" << endl;
    errorFound = true;
  }

  if (builtFile.rdstate()!=0) {
    cout << "Built file has error state" << endl;
    errorFound = true;
  }

  return errorFound;
}

CRingItem getNextItem(std::istream& file)
{
    CRingItem item(VOID);

    while ( file && (item.type() != PHYSICS_EVENT)) {
      file >> item;
    }

    return item;
}

int main(int argc, char* argv[])
{

  if (argc!=3) {
    cout << "Usage: ./compare_evts file0 file1" << endl;
    return 1;
  }


  std::string unbuilt_file_path(argv[1]);
  std::string built_file_path(argv[2]);

  std::ifstream unbuiltFile(unbuilt_file_path.c_str());
  std::ifstream builtFile(built_file_path.c_str());

  count = 0;
  while (unbuiltFile && builtFile) {
    CRingItem builtItem = getNextItem(unbuiltFile);
    CRingItem unbuiltItem = getNextItem(builtFile);

    if ( eofCondition(unbuiltFile, builtFile) || errorCondition(unbuiltFile, builtFile) ) {
      break;
    }

    compareItems(unbuiltItem, builtItem);

    count++;
  }
   
  unbuiltFile.close();
  builtFile.close();

  return 0;
}

int gpTCLApplication =0;

namespace DAQ {
  namespace V8 {
    int gBufferSize =0;
  }
}
