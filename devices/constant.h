/*
   Copyright 2010 Ken Domino

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#pragma once
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#include "types.h"

class CONSTANT
{
public:

    // Constants in PTX are represented using this class.  The type
    // of the constant is one of K_B8, K_B16, ..., which are values
    // defined in the PTX grammar, using Antlr.
    int type;

    // The actual value of the contant is store here.
    TYPES::Types value;

    CONSTANT(int i)
    {
        type = K_S32;
        value.s32 = i;
    }

    CONSTANT()
    {
        memset(&this->value, 0, sizeof(value));
    }

    CONSTANT Eval(int expected_type, TREE * const_expr);
};

