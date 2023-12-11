#ifndef PARSER_H
#define PARSER_H
#include "../common/program_descriptor.h"

#include <istream>

ProgramDescriptor Parse(std::istream* is);

#endif //PARSER_H