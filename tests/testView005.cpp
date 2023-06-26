// testView005.cpp - read row value

#include <rapidcsv/view.h>
#include "unittest.h"

bool isFirstCellPositive(const rapidcsv::Document::t_dataRow& dataRow)
{
  // NOTE : at index=1 as zero-index is label
  return (std::stoi(dataRow.at(1))) >= 0;
}


int main()
{
  int rv = 0;

  std::string csv =
    "-,A,B,C\n"
    "1,3,9,81\n"
    "2,-4,16,256\n"
    "3,5,25,625\n"
    "4,-6,36,1296\n"
    "5,7,49,2401\n"
    "6,-8,64,4096\n"
    "7,9,81,6561\n"
  ;

  std::string path = unittest::TempPath();
  unittest::WriteFile(path, csv);

  try
  {
    std::tuple<int, unsigned, long> ints;
    std::vector<std::string> strs;

    rapidcsv::Document doc(path, rapidcsv::LabelParams(0, 0));

    /////  Filter
    rapidcsv::FilterDocument<isFirstCellPositive> viewdoc(doc);

    ints = viewdoc.GetViewRow<int, unsigned, long>(1);
    unittest::ExpectEqual(size_t, std::tuple_size_v<decltype(ints)>, 3);
    unittest::ExpectEqual(int, std::get<0>(ints), 5);
    unittest::ExpectEqual(unsigned, std::get<1>(ints), 25);
    unittest::ExpectEqual(long, std::get<2>(ints), 625);

    ints = viewdoc.GetViewRow<int, unsigned, long>("1");
    unittest::ExpectEqual(size_t, std::tuple_size_v<decltype(ints)>, 3);
    unittest::ExpectEqual(int, std::get<0>(ints), 3);
    unittest::ExpectEqual(unsigned, std::get<1>(ints), 9);
    unittest::ExpectEqual(long, std::get<2>(ints), 81);

    strs = viewdoc.GetViewRow_VecStr(0);
    unittest::ExpectEqual(size_t, strs.size(), 3);
    unittest::ExpectEqual(std::string, strs.at(0), "3");
    unittest::ExpectEqual(std::string, strs.at(1), "9");
    unittest::ExpectEqual(std::string, strs.at(2), "81");

    std::string errMsgPre_v = (std::filesystem::path("include/rapidcsv/view.h")).make_preferred().string();
    ExpectExceptionMsg(viewdoc.GetViewRow<int COMMA unsigned COMMA long>("2"), std::out_of_range,
                       errMsgPre_v + " : _ViewDocument::GetDocumentRowIdx() : row filtered out: 2");

    /////   Sort
    const rapidcsv::SortParams<int> spA(1);
    rapidcsv::SortDocument<int> viewdoc1(doc, spA);

    ints = viewdoc1.GetViewRow<int, unsigned, long>(1);
    unittest::ExpectEqual(size_t, std::tuple_size_v<decltype(ints)>, 3);
    unittest::ExpectEqual(int, std::get<0>(ints), -6);
    unittest::ExpectEqual(unsigned, std::get<1>(ints), 36);
    unittest::ExpectEqual(long, std::get<2>(ints), 1296);

    ints = viewdoc1.GetViewRow<int, unsigned, long>("1");
    unittest::ExpectEqual(size_t, std::tuple_size_v<decltype(ints)>, 3);
    unittest::ExpectEqual(int, std::get<0>(ints), 3);
    unittest::ExpectEqual(unsigned, std::get<1>(ints), 9);
    unittest::ExpectEqual(long, std::get<2>(ints), 81);

    strs = viewdoc1.GetViewRow_VecStr(0);
    unittest::ExpectEqual(size_t, strs.size(), 3);
    unittest::ExpectEqual(std::string, strs.at(0), "-8");
    unittest::ExpectEqual(std::string, strs.at(1), "64");
    unittest::ExpectEqual(std::string, strs.at(2), "4096");

    ////  Filter + Sort
    const rapidcsv::SortParams<int, rapidcsv::e_SortOrder::DESCEND> spD(1);
    rapidcsv::FilterSortDocument<isFirstCellPositive, decltype(spD)> viewdoc2(doc, spD);

    ints = viewdoc2.GetViewRow<int, unsigned, long>(1);
    unittest::ExpectEqual(size_t, std::tuple_size_v<decltype(ints)>, 3);
    unittest::ExpectEqual(int, std::get<0>(ints), 7);
    unittest::ExpectEqual(unsigned, std::get<1>(ints), 49);
    unittest::ExpectEqual(long, std::get<2>(ints), 2401);

    ints = viewdoc2.GetViewRow<int, unsigned, long>("1");
    unittest::ExpectEqual(size_t, std::tuple_size_v<decltype(ints)>, 3);
    unittest::ExpectEqual(int, std::get<0>(ints), 3);
    unittest::ExpectEqual(unsigned, std::get<1>(ints), 9);
    unittest::ExpectEqual(long, std::get<2>(ints), 81);

    strs = viewdoc2.GetViewRow_VecStr(0);
    unittest::ExpectEqual(size_t, strs.size(), 3);
    unittest::ExpectEqual(std::string, strs.at(0), "9");
    unittest::ExpectEqual(std::string, strs.at(1), "81");
    unittest::ExpectEqual(std::string, strs.at(2), "6561");

    ExpectExceptionMsg(viewdoc.GetViewRow<int COMMA unsigned COMMA long>("2"), std::out_of_range,
                       errMsgPre_v + " : _ViewDocument::GetDocumentRowIdx() : row filtered out: 2");

  }
  catch (const std::exception& ex)
  {
    std::cout << ex.what() << std::endl;
    rv = 1;
  }

  unittest::DeleteFile(path);

  return rv;
}
