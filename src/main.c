/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2018 Alexandre Martins <alemartf(at)gmail(dot)com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * runtime/main.c
 * SurgeScript Runtime Engine entry point
 */

#include <stdio.h>
#include "util/util.h"
#include "runtime/vm.h"

/*
 * main()
 * Entry point
 */
int main(int argc, char* argv[])
{
    const char* file = argc > 1 ? argv[1] : NULL;
    if(file) {
        /* spawn the VM and compile the input file */
        surgescript_vm_t* vm = surgescript_vm_create();
        surgescript_vm_compile(vm, file);

        /* run the VM */
        surgescript_vm_launch_ex(vm, argc, argv);
        while(surgescript_vm_update(vm)) {
            ;
        }
        surgescript_vm_destroy(vm);

        /* done! */
        return 0;
    }
    else {
        /* print usage */
        printf("%s\n", SSINFO);
        printf("Usage: %s input-script.ss\n", surgescript_util_basename(argv[0]));
        return 1;
    }
}