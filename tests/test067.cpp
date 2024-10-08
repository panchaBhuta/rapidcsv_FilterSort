// test067.cpp - test automatic dequote enabled

#include <rapidcsv/rapidcsv.h>
#include "unittest.h"

int main()
{
  int rv = 0;

  std::string csv =
    "\"col 1\"\n"
    "\"\"\n"
    "\" \"\n"
    "\"a b\"\n"
    "\"\"\"a b\"\"\"\n"
    "\" \"\"a\"\" \"\n"
    "\"a\nb\"\n"
  ;

  std::string csvreadref =
    "\"col 1\"\n"
    "\" \"\n"
    "\"a b\"\n"
    "\"\"\"a b\"\"\"\n"
    "\" \"\"a\"\" \"\n"
    "\"a\nb\"\n"
  ;

  std::string path = unittest::TempPath();
  unittest::WriteFile(path, csv);

  try
  {
    // read
    {
      rapidcsv::Document doc(path, rapidcsv::LabelParams(rapidcsv::FlgColumnName::CN_PRESENT, rapidcsv::FlgRowName::RN_MISSING),
                             rapidcsv::SeparatorParams(',', false /* pTrim */, rapidcsv::sPlatformHasCR /* pHasCR */,
                                                       true /* pQuotedLinebreaks */, true /* pAutoQuote */));
      unittest::ExpectEqual(std::string, doc.GetCell<std::string>("col 1", 0), "");
      unittest::ExpectEqual(std::string, doc.GetCell<std::string>("col 1", 1), " ");
      unittest::ExpectEqual(std::string, doc.GetCell<std::string>("col 1", 2), "a b");
      unittest::ExpectEqual(std::string, doc.GetCell<std::string>("col 1", 3), "\"a b\"");
      unittest::ExpectEqual(std::string, doc.GetCell<std::string>("col 1", 4), " \"a\" ");
      unittest::ExpectEqual(std::string, doc.GetCell<std::string>("col 1", 5), "a\nb");
    }

    // write
    {
      unittest::WriteFile(path, csvreadref);
      rapidcsv::Document doc(path, rapidcsv::LabelParams(rapidcsv::FlgColumnName::CN_PRESENT, rapidcsv::FlgRowName::RN_MISSING),
                             rapidcsv::SeparatorParams(',', false /* pTrim */, rapidcsv::sPlatformHasCR /* pHasCR */,
                                                       true /* pQuotedLinebreaks */, true /* pAutoQuote */));

      doc.Save();
      const std::string& csvread = unittest::ReadFile(path);
      unittest::ExpectEqual(std::string, csvreadref, csvread);
    }
  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  unittest::DeleteFile(path);

  return rv;
}
