#include "pyBytecodeIterator.hpp"
//#include "opcode.h"
#include "pyopcode.hpp"

PyBytecodeIterator::PyBytecodeIterator(PyObject *bytecode_string)
    : error_(false),
      bytecode_str_((unsigned char *)PyStr_AS_STRING(bytecode_string)),
      bytecode_size_(PyStr_GET_SIZE(bytecode_string))
{
//    assert(PyStr_Check(bytecode_string) &&
  //         "Argument to PyBytecodeIterator() must be a Python string.");
    this->next_index_ = 0;
    // Take advantage of the implementation of Advance() to fill in
    // the other fields.
    this->Advance();
}

PyBytecodeIterator::PyBytecodeIterator(PyBytecodeIterator &iter, int index)
  : error_(false),
    bytecode_str_(iter.bytecode_str_),
    bytecode_size_(iter.bytecode_size_)
{
  this->next_index_ = index;
  this->Advance();
}

void
PyBytecodeIterator::Advance()
{
    this->cur_index_ = this->next_index_;
    if (this->Done()) {
        return;
    }
    this->opcode_ = this->bytecode_str_[this->cur_index_];
    this->next_index_++;
    if (HAS_ARG(this->opcode_)) {
        if (this->next_index_ + 1 >= this->bytecode_size_) {
            PyErr_SetString(PyExc_SystemError,
                            "Argument fell off the end of the bytecode");
            this->error_ = true;
            return;
        }
        this->oparg_ = (this->bytecode_str_[this->next_index_] |
                        this->bytecode_str_[this->next_index_ + 1] << 8);
        this->next_index_ += 2;
        if (this->opcode_ == EXTENDED_ARG) {
            if (this->next_index_ + 2 >= this->bytecode_size_) {
                PyErr_SetString(
                    PyExc_SystemError,
                    "EXTENDED_ARG fell off the end of the bytecode");
                this->error_ = true;
                return;
            }
            this->opcode_ = this->bytecode_str_[this->next_index_];
            if (!HAS_ARG(this->opcode_)) {
                PyErr_SetString(PyExc_SystemError,
                                "Opcode after EXTENDED_ARG must take argument");
                this->error_ = true;
                return;
            }
            this->oparg_ <<= 16;
            this->oparg_ |= (this->bytecode_str_[this->next_index_ + 1] |
                             this->bytecode_str_[this->next_index_ + 2] << 8);
            // Do not update cur_index_; this matches what the eval loop does
            // (ie, it doesn't update f_lasti for EXTENDED_ARG).
            this->next_index_ += 3;
        }
    }
}

/*
* TODO improve the code. Add other version's of python implementation
*/
#if 0
static int
getOPCodeSize(int opcode, int oparg, int jump)
{
#if PY_MAJOR_VERSION >= 3.6
	return 2;
#else
	if(HAS_ARG(opcode))
    switch (opcode) {
        case NOP:
    return PY_INVALID_STACK_EFFECT; /* not reachable */
#endif
}
#endif
