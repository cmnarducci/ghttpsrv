#ifndef __TOKENITERATOR_H__
#define __TOKENITERATOR_H__

#include <string>
#include <sstream>

template <typename T>
class TokenIterator
{
   bool _end;
   std::string::size_type _current;
   std::string _data;
   std::string _delimiters;
   std::string _token;
   bool _empty;

   T convert(std::string v) const
   {
      T r;
      std::istringstream iss(v);
      iss >> r;
      return r;
   };
   std::string next()
   {
      std::string r;
      if (_current == std::string::npos)
      {
         _end = true;
         return r;
      }
      std::string::size_type pos;
      if (_empty)
      {
         pos = _data.find_first_of(_delimiters, _current);
      }
      else
      {
         _current = _data.find_first_not_of(_delimiters, _current);
         if (_current == std::string::npos)
         {
            _end = true;
            return r;
         }
         pos = _data.find_first_of(_delimiters, _current);
      }
      if (pos == std::string::npos)
      {
         r = _data.substr(_current);
         _current = std::string::npos;
      }
      else
      {
         r = _data.substr(_current, pos - _current);
         _current = pos + 1;
      }
      return r;
   };
   void Init(const char *s, const char *delimiters, bool empty)
   {
      _current = 0;
      _data = s;
      _end = (_data.length() == 0);
      _empty = empty;
      if (delimiters)
         _delimiters = delimiters;
      _token = next();
   };

public:
   TokenIterator(const char *s, const char *delimiters = " \t\b\n\"'`~!@#$%^&*-+=|\\/?.,;:(){}[]<>", bool empty = false)
   {
      Init(s, delimiters, empty);
   };
   TokenIterator(const std::string& s, const char *delimiters = " \t\b\n\"'`~!@#$%^&*-+=|\\/?.,;:(){}[]<>", bool empty = false)
   {
      Init(s.c_str(), delimiters, empty);
   };
   ~TokenIterator()
   {
   };
   TokenIterator& operator=(const std::string& s)
   {
      _current = 0;
      _data = s;
      _end = (_data.length() == 0);
      _token = next();
      return *this;
   };
   TokenIterator(const TokenIterator& old)
   {
      *this = old;
   };
   TokenIterator& operator=(const TokenIterator& r)
   {
      _end = r._end; _current = r._current;
      _data = r._data; _delimiters = r._delimiters;
      _empty = r._empty;
      _token = r._token;
      return *this;
   };
   bool operator==(const TokenIterator& r) const
   {
      return _end == r._end && _current == r._current && _data == r._data && _empty == r._empty;
   };
   bool operator!=(const TokenIterator& r) const
   {
      return !(*this == r);
   };
   T operator*() const
   {
      return convert(_token);
   };
   TokenIterator& operator++()
   {
      _token = next();
      return *this;
   };
   TokenIterator operator++(int)
   {
      TokenIterator c(*this);
      ++*this;
      return c;
   };
   operator bool() const
   {
      return !_end;
   };
};

template <>
inline const char *TokenIterator<const char *>::convert(std::string) const
{
   return "<char *> invalid template type";
}

template <>
inline std::string TokenIterator<std::string>::convert(std::string v) const
{
   return v;
}

#endif
