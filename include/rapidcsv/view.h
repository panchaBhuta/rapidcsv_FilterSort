/*
 * _ViewDocument.h
 *
 * URL:      https://github.com/panchaBhuta/rapidcsv_FilterSort
 * Version:  2.02
 *
 * Copyright (C) 2022-2023 Gautam Dhar
 * All rights reserved.
 *
 * rapidcsv_CT is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#pragma once

#include <vector>
#include <map>
#include <iostream>
#include <string>

#include <rapidcsv/rapidcsv.h>

//TODO documentation

namespace rapidcsv
{
  enum e_SortOrder
  {
    ASCEND = 0,
    DESCEND
  };


  /**
   * @brief     Class representing sort/ordering parameters of CSV document.
   * @tparam    T_C             type info of the column to be sorted.
   *                            T -> can be data-type such as int, double etc ; xOR
   *                            C -> Conversion class statisfying concept 'c_S2Tconverter'.
   * @tparam    SORT_ORDER      ascending or descending order. Default ascending.
   */
  template< typename T_C,
            e_SortOrder SORT_ORDER = e_SortOrder::ASCEND >
  class SortParams
  {
    const size_t      _rawDataColumnIndex;

  public:
    /**
     * @brief       Conversion class statisfying concept 'c_S2Tconverter' for a column
     */
    using S2Tconv_type = t_S2Tconv_c<T_C>;
    /**
     * @brief       return-type either numeric type 'T' or 'variant std::variant<T, std::string>'
     */
    using return_type  = typename S2Tconv_type::return_type;

    /**
     * @brief       Sort ordering for a column (ascending or descending)
     */
    constexpr static e_SortOrder sortOrder = SORT_ORDER;

    /**
     * @brief   Constructor
     * @param   rawDataColumnIndex                Specifies the column index in CSV file.
     */
    SortParams(const size_t rawDataColumnIndex) :
        _rawDataColumnIndex(rawDataColumnIndex)
    {}

    /**
     * @brief   Gets cell-data(convertedfrom string) from Data-row.
     * @param   pRowData                 data row. vector<string>
     * @returns cell data of type R. By default, R is usually same type as T.
     *          Else if 'CONV_S2T ≃ ConvertFromStr_gNaN<T>::ToVal', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     */
    inline typename S2Tconv_type::return_type
    getValue(const Document::t_dataRow& pRowData) const
    {
      const std::string& cellStr = pRowData.at(_rawDataColumnIndex);
      return S2Tconv_type::ToVal(cellStr);
    }
  };

  template <typename, typename = void>
  struct is_SortParams : std::false_type {};

  template <typename CSP>
  struct is_SortParams< CSP, std::void_t< decltype(&CSP::getValue),    // check for the presence of 'NON static' member function CSP::getValue
                                          typename CSP::S2Tconv_type,  // check for the presence of type-def CSP::S2Tconv_type
                                          typename CSP::return_type    // check for the presence of type-def CSP::return_type
                                        >
                        >
          : std::is_same<decltype(&CSP::getValue), typename CSP::return_type(CSP::*)(const Document::t_dataRow& row) const> {}; // member is function

  template <typename CSP>
  concept c_SortParams = is_SortParams<CSP>::value;

  template <typename CSP>
  concept c_NOT_SortParams = !is_SortParams<CSP>::value;



  /**
   * @brief     Factory Class for creating sort keys using list of SortParams.
   * @tparam    SPtypes         variadic list of SortParams, one for each column to be sorted.
   */
  template<c_SortParams ... SPtypes>
  struct SortKeyFactory
  {
    //https://stackoverflow.com/questions/10014713/build-tuple-using-variadic-templates
    /**
     * @brief   set of columns/keys for sorting-ordering, the csv data.
     */
    using t_sortKey = std::tuple<typename SPtypes::return_type ...>;

    /**
     * @brief   create sortKey for indexing.
     * @param   pRowData                 data row. vector<string>
     * @param   sp                       sorting information, one for each ordering-column.
     * @returns tuple representing indexed-key of data-row.
     */
    inline static t_sortKey
    createSortKey(const Document::t_dataRow& pRowData, const SPtypes& ... sp)
    {
      t_sortKey result;
      auto write_tuple = [&pRowData, &sp ... ] (typename SPtypes::return_type & ... wrt_result) -> void
      {
        ( (wrt_result = sp.getValue(pRowData)), ... );  // comma operator ensures the element order is from left to right
      };
      std::apply(write_tuple, result);
      return result;
    }
  };

  /**
   * @brief     Comparator Class for sortKey. Comparision algorith is defined here.
   * @tparam    SPtypes         variadic list of SortParams, one for each column to be sorted.
   */
  template<c_SortParams ... SPtypes>
  struct _RowComparator
  {
  private:
    using t_tuple_SortParams = std::tuple<SPtypes ...>;
    using t_sortKey = typename SortKeyFactory<SPtypes ...>::t_sortKey;

    // https://stackoverflow.com/questions/5512910/explicit-specialization-of-template-class-member-function
    // https://stackoverflow.com/a/28697168/2299954
    template<size_t I,
             std::enable_if_t< I < (sizeof...(SPtypes)-1), int> = 0 >
    inline static bool _compare(const t_sortKey& lhCompositeKey,
                                const t_sortKey& rhCompositeKey)
    {
      using t_tupleSPtype = std::tuple_element_t<I, t_tuple_SortParams>;

      const typename t_tupleSPtype::return_type& lhVal = std::get<I>(lhCompositeKey);
      const typename t_tupleSPtype::return_type& rhVal = std::get<I>(rhCompositeKey);
      if constexpr ( t_tupleSPtype::sortOrder == e_SortOrder::ASCEND )
      {
        if(lhVal < rhVal)  return true;
        if(lhVal > rhVal)  return false;

        // lhVal == rhVal
        return _compare<I+1>(lhCompositeKey,rhCompositeKey);
      } else {
        if(lhVal > rhVal)  return true;
        if(lhVal < rhVal)  return false;

        // lhVal == rhVal
        return _compare<I+1>(lhCompositeKey,rhCompositeKey);
      }
    }

    // https://stackoverflow.com/a/28697168/2299954
    template<size_t I,
             std::enable_if_t< I >= (sizeof...(SPtypes)-1), int> = 0 >
    inline static bool _compare(const t_sortKey& lhCompositeKey,
                                const t_sortKey& rhCompositeKey)
    {
      using t_tupleSPtype = std::tuple_element_t<I, t_tuple_SortParams>;

      const typename t_tupleSPtype::return_type& lhVal = std::get<I>(lhCompositeKey);
      const typename t_tupleSPtype::return_type& rhVal = std::get<I>(rhCompositeKey);
      if constexpr ( t_tupleSPtype::sortOrder == e_SortOrder::ASCEND )
      {
          return (lhVal < rhVal);
      } else {
          return (lhVal > rhVal);
      }
    }


  public:
    /**
     * @brief   Compare two index keys
     * @param   lhCompositeKey           key to be compared.
     * @param   rhCompositeKey           key to be compared.
     * @returns bool value based on key-columns and ordering specification of each column.
     */
    bool operator() (const typename SortKeyFactory<SPtypes ...>::t_sortKey& lhCompositeKey,
                     const typename SortKeyFactory<SPtypes ...>::t_sortKey& rhCompositeKey) const
    {
      return _compare<0>(lhCompositeKey,rhCompositeKey);
    }
  };


  /**
   * @brief     Class representing a CSV document view. The underlying 'Document' is viewed after
   *            applying filter and/or sort on rows, based on cell values after type-conversion.
   *            Only intended for internal usage, but exposed externally to allow specialization on datatypes.
   */
  class _ViewDocument
  {
  protected:
    /**
     * @brief  underlying csv document
     */
    const Document& _document;
    /**
     * @brief a reference to the data structure, which derived class of _ViewDocument can use.
    */
    const std::vector<Document::t_dataRow>& _mData;
    /**
     * @brief indexes of the vector represents view-row-idx.
     *        the values of the vector are the indexes in the data-structure of '_mData'.
     */
    std::vector< size_t> _mapViewRowIdx2RowIdx;
    /**
     * @brief indexes of the vector represents the index in the data-structure of '_mData'.
     *        the values of the vector are view-row-idx.
     *        Since some rows in '_mData' might get filtered out and in that case would be
     *        seen in the view,  those '_mData' indexes will have value -1 to indicate filtered out data.
     */
    std::vector<ssize_t> _mapRowIdx2ViewRowIdx;

    explicit _ViewDocument(const Document& document)
      : _document(document),     _mData(document._mData),
        _mapViewRowIdx2RowIdx(), _mapRowIdx2ViewRowIdx()
    {}

    inline RawIdx _getDataRowIndex(const size_t pRowIdx) const
    {
      return _document._getDataRowIndex(pRowIdx);
    }

  public:
    /**
     * @brief   Get number of view rows (excluding label rows).
     * @returns view-row count.
     */
    inline size_t GetViewRowCount() const
    {
      return _mapViewRowIdx2RowIdx.size();
    }

    // TODO function and unit tests  for ARGS...
    /**
     * @brief   Get column by index.
     * @tparam  T_C                   T can be data-type such as int, double etc ;  xOR
     *                                C -> Conversion class statisfying concept 'c_S2Tconverter'.
     * @param   pColumnNameIdx        column-name or zero-based column-index.
     * @returns 'vector<R>' of column data, excluding the elements filtered out.
     *          By default, R is usually same type as T.
     *          Else if 'C = ConvertFromStr_gNaN<T>', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value, 
     *          else the string value which caused failure during conversion.
     */
    template< typename T_C >
    std::vector<typename t_S2Tconv_c<T_C>::return_type>
    GetViewColumn(const c_sizet_or_string auto& pColumnNameIdx) const
    {
      const size_t pColumnIdx = _document.GetColumnIdx(pColumnNameIdx);
      const size_t dataColumnIdx = _document._getDataColumnIndex(pColumnIdx).dataIdx;
      const size_t firstRowIdx = _document._getDataRowIndex(0).dataIdx;
      std::vector< typename t_S2Tconv_c<T_C>::return_type > column;
      for (auto itViewRowIdx = _mapViewRowIdx2RowIdx.begin();
           itViewRowIdx != _mapViewRowIdx2RowIdx.end(); ++itViewRowIdx)
      {
        const size_t rowIdx = firstRowIdx + static_cast<size_t>(*itViewRowIdx);
        const rapidcsv::Document::t_dataRow& row = _document._mData.at(rowIdx);
        if (dataColumnIdx < row.size())
        {
          const std::string& cellStrVal = row.at(dataColumnIdx);
          typename t_S2Tconv_c<T_C>::return_type val = t_S2Tconv_c<T_C>::ToVal(cellStrVal);
          column.push_back(val);
        } else {
          const std::string errStr = __RAPIDCSV_PREFERRED_PATH__+" : _ViewDocument::GetViewColumn()"+
            " # requested column index " +
            std::to_string(pColumnIdx) + " >= " +
            std::to_string(row.size() - _document._getDataColumnIndex(0).dataIdx) +
            " (number of columns on row index " + std::to_string(rowIdx) + ")";
          throw std::out_of_range(errStr);
        }
      }
      return column;
    }

    /**
     * @brief   Get column either by it's index or name.
     * @tparam  CONV_S2T              conversion function.
     * @param   pColumnNameIdx        column-name or zero-based column-index.
     * @returns 'vector<R>' of column data, excluding the elements filtered out.
     *          By default, R is usually same type as T.
     *          Else if 'CONV_S2T ≃ ConvertFromStr_gNaN<T>::ToVal', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value, 
     *          else the string value which caused failure during conversion.
     */
    template< auto (*CONV_S2T)(const std::string&) >
    inline std::vector< typename f_S2Tconv_c< CONV_S2T >::return_type >
    GetViewColumn(const c_sizet_or_string auto& pColumnNameIdx) const
    {
      return GetViewColumn< f_S2Tconv_c< CONV_S2T > >(pColumnNameIdx);
    }

    /**
     * @brief   Get view-row index by name.
     * @param   pRowName              row label name.
     * @returns zero-based row index.
     *          After removal of all filtered rows, the new row-index of remaining rows.
     *          If 'pRowName' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    inline size_t GetViewRowIdx(const std::string& pRowName) const
    {
      const size_t rowIdx = _document.GetRowIdx(pRowName);

      ssize_t viewRowIdx = _mapRowIdx2ViewRowIdx.at(rowIdx);
      if (viewRowIdx < 0)
      {
        throw std::out_of_range(__RAPIDCSV_PREFERRED_PATH__+" : _ViewDocument::GetViewRowIdx() : row filtered out: " + pRowName);
      }

      return static_cast<size_t>(viewRowIdx);
    }

    /**
     * @brief   Get  actual document-row index by name.
     * @param   pRowName              row label name.
     * @returns zero-based row index.
     *          This is the row-index as seen in the underlying CSV file.
     *          If 'pRowName' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    inline size_t GetDocumentRowIdx(const std::string& pRowName) const
    {
      const size_t rowIdx = _document.GetRowIdx(pRowName);

      ssize_t viewRowIdx = _mapRowIdx2ViewRowIdx.at(rowIdx);
      if (viewRowIdx < 0)
      {
        throw std::out_of_range(__RAPIDCSV_PREFERRED_PATH__+" : _ViewDocument::GetDocumentRowIdx() : row filtered out: " + pRowName);
      }

      return rowIdx;
    }
    /**
     * @brief   Get  actual document-row index by viewRowId.
     * @param   pViewRowIdx              row label name.
     * @returns zero-based row index.
     *          This is the row-index as seen in the underlying CSV file.
     *          If 'pRowName' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    inline size_t GetDocumentRowIdx(const size_t& pViewRowIdx) const
    {
      return _mapViewRowIdx2RowIdx.at(pViewRowIdx);
    }
    inline size_t GetDocumentRowIdx(const int& pViewRowIdx) const
    {
      return _mapViewRowIdx2RowIdx.at(static_cast<size_t>(pViewRowIdx));
    }

    // TODO function and unit tests  for ARGS...
    /**
     * @brief   Get row either by it's index or name.
     * @tparam  T_C                   T can be data-type such as int, double etc ;  xOR
     *                                C -> Conversion class statisfying concept 'c_S2Tconverter'.
     * @param   pRowName_ViewRowIdx   row-name or zero-based row-view-index(i.e index after removing filtered rows).
     * @returns 'tuple<R...>' of row data. By default, R is usually same type as T.
     *          Else if 'C = ConvertFromStr_gNaN<T>', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     *          If 'pRowName_ViewRowIdx' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< typename ... T_C >
    inline std::tuple<typename t_S2Tconv_c<T_C>::return_type ...>
    GetViewRow(const c_sizet_or_string auto& pRowName_ViewRowIdx) const
    {
      const size_t docRowIdx = GetDocumentRowIdx(pRowName_ViewRowIdx);
      return _document.GetRow< t_S2Tconv_c<T_C> ... >(docRowIdx);
    }

    /**
     * @brief   Get row either by it's index or name.
     * @tparam  CONV_S2T              conversion function of type 'R (*CONV_S2T)(const std::string&)'.
     * @param   pRowName_ViewRowIdx   row-name or zero-based row-view-index(i.e index after removing filtered rows).
     * @returns 'tuple<R...>' of row data. By default, R is usually same type as T.
     *          Else if 'CONV_S2T ≃ ConvertFromStr_gNaN<T>::ToVal', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     *          If 'pRowName_ViewRowIdx' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< auto ... CONV_S2T >
    inline std::tuple< typename f_S2Tconv_c< CONV_S2T >::return_type ... >
    GetViewRow(const c_sizet_or_string auto& pRowName_ViewRowIdx) const
    {
      return GetViewRow< f_S2Tconv_c< CONV_S2T >... >(pRowName_ViewRowIdx);
    }

    /**
     * @brief   Get row either by it's index or name.
     * @param   pRowName_ViewRowIdx   row-name or zero-based row-view-index(i.e index after removing filtered rows).
     * @returns 'vector<string>' of row data from view.
     *          If 'pRowName_ViewRowIdx' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    inline std::vector<std::string>
    GetViewRow_VecStr(const c_sizet_or_string auto& pRowName_ViewRowIdx) const
    {
      const size_t docRowIdx = GetDocumentRowIdx(pRowName_ViewRowIdx);
      return _document.GetRow_VecStr(docRowIdx);
    }

    // TODO function and unit tests  for ARGS...
    /**
     * @brief   Get cell by column index and row-name/zero-based-row-view-index.
     * @tparam  T_C                   T can be data-type such as int, double etc ;  xOR
     *                                C -> Conversion class statisfying concept 'c_S2Tconverter'.
     * @param   pColumnNameIdx        column-name or zero-based column index.
     * @param   pRowName_ViewRowIdx   row-name or zero-based row-view-index(i.e index after removing filtered rows).
     * @returns cell data of type R. By default, R is usually same type as T.
     *          Else if 'C ≃ ConvertFromStr_gNaN<T>', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     *          If 'pRowName_ViewRowIdx' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< typename T_C >
    inline typename t_S2Tconv_c<T_C>::return_type
    GetViewCell(const c_sizet_or_string auto& pColumnNameIdx,
	              const c_sizet_or_string auto& pRowName_ViewRowIdx) const
    {
      const size_t pColumnIdx = _document.GetColumnIdx(pColumnNameIdx);
      const size_t docRowIdx  = GetDocumentRowIdx(pRowName_ViewRowIdx);
      return _document.GetCell< t_S2Tconv_c<T_C> >(pColumnIdx, docRowIdx);
    }

    /**
     * @brief   Get cell by column index and row-name/zero-based-row-view-index.
     * @tparam  CONV_S2T              conversion function.
     * @param   pColumnNameIdx        column-name or zero-based column index.
     * @param   pRowName_ViewRowIdx   row-name or zero-based row-view-index(i.e index after removing filtered rows).
     * @returns cell data of type R. By default, R is usually same type as T.
     *          Else if 'CONV_S2T ≃ ConvertFromStr_gNaN<T>::ToVal', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     *          If 'pRowName_ViewRowIdx' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< auto (*CONV_S2T)(const std::string&) >
    inline typename f_S2Tconv_c< CONV_S2T >::return_type
    GetViewCell(const c_sizet_or_string auto& pColumnNameIdx,
	              const c_sizet_or_string auto& pRowName_ViewRowIdx) const
    {
      return GetViewCell< f_S2Tconv_c< CONV_S2T > >( pColumnNameIdx, pRowName_ViewRowIdx);
    }
  };


  typedef bool (* f_EvalBoolExpr)(const Document::t_dataRow& dataRow);

  //TODO  create test testView....cpp  similar to test048.cpp and test049.cpp  which checks for NaN and null value for comparsion and sort
  /**
   * @brief     Class representing a CSV document filtered-view. The underlying 'Document' is viewed after
   *            applying filter on rows, based on cell values after type-conversion.
   * @tparam    evaluateBooleanExpression         boolean function which determines row filtering.
   */
  template<f_EvalBoolExpr evaluateBooleanExpression>
  class FilterDocument : public _ViewDocument
  {
  public:

    /**
     * @brief   Constructor. After excluding the rows as defined by 'evaluateBooleanExpression',
     *          creates bi-directional map between view-rows and actual-csv-rows.
     * @param   document               'Document' object with CSV data.
     */
    explicit FilterDocument(const Document& document)
      : _ViewDocument(document)
    {
      size_t rowIdx = 0;
      ssize_t viewRowIdx = 0;
      const ssize_t rowIdxStart = static_cast<ssize_t>(_getDataRowIndex(0).dataIdx);
      for (auto itRow  = _mData.begin() + rowIdxStart;
                itRow != _mData.end(); ++itRow, ++rowIdx)
      {
        if (evaluateBooleanExpression(*itRow))
        {
          _mapViewRowIdx2RowIdx.push_back(rowIdx);
          _mapRowIdx2ViewRowIdx.push_back(viewRowIdx);
          ++viewRowIdx;
        } else {
          _mapRowIdx2ViewRowIdx.push_back(-10);
        }
      }
    }
  };

/*  not needed
  template< typename T_SP,
            e_SortOrder SORT_ORDER = e_SortOrder::ASCEND >
  struct t_SPpromo;

  //template< c_SortParams SP >
  template< typename T_C, e_SortOrder SORT_ORDER >
  struct t_SPpromo< SortParams<T_C, SORT_ORDER>,
                    SORT_ORDER
                  > {
    using sortparams_type = SortParams<T_C, SORT_ORDER>;
  };

  template< c_NOT_SortParams T,
            e_SortOrder SORT_ORDER >
  struct t_SPpromo<T, SORT_ORDER> {
    using sortparams_type = SortParams<T,SORT_ORDER>;
  };
*/

  /**
   * @brief     Class representing a CSV document filtered-sort-view. The underlying 'Document' is viewed after
   *            applying filter on rows and then sorted on minimum one column, based on cell values after type-conversion.
   * @tparam    evaluateBooleanExpression         boolean function which determines row filtering.
   * @tparam    SPtypes                           variadic list of SortParams satisfying concept 'c_SortParams',
   *                                              one for each column to be sorted.
   */
  template<f_EvalBoolExpr evaluateBooleanExpression, c_SortParams ... SPtypes>
  class FilterSortDocument : public _ViewDocument
  {
  private:
    using t_sortPredicate = _RowComparator<SPtypes ...>;
    using t_sortKey = typename SortKeyFactory<SPtypes ...>::t_sortKey;

    const t_sortPredicate _sortPredicate;
    std::map<t_sortKey, size_t,
             t_sortPredicate > _sortedData;

  public:

    /**
     * @brief   Constructor. After excluding the rows as defined by 'evaluateBooleanExpression',
     *          and sorting the rows based on SortPrams;
     *          creates bi-directional map between view-rows and actual-csv-rows.
     * @param   document               'Document' object with CSV data.
     */
    explicit FilterSortDocument(const Document& document, const SPtypes& ... spArgs)
      : _ViewDocument(document), _sortPredicate(), _sortedData(_sortPredicate)
    {
      size_t rowIdx = 0;
      const ssize_t rowIdxStart = static_cast<ssize_t>(_getDataRowIndex(0).dataIdx);
      for (auto itRow  = _mData.begin() + rowIdxStart;
                itRow != _mData.end(); ++itRow, ++rowIdx)
      {
        if (evaluateBooleanExpression(*itRow))
        {
          t_sortKey rowIndexValues  = SortKeyFactory<SPtypes ...>::createSortKey((*itRow), spArgs ...);
          _sortedData[rowIndexValues] = rowIdx;
        }
        _mapRowIdx2ViewRowIdx.push_back(-10);
      }

      ssize_t viewRowIdx = 0;
      for (auto itMap  = _sortedData.begin();
                itMap != _sortedData.end(); ++itMap, ++viewRowIdx)
      {
        rowIdx = itMap->second;
        _mapViewRowIdx2RowIdx.push_back(rowIdx);
        _mapRowIdx2ViewRowIdx.at(rowIdx) = viewRowIdx;
        // std::cout << "rowIdx("<< rowIdx <<") <-> viewRowIdx(" << viewRowIdx << ")" << std::endl;
      }
    }

    /**
     * @brief   Get row either by it's index or name.
     * @tparam  T_C                   T can be data-type such as int, double etc ;  xOR
     *                                C -> Conversion class statisfying concept 'c_S2Tconverter'.
     * @param   pRowKey               tuple representing indexed-key to data-row.
     * @returns tuple<R...> of row data. By default, R is usually same type as T.
     *          Else if 'C = ConvertFromStr_gNaN<T>', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     *          If 'pRowKey' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< typename ... T_C >
    std::tuple<typename t_S2Tconv_c<T_C>::return_type ...>
    GetOrderedRow(const typename SortKeyFactory<SPtypes ...>::t_sortKey& pRowKey) const
    {
      size_t docRowIdx;
      try {
        docRowIdx  = _sortedData.at(pRowKey);
      } catch (std::out_of_range& err) {
        throw std::out_of_range(__RAPIDCSV_PREFERRED_PATH__+" : FilterSortDocument::GetOrderedRow() : "
              +"rowKey not found in 'sortedKeyMap'. For pRowKey="
              +ConvertFromTuple<typename SPtypes::S2Tconv_type::return_type ...>::ToStr(pRowKey) + " : ERROR: " + err.what());
      }
      return _document.GetRow< t_S2Tconv_c<T_C> ... >(docRowIdx);
    }

    /**
     * @brief   Get row either by it's index or name.
     * @tparam  CONV_S2T              conversion function of type 'R (*CONV_S2T)(const std::string&)'.
     * @param   pRowNameIdx           row-name or zero-based row index.
     * @returns 'tuple<R...>' of row data. By default, R is usually same type as T.
     *          Else if 'CONV_S2T ≃ ConvertFromStr_gNaN<T>::ToVal', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value,
     *          else the string value which caused failure during conversion.
     *          If 'pRowKey' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< auto ... CONV_S2T >
    inline std::tuple< typename f_S2Tconv_c< CONV_S2T >::return_type... >
    GetOrderedRow(const typename SortKeyFactory<SPtypes ...>::t_sortKey& pRowKey) const
    {
      return GetOrderedRow< f_S2Tconv_c< CONV_S2T >... >(pRowKey);
    }

    /**
     * @brief   Get row either by it's index or name.
     * @param   pRowKey               tuple representing indexed-key to data-row.
     * @returns 'vector<std::string>' of row data.
     *          If 'pRowKey' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    inline std::vector<std::string>
    GetOrderedRow_VecStr(const typename SortKeyFactory<SPtypes ...>::t_sortKey& pRowKey) const
    {
      size_t docRowIdx;
      try {
        docRowIdx  = _sortedData.at(pRowKey);
      } catch (std::out_of_range& err) {
        throw std::out_of_range(__RAPIDCSV_PREFERRED_PATH__+" : FilterSortDocument::GetOrderedRow_VecStr() : "
              +"rowKey not found in 'sortedKeyMap'. For pRowKey="
              +ConvertFromTuple<typename SPtypes::S2Tconv_type::return_type ...>::ToStr(pRowKey) + " : ERROR: " + err.what());
      }
      return _document.GetRow_VecStr(docRowIdx);
    }

    /**
     * @brief   Get cell either by it's index or name.
     * @tparam  T_C                   T can be data-type such as int, double etc ;    xOR
     *                                C -> Conversion class statisfying concept 'c_S2Tconverter'.
     * @param   pColumnNameIdx        column-name or zero-based column-index.
     * @param   pRowKey               tuple representing indexed-key to data-row.
     * @returns cell data of type R. By default, R is usually same type as T.
     *          Else if 'C = ConvertFromStr_gNaN<T>', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value, 
     *          else the string value which caused failure during conversion.
     *          If 'pRowKey' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< typename T_C >
    typename t_S2Tconv_c<T_C>::return_type
    GetOrderedCell(const c_sizet_or_string auto& pColumnNameIdx,
	                 const t_sortKey& pRowKey) const
    {
      const size_t pColumnIdx = _document.GetColumnIdx(pColumnNameIdx);
      size_t docRowIdx;
      try {
        docRowIdx  = _sortedData.at(pRowKey);
      } catch (std::out_of_range& err) {
        throw std::out_of_range(__RAPIDCSV_PREFERRED_PATH__+" : FilterSortDocument::GetOrderedCell() : "
              +"rowKey not found in 'sortedKeyMap'. For pRowKey="
              +ConvertFromTuple<typename SPtypes::S2Tconv_type::return_type ...>::ToStr(pRowKey) + " : ERROR: " + err.what());
      }
      return _document.GetCell< t_S2Tconv_c<T_C> >(pColumnIdx, docRowIdx);
    }

    /**
     * @brief   Get cell either by it's index or name.
     * @tparam  CONV_S2T              conversion function.
     * @param   pColumnNameIdx        column-name or zero-based column-index.
     * @param   pRowKey               tuple representing indexed-key to data-row.
     * @returns cell data of type R. By default, R is usually same type as T.
     *          Else if 'CONV_S2T ≃ ConvertFromStr_gNaN<T>::ToVal', then 'R = std::variant<T, std::string>'.
     *          On conversion success variant has the converted value, 
     *          else the string value which caused failure during conversion.
     *          If 'pRowKey' belongs to a filtered out row, then 'out_of_range' error is thrown.
     */
    template< typename T, auto (*CONV_S2T)(const std::string&) >
    inline typename std::invoke_result_t< decltype(CONV_S2T),
                                          const std::string&
                                        >
    GetOrderedCell(const c_sizet_or_string auto& pColumnNameIdx,
	                 const t_sortKey& pRowKey) const
    {
      return GetOrderedCell< S2TwrapperFunction<T, CONV_S2T> >( pColumnNameIdx, pRowKey);
    }
  };



  constexpr bool selectAll(const Document::t_dataRow&) { return true; }

  /**
   * @brief     Class representing a CSV document sort-view. The underlying 'Document' is viewed after
   *            applying sorting on minimum one column, based on cell values after type-conversion.
   * @tparam    SPtypes                           variadic list of SortParams satisfying concept 'c_SortParams',
   *                                              one for each column to be sorted.
   */
  template<c_SortParams ... SPtypes>
  using SortDocument = FilterSortDocument<selectAll, SPtypes ...>;
}
