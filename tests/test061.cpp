// test061.cpp - read trimmed cell values

#include <rapidcsv/rapidcsv.h>
#include "unittest.h"

int main()
{
  int rv = 0;

  std::string csv =
    "-, A,B , C \n"
    " 1,3 , 9 , 81\n"
    "2 ,4  , 16, 256  \n"
  ;

  std::string path = unittest::TempPath();
  unittest::WriteFile(path, csv);

  try
  {
    rapidcsv::Document doc(path, rapidcsv::LabelParams(rapidcsv::FlgColumnName::CN_PRESENT, rapidcsv::FlgRowName::RN_PRESENT), rapidcsv::SeparatorParams(',', true));
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>("A", "1"), "3");
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>("B", "1"), "9");
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>("C", "1"), "81");

    unittest::ExpectEqual(std::string, doc.GetCell<std::string>("A", "2"), "4");
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>("B", "2"), "16");
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>("C", "2"), "256");
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  unittest::DeleteFile(path);

  return rv;
}
