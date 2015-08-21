
#include <RingIOV11.h>
#include <V11/DataFormatV11.h>
#include <V11/CRingItem.h>
#include <V11/CRingScalerItem.h>
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

      throw 1;
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

  size_t builtSize = bItem.getBodySize();
  size_t unbuiltSize = ubItem.getBodySize();
  uint16_t* pBItemBody  = reinterpret_cast<uint16_t*>(bItem.getBodyPointer());
  uint16_t* pUBItemBody = reinterpret_cast<uint16_t*>(ubItem.getBodyPointer());


  if ( builtSize != unbuiltSize ) {
    cout << "index=" << setw(6) << count << " Differing sizes observed in the ring items" << endl;
    cout << "      " << setw(6) << " " << "ubSize=" << unbuiltSize << "bSize=" << builtSize << endl;
    printItems(cout, pUBItem, pBItem);
  } else {
    if (! std::equal(pUBItemBody, pUBItemBody+ubItem.getBodySize()/sizeof(uint16_t), pBItemBody) ) {
      cout << "index=" << count << " Diffent content observed" << endl;

      printItems(cout, pUBItem, pBItem);
    }
  }
}

void compareScalerItems(CRingItem& ubItem, CRingItem bItem)
{
  unique_ptr<CRingItem> pItem0( CRingItemFactory::createRingItem(ubItem) );
  unique_ptr<CRingItem> pItem1( CRingItemFactory::createRingItem(bItem) );

  CRingScalerItem& sclrItem0 = dynamic_cast<CRingScalerItem&>(*pItem0);
  CRingScalerItem& sclrItem1 = dynamic_cast<CRingScalerItem&>(*pItem1);

  uint16_t* pBItem  = reinterpret_cast<uint16_t*>(bItem.getItemPointer());
  uint16_t* pUBItem = reinterpret_cast<uint16_t*>(ubItem.getItemPointer());

  size_t builtSize = bItem.getBodySize();
  size_t unbuiltSize = ubItem.getBodySize();

  if ( builtSize != unbuiltSize ) {
    cout << "index=" << setw(6) << count << " Differing sizes observed in the ring items" << endl;
    cout << "      " << setw(6) << " " << "ubSize=" << unbuiltSize << "bSize=" << builtSize << endl;
    printItems(cout, pUBItem, pBItem);
  } else {
    if ( sclrItem0.getScalers() != sclrItem1.getScalers() ) {
      cout << "index=" << count << " Diffent content observed" << endl;

      printItems(cout, pUBItem, pBItem);
    }
  }
}


bool eofCondition(std::istream& file0, std::istream& file1) 
{
  bool eofSet = false;

  // check for the stream state flags
  if (file0.eof()) {
    if (! file1.eof()) {
      cout << "built file contains more data than unbuilt" << endl;
    }
    eofSet = true;
  }
 
  if (file1.eof()) {
    if (! file0.eof()) {
      cout << "built file contains more data than unbuilt" << endl;
    } 
    eofSet = true;
  }
  
  return eofSet;
}




bool errorCondition(std::istream& file0, std::istream& file1) 
{
  bool errorFound = false;

  if (file0.rdstate()!=0) {
    cout << "Unbuilt file has error state" << endl;
    errorFound = true;
  }

  if (file1.rdstate()!=0) {
    cout << "Built file has error state" << endl;
    errorFound = true;
  }

  return errorFound;
}




CRingItem getNextItem(std::istream& file, uint32_t type)
{
    CRingItem item(VOID);

    while ( file && (item.type() != type)) {
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

  std::ifstream file0(unbuilt_file_path.c_str());
  if (!file0.is_open()) { cout << "failed to open " << unbuilt_file_path << endl; return 1;}
  std::ifstream file1(built_file_path.c_str());
  if (!file1.is_open()) { cout << "failed to open " << built_file_path << endl; return 1;}

  count = 0;
  uint32_t type = PHYSICS_EVENT;
  while (file0 && file1) {
    CRingItem builtItem   = getNextItem(file0, type);
    CRingItem unbuiltItem = getNextItem(file1, type);

    if ( eofCondition(file0, file1) || errorCondition(file0, file1) ) {
      break;
    }

    if (unbuiltItem.type() == PERIODIC_SCALERS && builtItem.type() == PERIODIC_SCALERS) {
      compareScalerItems(unbuiltItem, builtItem);
    } else if (unbuiltItem.type() == PHYSICS_EVENT && builtItem.type() == PHYSICS_EVENT) {
      compareItems(unbuiltItem, builtItem);
    } else {
      cout << "Files are out of sync! Found different ring item types" << endl;
      break;
    }

    count++;
  }
   
  cout << "Processed " << count << " PHYSICS_EVENTS" << endl;

  return 0;
}

int gpTCLApplication =0;

namespace DAQ {
  namespace V8 {
    int gBufferSize =0;
  }
}
