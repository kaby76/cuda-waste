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

/*
 * Class TEXREF is used to represent an associated linear global memory with a CUDA texture.
 */
class TEXREF
{
public:
    size_t * offset;
    struct textureReference *texref;
    void *devPtr;
    struct cudaChannelFormatDesc *desc;
    size_t size;
    size_t width;
    size_t height;
    size_t pitch;
};
