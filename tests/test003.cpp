// test003.cpp - read column values

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

    std::vector<int> ints;
    std::vector<std::string> strs;

    ints = doc.GetColumn<int>(0);
    unittest::ExpectEqual(size_t, ints.size(), 2);
    unittest::ExpectEqual(int, ints.at(0), 3);
    unittest::ExpectEqual(int, ints.at(1), 4);

    ints = doc.GetColumn<int>("B");
    unittest::ExpectEqual(size_t, ints.size(), 2);
    unittest::ExpectEqual(int, ints.at(0), 9);
    unittest::ExpectEqual(int, ints.at(1), 16);

    strs = doc.GetColumn<std::string>(2);
    unittest::ExpectEqual(size_t, strs.size(), 2);
    unittest::ExpectEqual(std::string, strs.at(0), "81");
    unittest::ExpectEqual(std::string, strs.at(1), "256");
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  unittest::DeleteFile(path);

  return rv;
}
