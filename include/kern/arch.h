#ifndef __ARCH_H
#define __ARCH_H
#ifdef amd64
#include <amd64/x86.h>
#include <amd64/mem_layout.h>
#include <amd64/pt_regs.h>
#else
#include <x86/x86.h>
#include <x86/mem_layout.h>
#include <x86/pt_regs.h>

#endif
#endif
