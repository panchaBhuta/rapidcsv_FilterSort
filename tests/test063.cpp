// test063.cpp - custom conversion per call

#include <iomanip>
#include <math.h>
#include <rapidcsv/rapidcsv.h>
#include "unittest.h"

// Conversion function for int data to be converted to fixed-point two decimal numbers
int ToVal(const std::string& pStr)
{
  return static_cast<int>(roundf(100.0f * std::stof(pStr)));
}

struct Struct
{
  int val = 0;
};

Struct ToStruct(const std::string& pStr)
{
  Struct sVal;
  sVal.val = static_cast<int>(roundf(100.0f * std::stof(pStr)));
  return sVal;
}

int main()
{
  int rv = 0;

  std::string csv =
    "-,A,B,C\n"
    "1,1,10,100,1000\n"
    "2,0.1,0.01,0.001,0.006\n"
  ;

  std::string path = unittest::TempPath();
  unittest::WriteFile(path, csv);

  try
  {
    rapidcsv::Document doc(path, rapidcsv::LabelParams(0, 0));

    // ToVal
    unittest::ExpectEqual(int, doc.GetCell<int COMMA ToVal>(0, 0), 100);
    unittest::ExpectEqual(int, doc.GetCell<int COMMA ToVal>(1, "1"), 1000);
    unittest::ExpectEqual(int, doc.GetCell<int COMMA ToVal>("A", 0), 100);
    unittest::ExpectEqual(int, doc.GetCell<int COMMA ToVal>("B", "1"), 1000);

    unittest::ExpectEqual(int, doc.GetRow<int COMMA ToVal>(0).at(0), 100);
    unittest::ExpectEqual(int, doc.GetRow<int COMMA ToVal>("2").at(0), 10);

    unittest::ExpectEqual(int, doc.GetColumn<int COMMA ToVal>(0).at(0), 100);
    unittest::ExpectEqual(int, doc.GetColumn<int COMMA ToVal>("B").at(0), 1000);

    // ToStruct
    unittest::ExpectEqual(int, doc.GetCell<Struct COMMA ToStruct>(0, 0).val, 100);
    unittest::ExpectEqual(int, doc.GetCell<Struct COMMA ToStruct>(1, "1").val, 1000);
    unittest::ExpectEqual(int, doc.GetCell<Struct COMMA ToStruct>("A", 0).val, 100);
    unittest::ExpectEqual(int, doc.GetCell<Struct COMMA ToStruct>("B", "1").val, 1000);

    unittest::ExpectEqual(int, doc.GetRow<Struct COMMA ToStruct>(0).at(0).val, 100);
    unittest::ExpectEqual(int, doc.GetRow<Struct COMMA ToStruct>("2").at(0).val, 10);

    unittest::ExpectEqual(int, doc.GetColumn<Struct COMMA ToStruct>(0).at(0).val, 100);
    unittest::ExpectEqual(int, doc.GetColumn<Struct COMMA ToStruct>("B").at(0).val, 1000);

    /*
     * all sytem types are supported by default. 
     * any type conversion errors are caught at compile time.
    // Missing custom conversion function
    ExpectException(doc.GetColumn<bool>(0), rapidcsv::no_converter);
    ExpectExceptionMsg(doc.GetColumn<bool>(0), rapidcsv::no_converter,
                       "unsupported conversion datatype");
    */
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  unittest::DeleteFile(path);

  return rv;
}
