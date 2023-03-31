/*
 * converter.h
 *
 * URL:      https://github.com/panchaBhuta/rapidcsv_FilterSort
 * Version:  2.01.fs-8.75
 *
 * Copyright (C) 2022-2023 Gautam Dhar
 * All rights reserved.
 *
 * rapidcsv_FilterSort is distributed under the BSD 3-Clause license, see LICENSE for details.
 *
 */

#pragma once

#include <functional>
#include <iostream>
#include <limits>
#include <string>
#include <sstream>
#include <iomanip>


#if  USE_CHRONO == 1
  #include <chrono>
#else
  #include <date/date.h>
#endif

/*
 *  NUMERIC_LOCALE : Number locales are specific settings for the 1000 separators and decimals.
 *                   Some countries use 1.000,00. Others switch the dot and the comma for 1,000.00.
 *                   Same goes for dates as there are different conventions on how to display
 *                   the day, the month, the year.
 */
namespace rapidcsv
{
#if  USE_CHRONO == 1
  namespace datelib = std::chrono;
#else
  namespace datelib = date;
#endif


  template<typename T>
  using f_ConvFuncToVal = std::function<T(const std::string & pStr)>;

  template<typename T, int USE_NUMERIC_LOCALE>
  inline T ToVal(const std::string& pStr);

  /**
   * @brief     variable declaration providing Not-A-Number (NaN) when encountering
   *            conversion error for numerical datatypes when converting from strings.
   */
  template<typename T, int USE_NAN = 1>
  static constexpr
  typename std::enable_if<USE_NAN != 0 && std::is_arithmetic<T>::value, T>::type
  idNaN = (std::numeric_limits<T>::has_signaling_NaN ?
           std::numeric_limits<T>::signaling_NaN() : 0);

  /**
   * @brief     Class providing conversion from strings to numerical datatypes.
   *            Use specialization for custom datatype conversions.
   */
  template<typename T, int USE_NUMERIC_LOCALE = 1, int USE_NAN = 0>
  struct ConvertFromStr
  {
    /**
     * @brief   Converts string holding a numerical value to numerical datatype representation.
     *          A conversion error would return a value of 'NaN' for particular data-type.
     * @param   pStr                  string holding a numerical value
     * @returns                       output T data-type
     */
    static
    typename std::enable_if<0 != USE_NAN, T>::type
    ToVal(const std::string& pStr)
    {
      try
      {
        return ConvertFromStr<T, USE_NUMERIC_LOCALE, 0>::ToVal(pStr);
      } catch (...) {
        return idNaN<T, USE_NAN>;
      }
    }
  };

  /**
   * @brief     Specialized implementation where a Not-A-Number error is propagated to its callee.
   */
  template<typename T, int USE_NUMERIC_LOCALE>
  struct ConvertFromStr<T, USE_NUMERIC_LOCALE, 0>
  {
    /**
     * @brief   Converts string holding a numerical value to numerical datatype representation.
     * @param   pStr                  string holding a numerical value
     * @returns                       output T data-type
     */
    inline static
    typename std::enable_if<(!std::is_same<T, std::string>::value) &&
                                   (0 != USE_NUMERIC_LOCALE),
                             T>::type
    ToVal(const std::string& pStr)
    {
      return rapidcsv::ToVal<T, USE_NUMERIC_LOCALE>(pStr);
    }
  };

  /**
   * @brief     Specialized implementation using string-stream for data conversion.
   *            On Not-A-Number error is thrown.
   */
  template<typename T>
  struct ConvertFromStr<T, 0, 0>
  {
    /**
     * @brief   Converts string holding a numerical value to numerical datatype representation.
     * @param   pStr                  string holding a numerical value
     * @returns                       output T data-type
     */
    static
    typename std::enable_if<!std::is_same<T, std::string>::value,
                            T>::type
    ToVal(const std::string& pStr)
    {
      T val;
      std::istringstream iss(pStr);
      if constexpr (  std::is_same<T, float>::value  ||
                      std::is_same<T, double>::value ||
                      std::is_same<T, long double>::value )
      {
        iss.imbue(std::locale::classic());
      }
      iss >> val;
      if (iss.fail() || iss.bad() || !iss.eof())
      {
        std::string errMsg(
          "Error: in function ConvertFromStr<T,USE_NUMERIC_LOCALE=0,USE_NAN=0>::ToVal(...) -> istringstream: no conversion for '");
        errMsg = errMsg + pStr + "'";
        throw std::invalid_argument(errMsg);
      }
      return val;
    }
  };

  template<>
  struct ConvertFromStr<datelib::year_month_day, 0, 0>
  {
    static
    datelib::year_month_day
    ToYMD(const std::string& pStr, std::string::value_type* fmt)
    {
      datelib::year_month_day ymd;
      std::istringstream iss(pStr);
      datelib::from_stream(iss, fmt, ymd);
      return ymd;
    }
  };

/*
  static
  auto createToYMDfunction(std::string::value_type* fmt)
  {
    return std::bind(ConvertFromStr<T,0,0>::ToYMD,std::placeholders::_1,fmt);
  }
*/

  /**
   * @brief     Specialized implementation handling string to string conversion.
   */
  template<int USE_NUMERIC_LOCALE>
  struct ConvertFromStr<std::string, USE_NUMERIC_LOCALE, 0>
  {
    /**
     * @brief     Specialized implementation handling string to string conversion.
     * @param     pStr                  string
     * @return                          string
     */
    inline static std::string ToVal(const std::string& pStr)
    {
      return pStr;
    }
  };


  template<>
  inline int ToVal<int, 1>(const std::string& pStr)
  {
    return std::stoi(pStr);
  }

  template<>
  inline long ToVal<long, 1>(const std::string& pStr)
  {
    return std::stol(pStr);
  }

  template<>
  inline long long ToVal<long long, 1>(const std::string& pStr)
  {
    return std::stoll(pStr);
  }

  template<>
  inline unsigned ToVal<unsigned, 1>(const std::string& pStr)
  {
    return static_cast<unsigned>(std::stoul(pStr));
  }

  template<>
  inline unsigned long ToVal<unsigned long, 1>(const std::string& pStr)
  {
    return std::stoul(pStr);
  }

  template<>
  inline unsigned long long ToVal<unsigned long long, 1>(const std::string& pStr)
  {
    return std::stoull(pStr);
  }

  template<>
  inline float ToVal<float, 1>(const std::string& pStr)
  {
    return std::stof(pStr);
  }

  template<>
  inline double ToVal<double, 1>(const std::string& pStr)
  {
    return std::stod(pStr);
  }

  template<>
  inline long double ToVal<long double, 1>(const std::string& pStr)
  {
    return std::stold(pStr);
  }

  template<>
  inline char ToVal<char, 1>(const std::string& pStr)
  {
    return pStr[0];
  }

  template<>
  inline bool ToVal<bool, 1>(const std::string& pStr)
  {
    return std::stoll(pStr) > 0 ? true : false;
  }



  template<typename T>
  using f_ConvFuncToStr = std::function<std::string(const T & pVal)>;

  template<typename T, int USE_NUMERIC_LOCALE>
  inline std::string ToStr(const T& pVal);

  template<typename T>
  constexpr int getDecimalPrecision()
  {
    return (   std::is_same<T, float>::value  ? 9  :
            (  std::is_same<T, double>::value ? 17 :
             ( std::is_same<T, long double>::value ? 25 :
               -1 ) ) );
  }

  /**
   * @brief     Class providing conversion from numerical datatypes to strings.
   *            Use specialization for custom datatype conversions.
   */
  template<typename T, int USE_NUMERIC_LOCALE=0,
           const int DECIMAL_PRECISION=-1>   // default value triggers a call to getDecimalPrecision<T>()
  struct ConvertFromVal
  {
    constexpr static int decimalPrecision = (DECIMAL_PRECISION<0) ? getDecimalPrecision<T>() : DECIMAL_PRECISION;

    /**
     * @brief   Converts a numerical value to string.
     * @param   pVal                  numerical value
     * @returns                       string
     */
    static
    typename std::enable_if<(!std::is_same<T, std::string>::value) &&
                                   (!std::is_same<T, char>::value),
                            std::string>::type
    ToStr(const T& pVal)
    {
      std::ostringstream oss;
      if constexpr(    std::is_same<T, float>::value  ||
                       std::is_same<T, double>::value ||
                       std::is_same<T, long double>::value)
      {
        oss << std::defaultfloat << std::setprecision(decimalPrecision) << pVal;
      } else {
        oss << pVal;
      }

      if (oss.fail() || oss.bad()) // || !oss.eof())
      {
        std::string errMsg("Error: in function ConvertFromVal<T,USE_NUMERIC_LOCALE="
                           + std::to_string(USE_NUMERIC_LOCALE) + ", DECIMAL_PRECISION=" + std::to_string(DECIMAL_PRECISION)
                           + ">::ToStr(...) -> ostringstream: no conversion for template type");
        throw std::invalid_argument(errMsg);
      }
      return oss.str();
    }
  };

  /**
   * @brief     Specialized implementation using Numeric-Local conversions.
   */
  // for types refer :: https://en.cppreference.com/w/cpp/language/type
  template<typename T>
  struct ConvertFromVal<T, 1>
  {
    /**
     * @brief   Converts numerical value to string using system numeric-local function.
     * @param   pVal                  numerical value
     * @returns                       string
     */
    inline static
    // refer  ::  https://stackoverflow.com/questions/11055923/stdenable-if-parameter-vs-template-parameter/11056146#11056146
    typename std::enable_if<std::is_integral<T>::value &&
                                   (!std::is_same<T, char>::value),
                            std::string>::type
    ToStr( const T& pVal)
    {
      // refer :: https://en.cppreference.com/w/cpp/string/basic_string/to_string
      return std::to_string(pVal);
    }
  };

  /**
   * @brief     Specialized implementation using Numeric-Local conversions.
   */
  template<typename T>
  struct ConvertFromVal<T, -1>
  {
    /**
     * @brief   Converts floating point numbers to string using system numeric-local function.
     * @param   pVal                  numerical value
     * @returns                       string
     */
    inline static
    typename std::enable_if<std::is_floating_point<T>::value &&
                                    (!std::is_same<T, char>::value),
                            std::string>::type
    ToStr(const T& pVal)
    {
      /*
         WARNING :: With floating point types std::to_string may yield unexpected results as the number
                    of significant digits in the returned string can be zero, for e.g: pVal = 1e-09

                    The return value may differ significantly from what std::cout prints by default, see the example.

          That's why this specialization is with USE_NUMERIC_LOCALE=-1  and not with 1

         std::cout: 23.43
         to_string: 23.430000

         std::cout: 1e-09
         to_string: 0.000000

         std::cout: 1e+40
         to_string: 10000000000000000303786028427003666890752.000000

         std::cout: 1e-40
         to_string: 0.000000

         std::cout: 1.23457e+08
         to_string: 123456789.000000
       */
      return std::to_string(pVal);
    }
  };

  /**
   * @brief     Specialized implementation handling char to string conversion.
   */
  template<int USE_NUMERIC_LOCALE>
  struct ConvertFromVal<char, USE_NUMERIC_LOCALE>
  {
    /**
     * @brief     Specialized implementation handling string to string conversion.
     * @param     pVal                  char
     * @return                          string
     */
    inline static std::string ToStr(const char& pVal)
    {
      return std::string(1, pVal);
    }
  };

  /**
   * @brief     Specialized implementation handling string to string conversion.
   */
  template<int USE_NUMERIC_LOCALE>
  struct ConvertFromVal<std::string, USE_NUMERIC_LOCALE>
  {
    /**
     * @brief     Specialized implementation handling string to string conversion.
     * @param     pVal                  string
     * @return                          string
     */
    inline static std::string ToStr(const std::string& pVal)
    {
      return pVal;
    }
  };

}
