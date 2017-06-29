#pragma once

void *IATPatch(HMODULE, char *, void *, const char *, void *);
void *IATPatch(HMODULE, DWORD, const char*, void*, const char*, void*);