// The MIT License (MIT)
// Copyright (c) 2014 Yufei (Benny) Chen
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
#ifndef LIBRARY_SYS_LASYNC_H_
#define LIBRARY_SYS_LASYNC_H_

#include <evhtp.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <memory>
#include "./utils.h"

namespace loopy {

class LAsyncBase;

typedef std::shared_ptr<LAsyncBase> LAsyncBasePtr;

// dummy base class so I can have a pointer to it
class LAsyncBase {
 public:
  LAsyncBase()
    : nextAsync_(nullptr)
  {}


  virtual ~LAsyncBase() = default;
  virtual void operator()() = 0;


 private:
  LAsyncBasePtr nextAsync_;
};

/// main specialization
/// dont bother converting lambda to std::function, it doesnt work
template <typename ArgType, typename ReturnType, typename LambdaType>
class LAsync : public LAsyncBase {
 public:
  explicit LAsync(LambdaType cb)
    : callback_(cb)
  {}
  typedef LambdaType Lambda;
  typedef ReturnType NextArgType;

 public:
  void injectArg(ArgType arg) {
    arg_ = arg;
  }
  /// execute the next callback on stack if there is any
  void operator() () {
    callback_(arg_);
  }

  /// T is NextLambdaType
  template<typename T>
  LAsync<NextArgType, decltype(&T::operator()), T>*
  next(T cb) {
    return new LAsync<
      NextArgType,
      decltype(&T::operator()),
      T
    >(cb);
  }

 private:
  /// libevhtp thread variable
  LambdaType    callback_;
  ArgType       arg_;
};

template <typename ReturnType, typename LambdaType>
class LAsync <void, ReturnType, LambdaType> : public LAsyncBase {
 public:
  explicit LAsync(LambdaType cb)
    : callback_(cb)
  {}
  typedef LambdaType Lambda;
  typedef ReturnType NextArgType;

 public:
  /// execute the next callback on stack if there is any
  void operator() () {
    callback_();
  }

  /// T is NextLambdaType
  template<typename T>
  LAsync<void, typename std::result_of<T()>::type, T>*
  next(T cb) {
    return new LAsync<
      void,
      typename std::result_of<T()>::type,
      T
    >(cb);
  }

  /// T is NextLambdaType
  template<typename T2>
  LAsync<NextArgType, typename std::result_of<T2(NextArgType)>::type, T2>*
  next(T2 cb) {
    return new LAsync<
      NextArgType,
      typename std::result_of<T2(NextArgType)>::type,
      T2
    >(cb);
  }


 private:
  /// libevhtp thread variable
  LambdaType    callback_;
};

// case where both are void
template <typename LambdaType>
class LAsync <void, void, LambdaType> : public LAsyncBase {
 public:
  explicit LAsync(LambdaType cb)
    : callback_(cb)
  {}
  typedef LambdaType Lambda;
  typedef void NextArgType;

 public:
  /// execute the next callback on stack if there is any
  void operator() () {
    callback_();
  }

  /// T is NextLambdaType
  template<typename T>
  LAsync<NextArgType, decltype(&T::operator()), T>*
  next(T cb) {
    return new LAsync<
      NextArgType,
      decltype(&T::operator()),
      T
    >(cb);
  }

 private:
  /// libevhtp thread variable
  LambdaType    callback_;
};
} // namespace loopy
#endif  // LIBRARY_SYS_LASYNC_H_
