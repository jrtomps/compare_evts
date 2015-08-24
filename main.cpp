
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
#include <stdexcept>

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
    cout << "      " << setw(6) << " " << "ubSize=" << unbuiltSize << " bSize=" << builtSize << endl;
    printItems(cout, pUBItem, pBItem);
  } else {
    if (! std::equal(pUBItem, pUBItem+ubItem.size()/sizeof(uint16_t), pBItem) ) {
      cout << "index=" << count << " Diffent content observed" << endl;
      printItems(cout, pUBItem, pBItem);
    }
  }
}



CRingItem getNextItem(std::istream& file) 
{
  CRingItem item(VOID);
  while (file && (item.type() != PHYSICS_EVENT)) {
    file >> item;
  }

  return item;
}



bool checkFileState(std::istream& unbuiltFile, std::istream& builtFile) {
  // check for the stream state flags
  if (unbuiltFile.eof()) {
    if (! builtFile.eof()) {
      throw runtime_error("unbuilt file contains more data than unbuilt");
    }
  }
  if (builtFile.eof()) {
    if (! unbuiltFile.eof()) {
      throw runtime_error("built file contains more data than unbuilt");
    }
  }
  if (unbuiltFile.rdstate()!=0 && !unbuiltFile.eof()) {
    throw runtime_error("Unbuilt file has error state");
  }

  if (builtFile.rdstate()!=0 && ! builtFile.eof()) {
    throw runtime_error("Built file has error state");
  }

  return (builtFile.eof() || unbuiltFile.eof());
}




int main(int argc, char* argv[])
{

  if (argc!=3) {
    cout << "Usage: ./compare_evts unbuilt_file built_file" << endl;
    return 1;
  }


  std::string unbuilt_file_path(argv[1]);
  std::string built_file_path(argv[2]);

  std::ifstream unbuiltFile(unbuilt_file_path.c_str());
  if (!unbuiltFile.is_open()) { cout << unbuilt_file_path << " not found" << endl; return 1; }
  std::ifstream builtFile(built_file_path.c_str());
  if (!builtFile.is_open()) { cout << built_file_path << " not found" << endl; return 1; }

  try {
    count = 0;
    while (unbuiltFile && builtFile) {
      CRingItem builtItem = getNextItem(builtFile);
      CRingItem unbuiltItem = getNextItem(unbuiltFile);

      if (checkFileState( unbuiltFile, builtFile)) break;

      compareEventItems(unbuiltItem, builtItem);

      count++;

    }

  } catch (exception& exc) {
    cout << exc.what() << endl;
  }

  cout << "Processed " << count << " events" << endl;

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
