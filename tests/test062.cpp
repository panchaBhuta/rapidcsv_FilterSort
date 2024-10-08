// test062.cpp - read cell value by mixed name / index

#include <rapidcsv/rapidcsv.h>
#include "unittest.h"

int main()
{
  int rv = 0;

  std::string csv =
    "-,A,B,C\n"
    "1,3,9,81\n"
    "2,4,16,256\n"
  ;

  std::string path = unittest::TempPath();
  unittest::WriteFile(path, csv);

  try
  {
    rapidcsv::Document doc(path, rapidcsv::LabelParams(rapidcsv::FlgColumnName::CN_PRESENT, rapidcsv::FlgRowName::RN_PRESENT));
    unittest::ExpectEqual(int, doc.GetCell<int>("A", 0), 3);
    unittest::ExpectEqual(int, doc.GetCell<int>("B", 0), 9);
    unittest::ExpectEqual(int, doc.GetCell<int>("C", 0), 81);

    unittest::ExpectEqual(std::string, doc.GetCell<std::string>(0, "2"), "4");
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>(1, "2"), "16");
    unittest::ExpectEqual(std::string, doc.GetCell<std::string>(2, "2"), "256");
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  unittest::DeleteFile(path);

  return rv;
}
