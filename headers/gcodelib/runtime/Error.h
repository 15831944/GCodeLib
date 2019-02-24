#ifndef GCODELIB_RUNTIME_ERROR_H_
#define GCODELIB_RUNTIME_ERROR_H_

#include <string>
#include <exception>

namespace GCodeLib {

  class GCodeRuntimeError : public std::exception {
   public:
    GCodeRuntimeError(const char *);
    GCodeRuntimeError(const std::string &);
  
    const std::string &getMessage() const;
    const char *what() const noexcept override;
   private:
    std::string message;
  };
}

#endif