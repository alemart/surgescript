/*
 * SurgeScript
 * A scripting language for games
 * Copyright 2016-2021 Alexandre Martins <alemartf(at)gmail(dot)com>
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
 * surgescript.h
 * SurgeScript main header
 */

#ifndef _SURGESCRIPT_H
#define _SURGESCRIPT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "surgescript/runtime/vm.h"
#include "surgescript/runtime/program.h"
#include "surgescript/runtime/object.h"
#include "surgescript/runtime/program_pool.h"
#include "surgescript/runtime/object_manager.h"
#include "surgescript/runtime/tag_system.h"
#include "surgescript/runtime/vm_time.h"
#include "surgescript/runtime/heap.h"
#include "surgescript/runtime/stack.h"
#include "surgescript/runtime/variable.h"
#include "surgescript/compiler/parser.h"
#include "surgescript/util/transform.h"
#include "surgescript/util/ssarray.h"
#include "surgescript/util/util.h"
#include "surgescript/util/version.h"

#ifdef __cplusplus
}
#endif

#endif
